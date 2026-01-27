/**
 * @file bus.h
 * @brief Core EventBus implementation with policy-based design
 *
 * This file contains the main EventBus class that provides a flexible,
 * thread-safe event publishing and subscription system. The EventBus uses
 * policy-based design to allow customization of execution strategies,
 * queue policies, and backpressure handling.
 *
 * Key features:
 * - Type-safe event handling with compile-time type checking
 * - Thread-safe publish and subscribe operations
 * - Configurable execution policies (synchronous/asynchronous)
 * - Flexible queue policies for backpressure handling
 * - RAII subscription management
 * - Exception isolation for fault tolerance
 */

#pragma once

#include <iostream>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "dispatcher.h"
#include "event.h"
#include "policy.h"
#include "queue.h"
#include "subscription.h"

namespace eventbus {

template <typename Handler, typename EventType>
struct is_event_handler {
  template <typename H, typename E>
  static auto test(int) -> decltype(std::declval<H>()(std::declval<E>()), std::true_type{});

  template <typename, typename>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<Handler, EventType>(0))::value;
};

template <typename Handler, typename EventType>
inline constexpr bool is_event_handler_v = is_event_handler<Handler, EventType>::value;

/**
 * @brief Main EventBus class providing flexible event publishing and subscription
 *
 * The EventBus class implements a thread-safe, type-safe event system using
 * policy-based design. It allows components to publish events and other
 * components to subscribe to specific event types.
 *
 * @tparam ExecutionPolicy Determines how events are processed (Synchronous/Asynchronous)
 * @tparam QueuePolicy Controls event queuing behavior (UnboundedQueue/BoundedQueue)
 * @tparam BackpressurePolicy Handles producer backpressure (BlockProducer/DropEvents)
 * @tparam EventTypes Variadic template parameters listing all supported event types
 *
 * Example usage:
 * @code
 * // Define event types
 * struct UserLoggedIn { std::string username; };
 * struct OrderPlaced { int order_id; double amount; };
 *
 * // Create event bus
 * using MyBus = EventBus<Synchronous, UnboundedQueue, BlockProducer,
 *                       UserLoggedIn, OrderPlaced>;
 * MyBus bus;
 *
 * // Subscribe to events
 * auto sub = bus.subscribe<UserLoggedIn>([](const UserLoggedIn& event) {
 *     std::cout << "User logged in: " << event.username << std::endl;
 * });
 *
 * // Publish events
 * bus.publish(UserLoggedIn{"alice"});
 * @endcode
 */
template <typename ExecutionPolicy = Asynchronous, typename QueuePolicy = UnboundedQueue,
          typename BackpressurePolicy = BlockProducer, typename... EventTypes>
class EventBus {
 public:
  using EventVariant = Event<EventTypes...>;

  /**
   * @brief Default constructor creating EventBus with default dispatcher
   *
   * Creates an EventBus instance with appropriate dispatcher based on the
   * ExecutionPolicy template parameter:
   * - Synchronous policy: Creates SynchronousDispatcher (no threading)
   * - Asynchronous policy: Creates SingleThreadDispatcher (background thread)
   */
  EventBus()
      : dispatcher_(createDispatcher()),
        queue_(std::make_unique<EventQueue<EventVariant, QueuePolicy, BackpressurePolicy>>()),
        next_id_(0),
        shutdown_(false) {}

  /**
   * @brief Constructor with custom dispatcher
   *
   * Allows injection of a custom dispatcher implementation for specialized
   * execution requirements (e.g., thread pool dispatchers).
   *
   * @param dispatcher Custom dispatcher instance to use for event processing
   */
  explicit EventBus(std::unique_ptr<Dispatcher> dispatcher)
      : dispatcher_(std::move(dispatcher)),
        queue_(std::make_unique<EventQueue<EventVariant, QueuePolicy, BackpressurePolicy>>()),
        next_id_(0),
        shutdown_(false) {}

 private:
  static std::unique_ptr<Dispatcher> createDispatcher() {
    if constexpr (std::is_same_v<ExecutionPolicy, Synchronous>) {
      return std::make_unique<SynchronousDispatcher>();
    } else {
      return std::make_unique<SingleThreadDispatcher>();
    }
  }

