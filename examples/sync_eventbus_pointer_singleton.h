#pragma once

#include <eventbus/bus.h>
#include <memory>
#include <mutex>
#include <string>
#include <chrono>

// Event definitions with pointers for ownership demonstration
struct DataProcessedEvent {
    std::string operation;
    std::shared_ptr<std::string> result_data;  // Shared ownership
    std::chrono::system_clock::time_point processed_at;
};

struct ResourceAllocatedEvent {
    std::string resource_type;
    std::unique_ptr<int> resource_id;  // Unique ownership - will be moved
    std::string allocator_name;
};

struct NotificationEvent {
    std::string title;
    std::string message;
    const std::string* sender_name;  // Raw pointer - borrower, no ownership
};

// Synchronous EventBus Singleton with pointer events
using PointerEventBus = eventbus::EventBus<
    eventbus::Synchronous,  // Synchronous execution
    eventbus::UnboundedQueue,
    eventbus::BlockProducer,
    DataProcessedEvent,
    ResourceAllocatedEvent,
    NotificationEvent
>;

// Global synchronous EventBus singleton for pointer examples
class PointerEventBusSingleton {
public:
    // Get the global EventBus instance
    static PointerEventBus& instance();

    // Initialize the singleton
    static void initialize();

    // Shutdown the singleton
    static void shutdown();

    // Prevent copying and moving
    PointerEventBusSingleton(const PointerEventBusSingleton&) = delete;
    PointerEventBusSingleton& operator=(const PointerEventBusSingleton&) = delete;
    PointerEventBusSingleton(PointerEventBusSingleton&&) = delete;
    PointerEventBusSingleton& operator=(PointerEventBusSingleton&&) = delete;

private:
    PointerEventBusSingleton() = default;
    ~PointerEventBusSingleton() = default;

    static std::unique_ptr<PointerEventBus> instance_;
    static std::once_flag init_flag_;
};

// Convenience functions for easy access
inline PointerEventBus& getPointerEventBus() {
    return PointerEventBusSingleton::instance();
}

inline void initializePointerEventBus() {
    PointerEventBusSingleton::initialize();
}

inline void shutdownPointerEventBus() {
    PointerEventBusSingleton::shutdown();
}
