#include "sync_eventbus_singleton.h"

// Static member definitions
std::unique_ptr<SyncEventBus> SyncEventBusSingleton::instance_;
std::once_flag SyncEventBusSingleton::init_flag_;

SyncEventBus& SyncEventBusSingleton::instance() {
  std::call_once(init_flag_, []() { instance_ = std::make_unique<SyncEventBus>(); });
  return *instance_;
}

void SyncEventBusSingleton::initialize() {
  // Ensure the instance is created
  instance();
}

void SyncEventBusSingleton::shutdown() {
  if (instance_) {
    instance_->shutdown();
    instance_.reset();
  }
}
