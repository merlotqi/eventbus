/**
 * @file context.h
 * @brief EventBus context structures for dependency injection
 *
 * This file provides context structures that can be used for dependency
 * injection of EventBus instances into components. Contexts allow components
 * to access EventBus functionality without direct coupling.
 *
 * Two types of contexts are provided:
 * - EventContext: For reference-based access (non-owning)
 * - SharedEventContext: For shared ownership access
 */

#pragma once

#include <memory>

namespace eventbus {

template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
class EventBus;

/**
 * @brief Non-owning EventBus context for dependency injection
 *
 * EventContext provides reference-based access to an EventBus instance.
 * The context does not own the EventBus - it merely provides access to it.
 * This is useful for components that need to publish/subscribe to events
 * but don't manage the EventBus lifecycle.
 *
 * @tparam ExecutionPolicy EventBus execution policy
 * @tparam QueuePolicy EventBus queue policy
 * @tparam BackpressurePolicy EventBus backpressure policy
 * @tparam EventTypes Supported event types
 */
template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
struct EventContext {
  /** Reference to the EventBus instance */
  EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...> &bus;
};

/**
 * @brief Shared ownership EventBus context
 *
 * SharedEventContext provides shared ownership access to an EventBus instance
 * via std::shared_ptr. Multiple components can share the same EventBus instance,
 * and the EventBus will remain alive as long as at least one SharedEventContext
 * references it.
 *
 * @tparam ExecutionPolicy EventBus execution policy
 * @tparam QueuePolicy EventBus queue policy
 * @tparam BackpressurePolicy EventBus backpressure policy
 * @tparam EventTypes Supported event types
 */
template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
struct SharedEventContext {
  /** Shared pointer to the EventBus instance */
  std::shared_ptr<EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>> bus;
};

}  // namespace eventbus
