/**
 * @file queue.h
 * @brief Thread-safe event queue implementations with configurable policies
 *
 * This file provides thread-safe queue implementations for storing and retrieving
 * events. The EventQueue class supports different capacity policies (bounded/unbounded)
 * and backpressure policies to handle queue overflow conditions.
 *
 * Key features:
 * - Thread-safe push/pop operations
 * - Configurable capacity limits
 * - Multiple backpressure strategies
 * - Graceful shutdown support
 * - Condition variable synchronization
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <optional>
#include <queue>

#include "error.h"
#include "policy.h"

namespace eventbus {

/**
 * @brief Thread-safe event queue with configurable policies
 *
 * EventQueue provides a thread-safe queue for storing events between producers
 * and consumers. It supports different capacity policies and backpressure
 * strategies to handle various usage patterns and load conditions.
 *
 * @tparam EventVariant The event type stored in the queue (typically Event<...>)
 * @tparam QueuePolicy Policy controlling queue capacity (UnboundedQueue/BoundedQueue)
 * @tparam BackpressurePolicy Policy controlling overflow behavior (BlockProducer/DropOldest/DropNewest)
 */
template <typename EventVariant, typename QueuePolicy = UnboundedQueue, typename BackpressurePolicy = BlockProducer>
class EventQueue {
 public:
  /**
   * @brief Queue capacity determined by QueuePolicy
   *
   * Compile-time constant representing the maximum number of events
   * the queue can hold. For UnboundedQueue, this is the maximum size_t value.
   * For BoundedQueue, this is the template parameter value.
   */
  static constexpr size_t capacity = []() constexpr {
    if constexpr (std::is_same_v<QueuePolicy, UnboundedQueue>) {
      return std::numeric_limits<size_t>::max();
    } else {
      return QueuePolicy::capacity;
    }
  }();

  /** Default constructor */
  EventQueue() = default;

  /** Destructor that calls shutdown() for proper cleanup */
  ~EventQueue() noexcept { shutdown(); }

  /** Copy constructor deleted - queues are not copyable */
  EventQueue(const EventQueue &) = delete;

  /** Copy assignment deleted - queues are not copyable */
  EventQueue &operator=(const EventQueue &) = delete;

  /**
   * @brief Push an event onto the queue
   *
   * Adds an event to the queue. Behavior depends on the BackpressurePolicy:
   *
   * **BlockProducer**: Blocks until space is available
   * **DropNewest**: Returns error if queue is full
   * **DropOldest**: Removes oldest event to make space for new one
   *
   * @param event The event to push onto the queue
   * @return ErrorCode indicating success or failure
   *
   * @retval ErrorCode::Success Event successfully queued
   * @retval ErrorCode::BusShutdown Queue is shutting down
   * @retval ErrorCode::QueueFull Queue is full (DropNewest policy only)
   */
  [[nodiscard]] std::error_code push(EventVariant event) {
    if constexpr (std::is_same_v<BackpressurePolicy, BlockProducer>) {
      std::unique_lock lock(mutex_);
      cv_.wait(lock, [this] { return shutdown_.load() || queue_.size() < capacity; });
      if (shutdown_.load()) {
        return ErrorCode::BusShutdown;
      }
      queue_.push(std::move(event));
      cv_.notify_one();
      return ErrorCode::Success;
    } else {
      std::scoped_lock lock(mutex_);
      if (shutdown_.load()) {
        return ErrorCode::BusShutdown;
      }
      if (queue_.size() >= capacity) {
        if constexpr (std::is_same_v<BackpressurePolicy, DropNewest>) {
          return ErrorCode::QueueFull;
        } else if constexpr (std::is_same_v<BackpressurePolicy, DropOldest>) {
          queue_.pop();
          queue_.push(std::move(event));
          cv_.notify_one();
          return ErrorCode::Success;
        }
      } else {
        queue_.push(std::move(event));
        cv_.notify_one();
        return ErrorCode::Success;
      }
    }
  }

  /**
   * @brief Pop an event from the queue
   *
   * Removes and returns the front event from the queue. If the queue is empty,
   * this method blocks until an event becomes available or shutdown is called.
   *
   * @return Optional containing the event if available, or nullopt if shutdown
   */
  [[nodiscard]] std::optional<EventVariant> pop() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return shutdown_.load() || !queue_.empty(); });
    if (shutdown_.load() && queue_.empty()) return std::nullopt;

    EventVariant event = std::move(queue_.front());
    queue_.pop();
    return event;
  }

  /**
   * @brief Shutdown the queue
   *
   * Initiates shutdown of the queue. After shutdown:
   * - push() operations will fail with BusShutdown error
   * - pop() operations will return nullopt when queue becomes empty
   * - All waiting threads are notified to wake up
   *
   * @note This method is noexcept and thread-safe
   */
  void shutdown() noexcept {
    shutdown_.store(true);
    cv_.notify_all();
  }

  /**
   * @brief Check if the queue is empty
   *
   * @return true if the queue contains no events, false otherwise
   * @note This method is thread-safe
   */
  [[nodiscard]] bool empty() const {
    std::scoped_lock lock(mutex_);
    return queue_.empty();
  }

  /**
   * @brief Get the current number of events in the queue
   *
   * @return Number of events currently in the queue
   * @note This method is thread-safe
   */
  [[nodiscard]] size_t size() const {
    std::scoped_lock lock(mutex_);
    return queue_.size();
  }

 private:
  std::queue<EventVariant> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> shutdown_ = false;
};

}  // namespace eventbus