 public:
  ~EventBus() noexcept { shutdown(); }
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  /**
   * @brief Subscribe to events of a specific type
   *
   * Registers an event handler for the specified event type. The handler will be
   * called whenever an event of type EventType is published to this EventBus.
   *
   * The subscription is managed via RAII - the returned Subscription object
   * automatically unsubscribes when it goes out of scope.
   *
   * @tparam EventType The event type to subscribe to (must be in EventTypes...)
   * @tparam Handler The callable type (function, lambda, functor) that handles the event
   * @param handler The event handler function/lambda/functor
   * @return Subscription object that manages the subscription lifetime
   *
   * @note The handler must be callable with a const reference to EventType:
   *       `void handler(const EventType& event)`
   *
   * Example:
   * @code
   * auto subscription = bus.subscribe<UserLoggedIn>([](const UserLoggedIn& event) {
   *     std::cout << "User logged in: " << event.username << std::endl;
   * });
   * // subscription automatically unsubscribes when it goes out of scope
   * @endcode
   */
  template <typename EventType, typename Handler, typename = std::enable_if_t<is_event_handler_v<Handler, EventType>>>
  [[nodiscard]] Subscription subscribe(Handler &&handler) {
    if (shutdown_.load()) {
      return Subscription();
    }

    std::scoped_lock lock(handlers_mutex_);
    auto id = next_id_.fetch_add(1);
    auto type_index = std::type_index(typeid(EventType));

    auto wrapper = std::make_shared<HandlerWrapper<EventType, Handler>>(std::forward<Handler>(handler));
    handlers_[type_index][id] = wrapper;
    subscription_map_.insert({id, type_index});

    return Subscription([this, id]() { unsubscribe_internal(id); });
  }

  /**
   * @brief Publish an event to all subscribed handlers
   *
   * Publishes an event to all handlers subscribed to the event type. The behavior
   * depends on the ExecutionPolicy:
   *
   * **Synchronous execution**: All handlers are called immediately in the caller's
   * thread. The publish() call blocks until all handlers complete.
   *
   * **Asynchronous execution**: The event is queued and handlers are called in a
   * background thread. The publish() call returns immediately.
   *
   * @tparam EventType The type of event to publish (must be in EventTypes...)
   * @param event The event instance to publish (moved for efficiency)
   * @return std::error_code indicating success or failure
   *
   * @retval ErrorCode::Success Event published successfully
   * @retval ErrorCode::BusShutdown Bus is shutting down, event not published
   * @retval ErrorCode::QueueFull Queue is full (bounded queue policy only)
   *
   * Example:
   * @code
   * UserLoggedIn event{"alice", std::chrono::system_clock::now()};
   * auto ec = bus.publish(event);
   * if (ec) {
   *     std::cerr << "Failed to publish event: " << ec.message() << std::endl;
   * }
   * @endcode
   */
  template <typename EventType, typename = std::enable_if_t<(std::is_same_v<EventType, EventTypes> || ...)>>
  [[nodiscard]] std::error_code publish(EventType event) {
    if (shutdown_.load()) {
      return ErrorCode::BusShutdown;
    }

    if constexpr (std::is_same_v<ExecutionPolicy, Synchronous>) {
      EventVariant wrapped_event = std::move(event);

      const EventVariant &event_ref = wrapped_event;
      std::scoped_lock lock(handlers_mutex_);

      eventbus::visit(
          [&](const auto &e) {
            using EventTypeInner = std::decay_t<decltype(e)>;
            auto type_index = std::type_index(typeid(EventTypeInner));
            auto it = handlers_.find(type_index);
            if (it != handlers_.end()) {
              for (auto &[id, handler] : it->second) {
                if (handler) {
                  handler->call(event_ref);
                }
              }
            }
          },
          wrapped_event);

      return ErrorCode::Success;
    } else {
      EventVariant wrapped_event = std::move(event);

      auto ec = queue_->push(std::move(wrapped_event));
      if (ec != ErrorCode::Success) {
        return ec;
      }

      dispatcher_->dispatch([this]() { process_events(); });

      return ErrorCode::Success;
    }
  }

  /**
   * @brief Shutdown the EventBus and stop processing events
   *
   * Initiates graceful shutdown of the EventBus. After calling shutdown():
   * - No new events can be published
   * - No new subscriptions are accepted
   * - Existing queued events may still be processed
   * - Background threads (if any) are stopped
   *
   * This method is thread-safe and can be called multiple times.
   * The EventBus destructor automatically calls shutdown().
   *
   * @note This method is noexcept and will not throw exceptions
   */
  void shutdown() noexcept {
    shutdown_.store(true);
    if (queue_) {
      queue_->shutdown();
    }
    if (dispatcher_) {
      dispatcher_->shutdown();
    }
  }

