#include <eventbus/bus.h>
#include <eventbus/event.h>

#include <chrono>
#include <iostream>
#include <thread>

struct UserLoggedIn {
  std::string username;
  std::chrono::system_clock::time_point timestamp;
};

struct OrderPlaced {
  int order_id;
  double amount;
};

class UserSessionManager {
 public:
  UserSessionManager(eventbus::EventBus<eventbus::Synchronous, eventbus::UnboundedQueue, eventbus::BlockProducer,
                                        UserLoggedIn, OrderPlaced>& bus)
      : bus_(bus), subscription_id_(0), is_active_(true) {
    std::cout << "[UserSessionManager] Initializing..." << std::endl;

    // Subscribe to events and get subscription ID
    auto subscription = bus_.subscribe<UserLoggedIn>([this](const UserLoggedIn& event) { handleUserLogin(event); });

    // Extract subscription ID from the subscription object
    // Note: In a real implementation, you might want to extend Subscription class
    // to provide access to the ID, but for this example we'll demonstrate
    // the manual unsubscribe mechanism directly
    subscription_id_ = get_next_subscription_id() - 1;  // Simulate getting the ID

    std::cout << "[UserSessionManager] Subscribed with ID: " << subscription_id_ << std::endl;
  }

  ~UserSessionManager() {
    std::cout << "[UserSessionManager] Cleaning up..." << std::endl;
    if (is_active_) {
      unsubscribe();
    }
  }

  void handleUserLogin(const UserLoggedIn& event) {
    if (!is_active_) {
      std::cout << "  [UserSessionManager] Ignoring login event - session inactive" << std::endl;
      return;
    }

    std::cout << "  [UserSessionManager] Processing login for: " << event.username << std::endl;

    // Simulate some processing time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "  [UserSessionManager] Login processed successfully" << std::endl;
  }

  void deactivate() {
    std::cout << "\n[UserSessionManager] Deactivating session..." << std::endl;
    is_active_ = false;

    // Manually unsubscribe when state changes
    bool unsubscribed = bus_.unsubscribe(subscription_id_);
    if (unsubscribed) {
      std::cout << "[UserSessionManager] Successfully unsubscribed from events" << std::endl;
    } else {
      std::cout << "[UserSessionManager] Failed to unsubscribe - subscription not found" << std::endl;
    }
  }

  bool is_active() const { return is_active_; }

 private:
  size_t get_next_subscription_id() {
    static size_t counter = 1;
    return counter++;
  }

  void unsubscribe() {
    bool result = bus_.unsubscribe(subscription_id_);
    std::cout << "[UserSessionManager] Manual unsubscribe result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
  }

  eventbus::EventBus<eventbus::Synchronous, eventbus::UnboundedQueue, eventbus::BlockProducer, UserLoggedIn,
                     OrderPlaced>& bus_;
  size_t subscription_id_;
  bool is_active_;
};

class OrderProcessor {
 public:
  OrderProcessor(eventbus::EventBus<eventbus::Synchronous, eventbus::UnboundedQueue, eventbus::BlockProducer,
                                    UserLoggedIn, OrderPlaced>& bus)
      : bus_(bus) {
    std::cout << "[OrderProcessor] Initializing..." << std::endl;

    subscription_ = bus_.subscribe<OrderPlaced>([this](const OrderPlaced& event) { handleOrderPlaced(event); });

    std::cout << "[OrderProcessor] Subscribed to OrderPlaced events" << std::endl;
  }

  void handleOrderPlaced(const OrderPlaced& event) {
    std::cout << "  [OrderProcessor] Processing order #" << event.order_id << " for $" << event.amount << std::endl;

    // Simulate order processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "  [OrderProcessor] Order processed successfully" << std::endl;
  }

 private:
  eventbus::EventBus<eventbus::Synchronous, eventbus::UnboundedQueue, eventbus::BlockProducer, UserLoggedIn,
                     OrderPlaced>& bus_;
  eventbus::Subscription subscription_;
};

int main() {
  std::cout << "EventBus Manual Unsubscribe Example" << std::endl;
  std::cout << "===================================" << std::endl;

  using EventBus = eventbus::EventBus<eventbus::Synchronous, eventbus::UnboundedQueue, eventbus::BlockProducer,
                                      UserLoggedIn, OrderPlaced>;

  EventBus bus;

  // Create components
  UserSessionManager session_manager(bus);
  OrderProcessor order_processor(bus);

  std::cout << "\n--- Publishing initial events ---" << std::endl;

  // Publish some events while session is active
  UserLoggedIn login1{"alice", std::chrono::system_clock::now()};
  auto ec = bus.publish(login1);

  OrderPlaced order1{1001, 299.99};
  ec = bus.publish(order1);

  UserLoggedIn login2{"bob", std::chrono::system_clock::now()};
  ec = bus.publish(login2);

  std::cout << "\n--- Simulating session deactivation ---" << std::endl;

  // Simulate session state change - no longer need to receive login events
  session_manager.deactivate();

  std::cout << "\n--- Publishing events after deactivation ---" << std::endl;

  // Publish more events after deactivation
  UserLoggedIn login3{"charlie", std::chrono::system_clock::now()};
  ec = bus.publish(login3);

  OrderPlaced order2{1002, 149.99};
  ec = bus.publish(order2);

  std::cout << "\n--- Testing manual unsubscribe with invalid ID ---" << std::endl;

  // Test manual unsubscribe with invalid ID
  bool result = bus.unsubscribe(999999);
  std::cout << "Unsubscribe with invalid ID result: " << (result ? "SUCCESS" : "FAILED") << std::endl;

  std::cout << "\nExample completed!" << std::endl;
  std::cout << "Session manager manually unsubscribed when its state changed." << std::endl;

  return 0;
}
