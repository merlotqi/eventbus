#include <eventbus/bus.h>
#include <eventbus/event.h>

#include <iostream>
#include <string>

struct TemplateAddEvent {
    std::string class_name;
    std::string value;
    std::string type_name;
};

template<typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
class EventBusManager {
private:
    static std::unique_ptr<eventbus::EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>> instance_;

public:
    static eventbus::EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>& instance() {
        if (!instance_) {
            instance_ = std::make_unique<eventbus::EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>>();
        }
        return *instance_;
    }

    static void shutdown() {
        if (instance_) {
            instance_->shutdown();
            instance_.reset();
        }
    }
};

template<typename ExecutionPolicy, typename QueuePolicy, typename BackpressurePolicy, typename... EventTypes>
std::unique_ptr<eventbus::EventBus<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>>
EventBusManager<ExecutionPolicy, QueuePolicy, BackpressurePolicy, EventTypes...>::instance_ = nullptr;

template<typename T>
class TemplateClass {
public:
    using EventBusType = EventBusManager<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >;

    void add(const T& value) {
        TemplateAddEvent event{
            "TemplateClass<" + std::string(typeid(T).name()) + ">",
            std::to_string(value),
            typeid(T).name()
        };

        std::cout << "[TemplateClass] Publishing add event for value: " << value << std::endl;
        auto ec = EventBusType::instance().publish(event);
        if (ec) {
            std::cerr << "Failed to publish event: " << ec.message() << std::endl;
        }
    }
};

template<>
class TemplateClass<std::string> {
public:
    using EventBusType = EventBusManager<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >;

    void add(const std::string& value) {
        TemplateAddEvent event{
            "TemplateClass<std::string>",
            value,
            "std::string"
        };

        std::cout << "[TemplateClass<std::string>] Publishing add event for value: " << value << std::endl;
        auto ec = EventBusType::instance().publish(event);
        if (ec) {
            std::cerr << "Failed to publish event: " << ec.message() << std::endl;
        }
    }
};

template<>
class TemplateClass<int> {
public:
    using EventBusType = EventBusManager<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >;

    void add(int value) {
        TemplateAddEvent event{
            "TemplateClass<int>",
            std::to_string(value),
            "int"
        };

        std::cout << "[TemplateClass<int>] Publishing add event for value: " << value << std::endl;
        auto ec = EventBusType::instance().publish(event);
        if (ec) {
            std::cerr << "Failed to publish event: " << ec.message() << std::endl;
        }
    }
};


int main() {
    std::cout << "EventBus Template Specialization Example" << std::endl;
    std::cout << "Demonstrates template classes with event publishing" << std::endl;
    std::cout << "=================================================" << std::endl;

    using EventBusType = eventbus::EventBus<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >;

    EventBusType& bus = EventBusManager<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >::instance();

    auto subscription = bus.subscribe<TemplateAddEvent>([](const TemplateAddEvent& event) {
        std::cout << "  [Subscriber] Received event:" << std::endl;
        std::cout << "    -> Class: " << event.class_name << std::endl;
        std::cout << "    -> Value: " << event.value << std::endl;
        std::cout << "    -> Type: " << event.type_name << std::endl;
        std::cout << "    -> Event processed successfully!" << std::endl;
    });

    std::cout << "\n--- Testing Template Specialization ---" << std::endl;

    std::cout << "\n1. Testing generic template class with double:" << std::endl;
    TemplateClass<double> genericClass;
    genericClass.add(3.14159);

    std::cout << "\n2. Testing specialized template class with std::string:" << std::endl;
    TemplateClass<std::string> stringClass;
    stringClass.add("Hello EventBus!");

    std::cout << "\n3. Testing specialized template class with int:" << std::endl;
    TemplateClass<int> intClass;
    intClass.add(42);

    std::cout << "\n4. Testing generic template class with float:" << std::endl;
    TemplateClass<float> floatClass;
    floatClass.add(2.718f);

    std::cout << "\n--- All tests completed ---" << std::endl;

    // clear
    EventBusManager<
        eventbus::Synchronous,
        eventbus::UnboundedQueue,
        eventbus::BlockProducer,
        TemplateAddEvent
    >::shutdown();

    std::cout << "\nTemplate specialization example completed!" << std::endl;
    std::cout << "Each template class specialization published events that were handled by the main subscriber." << std::endl;

    return 0;
}
