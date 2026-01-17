/**
 * @file policy.h
 * @brief Policy classes for EventBus configuration
 *
 * This file defines policy classes that control various aspects of EventBus
 * behavior. Policies use template metaprogramming to customize EventBus
 * functionality at compile time, providing zero-cost abstractions.
 *
 * Policies are divided into three categories:
 * - Execution policies: Control how events are dispatched and executed
 * - Queue policies: Control event queuing behavior and capacity
 * - Backpressure policies: Control what happens when queues are full
 */

#pragma once

#include <cstddef>

namespace eventbus {

/**
 * @brief Execution policy classes
 *
 * These policies determine how event handler tasks are executed.
 * They control threading behavior and execution timing.
 */

/**
 * @brief Asynchronous execution policy
 *
 * Events are queued and executed in background threads. Publish operations
 * return immediately without waiting for handlers to complete. Provides
 * non-blocking behavior but may reorder events.
 */
struct Asynchronous {};

/**
 * @brief Synchronous execution policy
 *
 * Events are executed immediately in the calling thread. Publish operations
 * block until all handlers complete. Provides deterministic execution order
 * but may block the caller.
 */
struct Synchronous {};

/**
 * @brief Thread pool execution policy
 *
 * Events are executed using a fixed-size thread pool. Provides concurrent
 * execution with controlled resource usage.
 *
 * @tparam N Number of threads in the pool
 */
template <size_t N>
struct ThreadPool {
  static constexpr size_t thread_count = N;
};

/**
 * @brief External executor policy
 *
 * Allows integration with external executor frameworks (e.g., boost::asio,
 * Qt event loops, custom thread pools). The user provides an executor
 * callback for task scheduling.
 */
struct ExternalExecutor {};

/**
 * @brief Queue policy classes
 *
 * These policies control event queuing behavior, including capacity limits
 * and memory usage characteristics.
 */

/**
 * @brief Unbounded queue policy
 *
 * Queue can grow indefinitely. No capacity limits, but may consume
 * unlimited memory under high load. Suitable for applications where
 * memory is not a concern and events must never be dropped.
 */
struct UnboundedQueue {};

/**
 * @brief Bounded queue policy
 *
 * Queue has a fixed maximum capacity. When full, backpressure policy
 * determines what happens to new events.
 *
 * @tparam N Maximum number of events the queue can hold
 */
template <size_t N>
struct BoundedQueue {
  static constexpr size_t capacity = N;
};

/**
 * @brief Backpressure policy classes
 *
 * These policies determine what happens when a bounded queue is full
 * and new events arrive.
 */

/**
 * @brief Block producer backpressure policy
 *
 * When queue is full, publish operations block until space becomes
 * available. Provides backpressure to slow down producers, ensuring
 * no events are lost but may block the calling thread.
 */
struct BlockProducer {};

/**
 * @brief Drop oldest events backpressure policy
 *
 * When queue is full, the oldest event in the queue is discarded
 * to make room for the new event. Ensures newest events are always
 * processed, but may lose older events under high load.
 */
struct DropOldest {};

/**
 * @brief Drop newest events backpressure policy
 *
 * When queue is full, new events are silently discarded.
 * Preserves existing queued events but may lose recent events
 * under high load. Useful when processing order matters.
 */
struct DropNewest {};

}  // namespace eventbus
