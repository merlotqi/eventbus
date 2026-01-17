#include "sync_eventbus_pointer_singleton.h"

// Static member definitions
std::unique_ptr<PointerEventBus> PointerEventBusSingleton::instance_;
std::once_flag PointerEventBusSingleton::init_flag_;

PointerEventBus& PointerEventBusSingleton::instance() {
  std::call_once(init_flag_, []() { instance_ = std::make_unique<PointerEventBus>(); });
  return *instance_;
}

void PointerEventBusSingleton::initialize() {
  // Ensure the instance is created
  instance();
}

void PointerEventBusSingleton::shutdown() {
  if (instance_) {
    instance_->shutdown();
    instance_.reset();
  }
}
