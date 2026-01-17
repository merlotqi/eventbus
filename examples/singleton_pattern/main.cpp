#include <iostream>
#include <string>
#include <thread>

#include "../sync_eventbus_singleton.h"

class EventPublisher {
 public:
  EventPublisher() { std::cout << "[EventPublisher] Initialized with global EventBus" << std::endl; }

  void publishUserLogin(const std::string& username, const std::string& ip) {
    UserLoginEvent event{username, ip, std::chrono::system_clock::now()};
    std::cout << "\n[Publisher] Publishing UserLoginEvent for '" << username << "'" << std::endl;

    auto ec = getSyncEventBus().publish(event);
    if (ec) {
      std::cerr << "Failed to publish login event: " << ec.message() << std::endl;
    } else {
      std::cout << "[Publisher] User login event processed by all subscribers!" << std::endl;
    }
  }

  void publishOrderPlaced(int orderId, const std::string& customer, double amount) {
    OrderPlacedEvent event{orderId, customer, amount, "Product"};
    std::cout << "\n[Publisher] Publishing OrderPlacedEvent #" << orderId << std::endl;

    auto ec = getSyncEventBus().publish(event);
    if (ec) {
      std::cerr << "Failed to publish order event: " << ec.message() << std::endl;
    } else {
      std::cout << "[Publisher] Order placed event processed by all subscribers!" << std::endl;
    }
  }
};

class SecurityMonitor {
 public:
  SecurityMonitor() {
    std::cout << "[SecurityMonitor] Connecting to global EventBus and subscribing" << std::endl;
    login_subscription_ =
        getSyncEventBus().subscribe<UserLoginEvent>([this](const UserLoginEvent& event) { handleLogin(event); });
  }

  void handleLogin(const UserLoginEvent& event) {
    std::cout << "  [SecurityMonitor] Processing login for '" << event.username << "'" << std::endl;
    std::cout << "    -> Security check in progress..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::cout << "    -> Security check completed" << std::endl;
  }

 private:
  eventbus::Subscription login_subscription_;
};

class OrderProcessor {
 public:
  OrderProcessor() {
    std::cout << "[OrderProcessor] Connecting to global EventBus and subscribing" << std::endl;
    order_subscription_ =
        getSyncEventBus().subscribe<OrderPlacedEvent>([this](const OrderPlacedEvent& event) { handleOrder(event); });
  }

  void handleOrder(const OrderPlacedEvent& event) {
    std::cout << "  [OrderProcessor] Processing order #" << event.order_id << std::endl;
    std::cout << "    -> Customer: " << event.customer << std::endl;
    std::cout << "    -> Amount: $" << event.amount << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    std::cout << "    -> Order processed successfully!" << std::endl;
  }

 private:
  eventbus::Subscription order_subscription_;
};

int main() {
  std::cout << "EventBus Singleton Pattern Example" << std::endl;
  std::cout << "No need to explicitly pass EventBus to constructors" << std::endl;
  std::cout << "Global synchronous EventBus singleton provides automatic connectivity" << std::endl;
  std::cout << "=========================================================================" << std::endl;

  initializeSyncEventBus();

  EventPublisher publisher;  // Uses getSyncEventBus() internally
  SecurityMonitor security;  // Subscribes to global EventBus automatically
  OrderProcessor orders;     // Subscribes to global EventBus automatically

  std::cout << "\n--- Components initialized and connected ---" << std::endl;

  // Publish events - components handle them automatically
  publisher.publishUserLogin("alice", "192.168.1.100");
  publisher.publishOrderPlaced(1001, "Alice Johnson", 299.99);
  publisher.publishUserLogin("bob", "10.0.0.5");
  publisher.publishOrderPlaced(1002, "Bob Smith", 49.99);

  // Shutdown global EventBus (normally at app shutdown)
  shutdownSyncEventBus();

  std::cout << "\nSingleton example completed!" << std::endl;
  std::cout << "Components were automatically connected without explicit EventBus passing." << std::endl;
  return 0;
}
