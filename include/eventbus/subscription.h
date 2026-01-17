/**
 * @file subscription.h
 * @brief RAII subscription management for EventBus
 *
 * This file provides the Subscription class which implements RAII (Resource
 * Acquisition Is Initialization) pattern for managing event subscriptions.
 * When a Subscription object goes out of scope, it automatically unsubscribes
 * the associated event handler from the EventBus.
 *
 * Key features:
 * - Automatic cleanup when going out of scope
 * - Move-only semantics (no copying)
 * - Thread-safe unsubscription
 * - Validity checking
 */

#pragma once

#include <functional>
#include <utility>

namespace eventbus {

/**
 * @brief RAII subscription manager for EventBus event handlers
 *
 * The Subscription class manages the lifetime of event handler subscriptions.
 * It automatically unsubscribes the handler when the Subscription object is
 * destroyed, ensuring that event handlers are properly cleaned up and don't
 * receive events after they should stop listening.
 *
 * Subscriptions are move-only - they cannot be copied, only moved. This
 * ensures that only one Subscription object owns the unsubscription logic
 * at any time.
 *
 * Example usage:
 * @code
 * {
 *     auto subscription = bus.subscribe<UserLoggedIn>([](const UserLoggedIn& event) {
 *         std::cout << "User logged in: " << event.username << std::endl;
 *     });
 *     // subscription is active here
 *
 *     // Handler will receive events
 *     bus.publish(UserLoggedIn{"alice"});
 *
 * } // subscription goes out of scope, handler is automatically unsubscribed
 *
 * // Handler will NOT receive events anymore
 * bus.publish(UserLoggedIn{"bob"});
 * @endcode
 */
class Subscription {
 public:
  /**
   * @brief Default constructor creating an invalid subscription
   *
   * Creates a subscription that doesn't manage any handler. This is useful
   * for creating placeholder subscriptions or handling error cases.
   */
  Subscription() = default;

  /**
   * @brief Move constructor
   *
   * Transfers ownership of the subscription from another Subscription object.
   * The source subscription becomes invalid after the move.
   *
   * @param other The subscription to move from
   */
  Subscription(Subscription &&other) noexcept : unsubscribe_func_(std::exchange(other.unsubscribe_func_, nullptr)) {}

  /**
   * @brief Move assignment operator
   *
   * Transfers ownership of the subscription from another Subscription object.
   * The current subscription is first unsubscribed, then ownership is transferred.
   * The source subscription becomes invalid after the assignment.
   *
   * @param other The subscription to move from
   * @return Reference to this subscription
   */
  Subscription &operator=(Subscription &&other) noexcept {
    if (this != &other) {
      unsubscribe();
      unsubscribe_func_ = std::exchange(other.unsubscribe_func_, nullptr);
    }
    return *this;
  }

  /**
   * @brief Destructor that automatically unsubscribes
   *
   * Automatically calls unsubscribe() to ensure the event handler is
   * properly removed from the EventBus when the Subscription goes out of scope.
   */
  ~Subscription() noexcept { unsubscribe(); }

  /**
   * @brief Check if the subscription is valid
   *
   * A subscription is valid if it has an associated unsubscribe function,
   * meaning it was successfully created from an EventBus subscription.
   *
   * @return true if the subscription is valid, false otherwise
   */
  [[nodiscard]] bool valid() const noexcept { return unsubscribe_func_ != nullptr; }

  /**
   * @brief Manually unsubscribe the event handler
   *
   * Explicitly unsubscribes the event handler from the EventBus. After calling
   * this method, the subscription becomes invalid and the event handler will
   * no longer receive events.
   *
   * This method is idempotent - calling it multiple times is safe.
   */
  void unsubscribe() {
    if (unsubscribe_func_) {
      unsubscribe_func_();
      unsubscribe_func_ = nullptr;
    }
  }

 private:
  template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
  friend class EventBus;

  explicit Subscription(std::function<void()> unsubscribe_func) : unsubscribe_func_(std::move(unsubscribe_func)) {}

  std::function<void()> unsubscribe_func_;
};

}  // namespace eventbus
