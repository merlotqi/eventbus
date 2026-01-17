#include <eventbus/bus.h>
#include <eventbus/event.h>

#include <iostream>
#include <string>
#include <thread>

// Define multiple event types
struct UserLoginEvent {
  std::string username;
  std::string ip_address;
};

struct OrderPlacedEvent {
  int order_id;
  std::string customer;
  double amount;
};

struct SystemAlertEvent {
  std::string level;
  std::string message;
};

// Synchronous EventBus - single-threaded, blocking execution
using SyncEventBus = eventbus::EventBus<eventbus::Synchronous,  // Synchronous execution - blocking, single-threaded
                                        eventbus::UnboundedQueue, eventbus::BlockProducer, UserLoginEvent,
                                        OrderPlacedEvent, SystemAlertEvent>;

// Publisher class - sends different types of events
class EventPublisher {
 public:
  EventPublisher(SyncEventBus& bus) : bus_(bus) {}

  void publishUserLogin(const std::string& username, const std::string& ip) {
    UserLoginEvent event{username, ip};
    std::cout << "\n[Publisher] Publishing UserLoginEvent for '" << username << "'" << std::endl;

    auto ec = bus_.publish(event);
    if (ec) {
      std::cerr << "Failed to publish login event: " << ec.message() << std::endl;
    } else {
      std::cout << "[Publisher] User login event processed by all subscribers!" << std::endl;
    }
  }

  void publishOrderPlaced(int orderId, const std::string& customer, double amount) {
    OrderPlacedEvent event{orderId, customer, amount};
    std::cout << "\n[Publisher] Publishing OrderPlacedEvent #" << orderId << " for $" << amount << std::endl;

    auto ec = bus_.publish(event);
    if (ec) {
      std::cerr << "Failed to publish order event: " << ec.message() << std::endl;
    } else {
      std::cout << "[Publisher] Order placed event processed by all subscribers!" << std::endl;
    }
  }

  void publishSystemAlert(const std::string& level, const std::string& message) {
    SystemAlertEvent event{level, message};
    std::cout << "\n[Publisher] Publishing SystemAlertEvent [" << level << "]" << std::endl;

    auto ec = bus_.publish(event);
    if (ec) {
      std::cerr << "Failed to publish alert event: " << ec.message() << std::endl;
    } else {
      std::cout << "[Publisher] System alert event processed by all subscribers!" << std::endl;
    }
  }

 private:
  SyncEventBus& bus_;
};

// Security Monitor - only interested in login events
class SecurityMonitor {
 public:
  SecurityMonitor(SyncEventBus& bus) : bus_(bus) {
    std::cout << "[SecurityMonitor] Subscribing to UserLoginEvent only" << std::endl;
    login_subscription_ = bus_.subscribe<UserLoginEvent>([this](const UserLoginEvent& event) { handleLogin(event); });
  }

  void handleLogin(const UserLoginEvent& event) {
    std::cout << "  [SecurityMonitor] User '" << event.username << "' logged in from " << event.ip_address << std::endl;
    std::cout << "    -> Performing security check..." << std::endl;

    // Simulate security processing
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::cout << "    -> Security check completed for " << event.username << std::endl;
  }

 private:
  SyncEventBus& bus_;
  eventbus::Subscription login_subscription_;
};

// Order Processor - only interested in order events
class OrderProcessor {
 public:
  OrderProcessor(SyncEventBus& bus) : bus_(bus) {
    std::cout << "[OrderProcessor] Subscribing to OrderPlacedEvent only" << std::endl;
    order_subscription_ =
        bus_.subscribe<OrderPlacedEvent>([this](const OrderPlacedEvent& event) { handleOrder(event); });
  }

  void handleOrder(const OrderPlacedEvent& event) {
    std::cout << "  [OrderProcessor] Processing order #" << event.order_id << " for customer '" << event.customer << "'"
              << std::endl;
    std::cout << "    -> Amount: $" << event.amount << std::endl;
    std::cout << "    -> Updating inventory and billing..." << std::endl;

    // Simulate order processing
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    std::cout << "    -> Order #" << event.order_id << " processed successfully!" << std::endl;
  }

 private:
  SyncEventBus& bus_;
  eventbus::Subscription order_subscription_;
};

// System Administrator - interested in alerts and logins
class SystemAdministrator {
 public:
  SystemAdministrator(SyncEventBus& bus) : bus_(bus) {
    std::cout << "[SystemAdministrator] Subscribing to UserLoginEvent and SystemAlertEvent" << std::endl;

    login_subscription_ = bus_.subscribe<UserLoginEvent>([this](const UserLoginEvent& event) { handleLogin(event); });

    alert_subscription_ =
        bus_.subscribe<SystemAlertEvent>([this](const SystemAlertEvent& event) { handleAlert(event); });
  }

  void handleLogin(const UserLoginEvent& event) {
    std::cout << "  [SystemAdministrator] User '" << event.username << "' login monitored - logging to audit trail"
              << std::endl;
  }

  void handleAlert(const SystemAlertEvent& event) {
    std::cout << "  [SystemAdministrator] ALERT [" << event.level << "]: " << event.message << std::endl;
    std::cout << "    -> Escalating to system administrators immediately!" << std::endl;
  }

 private:
  SyncEventBus& bus_;
  eventbus::Subscription login_subscription_;
  eventbus::Subscription alert_subscription_;
};

int main() {
  std::cout << "EventBus Synchronous Multi-Event Example" << std::endl;
  std::cout << "Single-threaded, blocking execution - publish waits for all subscribers" << std::endl;
  std::cout << "Different subscribers handle different event types" << std::endl;
  std::cout << "==================================================================" << std::endl;

  // Create synchronous EventBus
  SyncEventBus bus;

  // Create subscribers - each handles different event types
  SecurityMonitor securityMonitor(bus);  // Only UserLoginEvent
  OrderProcessor orderProcessor(bus);    // Only OrderPlacedEvent
  SystemAdministrator sysAdmin(bus);     // UserLoginEvent + SystemAlertEvent

  std::cout << "\n--- Subscribers registered ---" << std::endl;

  // Create publisher
  EventPublisher publisher(bus);

  // Publish multiple events of different types
  std::cout << "\n=== PUBLISHING SEQUENCE START ===" << std::endl;

  // Event 1: User login
  publisher.publishUserLogin("alice", "192.168.1.100");

  // Event 2: Order placed
  publisher.publishOrderPlaced(1001, "Alice Johnson", 299.99);

  // Event 3: Another user login
  publisher.publishUserLogin("bob", "10.0.0.5");

  // Event 4: System alert
  publisher.publishSystemAlert("WARNING", "High CPU usage detected");

  // Event 5: Another order
  publisher.publishOrderPlaced(1002, "Bob Smith", 49.99);

  // Event 6: Another alert
  publisher.publishSystemAlert("ERROR", "Database connection lost");

  std::cout << "\n=== PUBLISHING SEQUENCE COMPLETE ===" << std::endl;

  // Shutdown
  bus.shutdown();

  std::cout << "\nSynchronous multi-event example completed!" << std::endl;
  std::cout << "All events were processed in order, each publish blocked until completion." << std::endl;
  return 0;
}
