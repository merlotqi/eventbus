#include <eventbus/bus.h>
#include <eventbus/event.h>
#include <iostream>

struct TestEvent {
    std::string message;
};

int main() {
    using SyncBus = eventbus::EventBus<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TestEvent
    >;

    SyncBus bus;

    auto sub = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Received: " << e.message << std::endl;
    });

    TestEvent event{"Hello Synchronous!"};
    auto ec = bus.publish(event);
    if (ec) {
        std::cerr << "Error: " << ec.message() << std::endl;
    }

    bus.shutdown();
    std::cout << "Test completed!" << std::endl;
    return 0;
}
