#include "../sync_eventbus_singleton.h"
#include <iostream>
#include <string>

class UserManager {
public:
    UserManager() {
        std::cout << "[UserManager] Initialized - will publish UserLoginEvent" << std::endl;
    }

    void loginUser(const std::string& username, const std::string& ip) {
        std::cout << "\n[UserManager] User '" << username << "' logging in from " << ip << std::endl;

        UserLoginEvent event{username, ip, std::chrono::system_clock::now()};
        auto ec = getSyncEventBus().publish(event);
        if (ec) {
            std::cerr << "[UserManager] Failed to publish login event" << std::endl;
        } else {
            std::cout << "[UserManager] Login event published successfully" << std::endl;
        }
    }

    void logoutUser(const std::string& username) {
        std::cout << "\n[UserManager] User '" << username << "' logging out" << std::endl;

        UserLoginEvent event{username, "logout", std::chrono::system_clock::now()};
        auto ec = getSyncEventBus().publish(event);
        if (ec) {
            std::cerr << "[UserManager] Failed to publish logout event" << std::endl;
        } else {
            std::cout << "[UserManager] Logout event published successfully" << std::endl;
        }
    }
};

class OrderProcessor {
public:
    OrderProcessor() : order_counter_(2000) {
        std::cout << "[OrderProcessor] Initialized - will publish OrderPlacedEvent" << std::endl;
    }

    void processOrder(const std::string& customer, double amount) {
        int order_id = order_counter_++;
        std::cout << "\n[OrderProcessor] Processing order #" << order_id
                  << " for '" << customer << "' - $" << amount << std::endl;

        OrderPlacedEvent event{order_id, customer, amount, "Online Purchase"};
        auto ec = getSyncEventBus().publish(event);
        if (ec) {
            std::cerr << "[OrderProcessor] Failed to publish order event" << std::endl;
        } else {
            std::cout << "[OrderProcessor] Order event published successfully" << std::endl;
        }
    }

private:
    int order_counter_;
};

class SystemMonitor {
public:
    SystemMonitor() {
        std::cout << "[SystemMonitor] Initialized - will publish SystemAlertEvent" << std::endl;
    }

    void reportHighCpu() {
        std::cout << "\n[SystemMonitor] High CPU usage detected!" << std::endl;

        SystemAlertEvent event{"WARNING", "CPU usage above 90%", std::chrono::system_clock::now()};
        auto ec = getSyncEventBus().publish(event);
        if (ec) {
            std::cerr << "[SystemMonitor] Failed to publish CPU alert" << std::endl;
        } else {
            std::cout << "[SystemMonitor] CPU alert published successfully" << std::endl;
        }
    }

    void reportLowMemory() {
        std::cout << "\n[SystemMonitor] Low memory warning!" << std::endl;

        SystemAlertEvent event{"ERROR", "Available memory below 100MB", std::chrono::system_clock::now()};
        auto ec = getSyncEventBus().publish(event);
        if (ec) {
            std::cerr << "[SystemMonitor] Failed to publish memory alert" << std::endl;
        } else {
            std::cout << "[SystemMonitor] Memory alert published successfully" << std::endl;
        }
    }
};

class EventLogger {
public:
    EventLogger() {
        std::cout << "[EventLogger] Initializing - subscribing to all event types" << std::endl;

        // Subscribe to all event types
        login_subscription_ = getSyncEventBus().subscribe<UserLoginEvent>(
            [this](const UserLoginEvent& event) {
                logUserEvent(event);
            }
        );

        order_subscription_ = getSyncEventBus().subscribe<OrderPlacedEvent>(
            [this](const OrderPlacedEvent& event) {
                logOrderEvent(event);
            }
        );

        alert_subscription_ = getSyncEventBus().subscribe<SystemAlertEvent>(
            [this](const SystemAlertEvent& event) {
                logAlertEvent(event);
            }
        );

        std::cout << "[EventLogger] Successfully subscribed to all event types" << std::endl;
    }

    void logUserEvent(const UserLoginEvent& event) {
        std::cout << "  [EVENT_LOG] User Event: '" << event.username
                  << "' from IP " << event.ip_address << std::endl;
        std::cout << "    -> Logged to user activity audit trail" << std::endl;
        user_events_logged_++;
    }

    void logOrderEvent(const OrderPlacedEvent& event) {
        std::cout << "  [EVENT_LOG] Order Event: #" << event.order_id
                  << " by '" << event.customer << "' for $" << event.amount << std::endl;
        std::cout << "    -> Product: " << event.product << std::endl;
        std::cout << "    -> Logged to business analytics" << std::endl;
        order_events_logged_++;
    }

    void logAlertEvent(const SystemAlertEvent& event) {
        std::cout << "  [EVENT_LOG] System Alert [" << event.level << "]: "
                  << event.message << std::endl;
        std::cout << "    -> Escalated to system administrators" << std::endl;
        std::cout << "    -> Logged to system monitoring dashboard" << std::endl;
        alert_events_logged_++;
    }

    void printStatistics() {
        std::cout << "\n[EventLogger] Final Statistics:" << std::endl;
        std::cout << "  - User events logged: " << user_events_logged_ << std::endl;
        std::cout << "  - Order events logged: " << order_events_logged_ << std::endl;
        std::cout << "  - Alert events logged: " << alert_events_logged_ << std::endl;
        std::cout << "  - Total events processed: "
                  << (user_events_logged_ + order_events_logged_ + alert_events_logged_)
                  << std::endl;
    }

private:
    eventbus::Subscription login_subscription_;
    eventbus::Subscription order_subscription_;
    eventbus::Subscription alert_subscription_;

    int user_events_logged_ = 0;
    int order_events_logged_ = 0;
    int alert_events_logged_ = 0;
};

int main() {
    std::cout << "Multiple Publishers, Single Subscriber Example" << std::endl;
    std::cout << "Using global EventBus singleton - synchronous blocking mode" << std::endl;
    std::cout << "=================================================================" << std::endl;

    // Initialize global EventBus
    initializeSyncEventBus();

    // Create multiple publishers
    UserManager userManager;
    OrderProcessor orderProcessor;
    SystemMonitor systemMonitor;

    // Create single subscriber that handles all event types
    EventLogger eventLogger;

    std::cout << "\n--- Starting event publishing sequence ---" << std::endl;

    // User events
    userManager.loginUser("alice", "192.168.1.100");
    userManager.loginUser("bob", "10.0.0.5");

    // Order events
    orderProcessor.processOrder("Alice Johnson", 299.99);
    orderProcessor.processOrder("Bob Smith", 49.99);

    // System events
    systemMonitor.reportHighCpu();

    // More user events
    userManager.logoutUser("alice");
    userManager.loginUser("charlie", "172.16.0.10");

    // More system events
    systemMonitor.reportLowMemory();

    // More order events
    orderProcessor.processOrder("Charlie Brown", 149.99);

    std::cout << "\n--- Publishing sequence complete ---" << std::endl;

    // Show statistics
    eventLogger.printStatistics();

    // Cleanup
    shutdownSyncEventBus();

    std::cout << "\nExample completed successfully!" << std::endl;
    std::cout << "Demonstrated multiple publishers with different event types," << std::endl;
    std::cout << "all handled by a single subscriber via global EventBus singleton." << std::endl;
    return 0;
}
