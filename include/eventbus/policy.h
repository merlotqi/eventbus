#pragma once

#include <cstddef>

namespace eventbus {

// Execution policies
struct Asynchronous {};
struct Synchronous {};
template <size_t N>
struct ThreadPool {
  static constexpr size_t thread_count = N;
};
struct ExternalExecutor {};

// Queue policies (for future bounded queues)
struct UnboundedQueue {};
template <size_t N>
struct BoundedQueue {
  static constexpr size_t capacity = N;
};

// Backpressure policies
struct BlockProducer {};
struct DropOldest {};
struct DropNewest {};

}  // namespace eventbus