  /**
   * @brief Check if the EventBus is still active
   *
   * Returns true if the EventBus is still accepting new events and subscriptions.
   * Returns false after shutdown() has been called.
   *
   * @return true if the EventBus is active, false if shut down
   *
   * @note This method is thread-safe and can be called concurrently
   */
  [[nodiscard]] bool active() const noexcept { return !shutdown_.load(); }

  /**
   * @brief Manually unsubscribe a handler by subscription ID
   *
   * Allows manual unsubscription of an event handler. This is useful when
   * a subscriber's state changes and it no longer needs to receive events.
   * The subscription ID is returned by the subscribe() method.
   *
   * @param subscription_id The ID of the subscription to unsubscribe
   * @return true if the subscription was found and removed, false otherwise
   *
   * @note This method is thread-safe and can be called from any thread
   * @note If the subscription ID is invalid or already unsubscribed, this is a no-op
   *
   * Example:
   * @code
   * auto subscription_id = bus.subscribe<UserLoggedIn>(handler);
   * // ... later, when no longer needed
   * bus.unsubscribe(subscription_id);
   * @endcode
   */
  [[nodiscard]] bool unsubscribe(size_t subscription_id) {
    std::scoped_lock lock(handlers_mutex_);
    auto it = subscription_map_.find(subscription_id);
    if (it != subscription_map_.end()) {
      auto type_index = it->second;
      auto handler_it = handlers_.find(type_index);
      if (handler_it != handlers_.end()) {
        handler_it->second.erase(subscription_id);
        if (handler_it->second.empty()) {
          handlers_.erase(handler_it);
        }
      }
      subscription_map_.erase(it);
      return true;
    }
    return false;
  }

 private:
  friend class Subscription;

  struct HandlerWrapperBase {
    virtual ~HandlerWrapperBase() = default;
    virtual void call(const EventVariant &event) = 0;
  };

  template <typename EventType, typename Handler>
  struct HandlerWrapper : HandlerWrapperBase {
    Handler handler;
    explicit HandlerWrapper(Handler &&h) : handler(std::move(h)) {}
    void call(const EventVariant &event) override {
      eventbus::visit(
          [this](const auto &e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, EventType>) {
              handler(e);
            }
          },
          event);
    }
  };

  void unsubscribe_internal(size_t id) {
    std::scoped_lock lock(handlers_mutex_);
    auto it = subscription_map_.find(id);
    if (it != subscription_map_.end()) {
      auto type_index = it->second;
      auto handler_it = handlers_.find(type_index);
      if (handler_it != handlers_.end()) {
        handler_it->second.erase(id);
        if (handler_it->second.empty()) {
          handlers_.erase(handler_it);
        }
      }
      subscription_map_.erase(it);
    }
  }

  void process_events() {
    while (auto event_opt = queue_->pop()) {
      if (!event_opt) continue;

      const EventVariant &event = *event_opt;

      std::vector<std::shared_ptr<HandlerWrapperBase>> handlers_copy;
      {
        std::scoped_lock lock(handlers_mutex_);
        eventbus::visit(
            [&](const auto &e) {
              using EventTypeInner = std::decay_t<decltype(e)>;
              auto type_index = std::type_index(typeid(EventTypeInner));
              auto it = handlers_.find(type_index);
              if (it != handlers_.end()) {
                for (auto &[id, handler] : it->second) {
                  if (handler) {
                    handlers_copy.push_back(handler);
                  }
                }
              }
            },
            event);
      }

      for (auto &handler : handlers_copy) {
        try {
          handler->call(event);
        } catch (const std::exception &ex) {
          std::cerr << "Handler execution error: " << ex.what() << std::endl;
        } catch (...) {
          std::cerr << "Handler execution error: unknown exception" << std::endl;
        }
      }
    }
  }

  std::unique_ptr<Dispatcher> dispatcher_;
  std::unique_ptr<EventQueue<EventVariant, QueuePolicy, BackpressurePolicy>> queue_;

  // Map from event type to map of handler ID to handler
  std::unordered_map<std::type_index, std::unordered_map<size_t, std::shared_ptr<HandlerWrapperBase>>> handlers_;
  std::unordered_map<size_t, std::type_index> subscription_map_;

  std::mutex handlers_mutex_;
  std::atomic<size_t> next_id_;
  std::atomic<bool> shutdown_;
};

}  // namespace eventbus
