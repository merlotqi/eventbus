#pragma once

#include <memory>

namespace eventbus {

template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
class EventBus;

template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
struct EventContext {
  EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...> &bus;
};

template <typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
struct SharedEventContext {
  std::shared_ptr<EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>> bus;
};

}  // namespace eventbus
