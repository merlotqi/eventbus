#include "../sync_eventbus_pointer_singleton.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class DataProcessor {
public:
    DataProcessor() {
        std::cout << "[DataProcessor] Initialized" << std::endl;
    }

    void processData(const std::string& input) {
        std::cout << "\n[DataProcessor] Processing: '" << input << "'" << std::endl;

        auto result = std::make_shared<std::string>(input + " [PROCESSED]");

        DataProcessedEvent event{
            "text_processing",
            result,
            std::chrono::system_clock::now()
        };

        std::cout << "[DataProcessor] Publishing DataProcessedEvent with shared_ptr" << std::endl;
        auto ec = getPointerEventBus().publish(event);
        if (ec) {
            std::cerr << "Failed to publish data event: " << ec.message() << std::endl;
        } else {
            std::cout << "[DataProcessor] Data processing event sent!" << std::endl;
        }

        std::cout << "[DataProcessor] Original processor still has access to result: "
                  << *result << std::endl;
    }
};

class ResourceManager {
public:
    ResourceManager() : next_id_(1000) {
        std::cout << "[ResourceManager] Initialized" << std::endl;
    }

    void allocateResource(const std::string& type) {
        std::cout << "\n[ResourceManager] Allocating " << type << " resource" << std::endl;

        auto resource_id = std::make_unique<int>(next_id_++);

        std::cout << "[ResourceManager] Created resource ID: " << *resource_id << std::endl;

        ResourceAllocatedEvent event{
            type,
            std::move(resource_id),
            "ResourceManager"
        };

        if (!resource_id) {
            std::cout << "[ResourceManager] Ownership transferred, resource_id is now null" << std::endl;
        }

        std::cout << "[ResourceManager] Publishing ResourceAllocatedEvent with moved unique_ptr" << std::endl;
        auto ec = getPointerEventBus().publish(std::move(event));  // Move the entire event
        if (ec) {
            std::cerr << "Failed to publish resource event: " << ec.message() << std::endl;
        } else {
            std::cout << "[ResourceManager] Resource allocation event sent!" << std::endl;
        }
    }

private:
    int next_id_;
};

class NotificationService {
public:
    NotificationService() : service_name_("NotificationService") {
        std::cout << "[NotificationService] Initialized" << std::endl;
    }

    void sendNotification(const std::string& title, const std::string& message) {
        std::cout << "\n[NotificationService] Sending notification: " << title << std::endl;

        NotificationEvent event{
            title,
            message,
            &service_name_
        };

        std::cout << "[NotificationService] Publishing NotificationEvent with raw pointer" << std::endl;
        auto ec = getPointerEventBus().publish(event);
        if (ec) {
            std::cerr << "Failed to publish notification event: " << ec.message() << std::endl;
        } else {
            std::cout << "[NotificationService] Notification event sent!" << std::endl;
        }

        std::cout << "[NotificationService] Service name still accessible: " << service_name_ << std::endl;
    }

private:
    std::string service_name_;
};

class AnalyticsService {
public:
    AnalyticsService() {
        std::cout << "[AnalyticsService] Subscribing to DataProcessedEvent (shared_ptr)" << std::endl;
        data_subscription_ = getPointerEventBus().subscribe<DataProcessedEvent>(
            [this](const DataProcessedEvent& event) {
                handleDataProcessed(event);
            }
        );
    }

    void handleDataProcessed(const DataProcessedEvent& event) {
        std::cout << "  [AnalyticsService] Analyzing processed data" << std::endl;
        std::cout << "    -> Operation: " << event.operation << std::endl;
        if (event.result_data) {
            std::cout << "    -> Data: " << *event.result_data << std::endl;
            std::cout << "    -> Data length: " << event.result_data->length() << std::endl;
        } else {
            std::cout << "    -> No data available" << std::endl;
        }
        std::cout << "    -> Analytics completed" << std::endl;
    }

private:
    eventbus::Subscription data_subscription_;
};

// Resource Monitor - handles moved unique_ptr
class ResourceMonitor {
public:
    ResourceMonitor() {
        std::cout << "[ResourceMonitor] Subscribing to ResourceAllocatedEvent (unique_ptr)" << std::endl;
        resource_subscription_ = getPointerEventBus().subscribe<ResourceAllocatedEvent>(
            [this](const ResourceAllocatedEvent& event) {  // Receive by const reference
                handleResourceAllocated(event);  // Will move from const reference (dangerous!)
            }
        );
    }

    void handleResourceAllocated(const ResourceAllocatedEvent& event) {
        std::cout << "  [ResourceMonitor] Monitoring allocated resource" << std::endl;
        std::cout << "    -> Type: " << event.resource_type << std::endl;
        std::cout << "    -> Allocator: " << event.allocator_name << std::endl;
        if (event.resource_id) {
            std::cout << "    -> Resource ID: " << *event.resource_id << std::endl;
            std::cout << "    -> Monitoring resource (no ownership transfer)" << std::endl;
            // In a real system, we might log this or update monitoring stats
            monitored_resources_.push_back(*event.resource_id);
        } else {
            std::cout << "    -> No resource ID provided" << std::endl;
        }
    }

private:
    eventbus::Subscription resource_subscription_;
    std::vector<int> monitored_resources_;  // Track monitored resource IDs
};

// Alert Handler - handles raw pointer notifications
class AlertHandler {
public:
    AlertHandler() {
        std::cout << "[AlertHandler] Subscribing to NotificationEvent (raw pointer)" << std::endl;
        notification_subscription_ = getPointerEventBus().subscribe<NotificationEvent>(
            [this](const NotificationEvent& event) {
                handleNotification(event);
            }
        );
    }

    void handleNotification(const NotificationEvent& event) {
        std::cout << "  [AlertHandler] Handling notification" << std::endl;
        std::cout << "    -> Title: " << event.title << std::endl;
        std::cout << "    -> Message: " << event.message << std::endl;
        if (event.sender_name) {
            std::cout << "    -> From: " << *event.sender_name << std::endl;
        } else {
            std::cout << "    -> Sender unknown" << std::endl;
        }
        std::cout << "    -> Alert processed" << std::endl;
    }

private:
    eventbus::Subscription notification_subscription_;
};

int main() {
    std::cout << "EventBus Pointer Ownership Example" << std::endl;
    std::cout << "Demonstrating different pointer semantics in events" << std::endl;
    std::cout << "=====================================================" << std::endl;

    // Initialize global EventBus
    initializePointerEventBus();

    // Create publishers
    DataProcessor dataProcessor;
    ResourceManager resourceManager;
    NotificationService notificationService;

    // Create subscribers
    AnalyticsService analytics;
    ResourceMonitor resourceMonitor;
    AlertHandler alertHandler;

    std::cout << "\n--- Starting pointer ownership demonstrations ---" << std::endl;

    // Demonstrate shared_ptr ownership (multiple access)
    dataProcessor.processData("Hello World");

    // Demonstrate unique_ptr ownership transfer
    resourceManager.allocateResource("database_connection");
    resourceManager.allocateResource("file_handle");

    // Demonstrate raw pointer borrowing (no ownership)
    notificationService.sendNotification("System Update", "Version 2.0 deployed successfully");
    notificationService.sendNotification("Maintenance", "Scheduled downtime in 1 hour");

    // Cleanup
    shutdownPointerEventBus();

    std::cout << "\nPointer ownership example completed!" << std::endl;
    std::cout << "Demonstrated shared_ptr (shared), unique_ptr (moved), and raw pointer (borrowed)" << std::endl;
    return 0;
}
