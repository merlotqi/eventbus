#include <eventbus/bus.h>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

struct TestEvent {
    std::string message;
};

int main() {
using AsyncBus = eventbus::EventBus<
    eventbus::Asynchronous,
    eventbus::UnboundedQueue,
    eventbus::BlockProducer,
    TestEvent
>;

    AsyncBus bus;

    // Subscribe multiple handlers, some will throw exceptions
    auto sub1 = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Handler 1: Processing '" << e.message << "' - SUCCESS" << std::endl;
    });

    auto sub2 = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Handler 2: About to throw std::runtime_error..." << std::endl;
        throw std::runtime_error("Handler 2 failed!");
    });

    auto sub3 = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Handler 3: Processing '" << e.message << "' - SUCCESS" << std::endl;
    });

    auto sub4 = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Handler 4: About to throw unknown exception..." << std::endl;
        throw 42;  // Unknown exception type
    });

    auto sub5 = bus.subscribe<TestEvent>([](const TestEvent& e) {
        std::cout << "Handler 5: Processing '" << e.message << "' - SUCCESS" << std::endl;
    });

    std::cout << "Publishing event with handlers that throw exceptions..." << std::endl;
    std::cout << "Expected: Handlers 1,3,5 succeed; Handlers 2,4 fail but don't crash others" << std::endl;
    std::cout << "=================================================================" << std::endl;

    TestEvent event{"Test Exception Safety"};
    auto ec = bus.publish(event);
    if (ec) {
        std::cerr << "Publish failed: " << ec.message() << std::endl;
    } else {
        std::cout << "Publish completed - all handlers were attempted!" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    bus.shutdown();
    std::cout << "\nException safety test completed!" << std::endl;
    return 0;
}
