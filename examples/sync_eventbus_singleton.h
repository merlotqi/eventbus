#pragma once

#include <eventbus/bus.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

// Event definitions for the synchronous singleton
struct UserLoginEvent {
  std::string username;
  std::string ip_address;
  std::chrono::system_clock::time_point login_time;
};

struct OrderPlacedEvent {
  int order_id;
  std::string customer;
  double amount;
  std::string product;
};

struct SystemAlertEvent {
  std::string level;
  std::string message;
  std::chrono::system_clock::time_point timestamp;
};

// Synchronous EventBus Singleton - global access without explicit passing
// This provides a global synchronous event bus for easy component integration

using SyncEventBus = eventbus::EventBus<eventbus::Synchronous,  // Synchronous execution
                                        eventbus::UnboundedQueue, eventbus::BlockProducer, UserLoginEvent,
                                        OrderPlacedEvent, SystemAlertEvent>;

// Global synchronous EventBus singleton
class SyncEventBusSingleton {
 public:
  // Get the global synchronous EventBus instance
  static SyncEventBus& instance();

  // Initialize the singleton (call once at application startup)
  static void initialize();

  // Shutdown the singleton (call at application shutdown)
  static void shutdown();

  // Prevent copying and moving
  SyncEventBusSingleton(const SyncEventBusSingleton&) = delete;
  SyncEventBusSingleton& operator=(const SyncEventBusSingleton&) = delete;
  SyncEventBusSingleton(SyncEventBusSingleton&&) = delete;
  SyncEventBusSingleton& operator=(SyncEventBusSingleton&&) = delete;

 private:
  SyncEventBusSingleton() = default;
  ~SyncEventBusSingleton() = default;

  static std::unique_ptr<SyncEventBus> instance_;
  static std::once_flag init_flag_;
};

// Convenience functions for easy access
inline SyncEventBus& getSyncEventBus() { return SyncEventBusSingleton::instance(); }

inline void initializeSyncEventBus() { SyncEventBusSingleton::initialize(); }

inline void shutdownSyncEventBus() { SyncEventBusSingleton::shutdown(); }
