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

template <typename EventVariant, typename QueuePolicy = UnboundedQueue, typename BackpressurePolicy = BlockProducer>
class EventQueue {
 public:
  static constexpr size_t capacity = []() constexpr {
    if constexpr (std::is_same_v<QueuePolicy, UnboundedQueue>) {
      return std::numeric_limits<size_t>::max();
    } else {
      return QueuePolicy::capacity;
    }
  }();

  EventQueue() = default;
  ~EventQueue() noexcept { shutdown(); }

  EventQueue(const EventQueue &) = delete;
  EventQueue &operator=(const EventQueue &) = delete;

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

  [[nodiscard]] std::optional<EventVariant> pop() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return shutdown_.load() || !queue_.empty(); });
    if (shutdown_.load() && queue_.empty()) return std::nullopt;

    EventVariant event = std::move(queue_.front());
    queue_.pop();
    return event;
  }

  void shutdown() noexcept {
    shutdown_.store(true);
    cv_.notify_all();
  }

  [[nodiscard]] bool empty() const {
    std::scoped_lock lock(mutex_);
    return queue_.empty();
  }

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
