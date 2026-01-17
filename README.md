# EventBus

A modern, header-only C++20 event bus library with flexible execution policies, thread safety, and comprehensive examples.

## Features

- **Header-only**: No need to link against a library
- **Multiple Execution Policies**: Synchronous and Asynchronous
- **Thread-safe**: Concurrent publish/subscribe operations
- **Type-safe**: Compile-time event type checking
- **Flexible**: Support for multiple event types in a single bus
- **Exception-safe**: Isolated error handling
- **Modern C++**: Requires C++20, uses latest language features

## Quick Start

### Basic Usage

```cpp
#include <eventbus/bus.h>
#include <eventbus/event.h>

// Define your event types
struct UserLoggedIn {
    std::string username;
    std::chrono::system_clock::time_point timestamp;
};

struct OrderPlaced {
    int order_id;
    double amount;
};

// Create an event bus
using MyEventBus = eventbus::EventBus<
    eventbus::Synchronous,  // Execution policy
    eventbus::UnboundedQueue,  // Queue policy
    eventbus::BlockProducer,  // Backpressure policy
    UserLoggedIn,
    OrderPlaced
>;

MyEventBus bus;

// Subscribe to events
auto subscription = bus.subscribe<UserLoggedIn>([](const UserLoggedIn& event) {
    std::cout << "User logged in: " << event.username << std::endl;
});

// Publish events
UserLoggedIn login{"alice", std::chrono::system_clock::now()};
bus.publish(login);

bus.shutdown(); // Clean shutdown
```

### Singleton Pattern

For global access without dependency injection:

```cpp
#include "sync_eventbus_singleton.h"

// Use global singleton
getSyncEventBus().subscribe<UserLoggedIn>(handler);
getSyncEventBus().publish(login_event);
```

## Execution Policies

### Synchronous
- Events are processed immediately in the caller's thread
- Blocking publish - waits for all subscribers to complete
- Simple, predictable execution order

### Asynchronous
- Events are queued and processed in background threads
- Non-blocking publish - returns immediately
- Better performance for high-throughput scenarios

## Build and Installation

### Build Library Only

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

### Build with Examples

```bash
mkdir build && cd build
cmake .. -DEVENTBUS_BUILD_EXAMPLES=ON
make
# Run examples
./examples/basic_sync/basic_sync
```

### Generate Documentation

EventBus uses Doxygen for API documentation generation:

```bash
mkdir build && cd build
cmake ..
make docs
# Open docs/html/index.html in your browser
```

### Using in Your Project

#### CMake (Recommended)

```cmake
find_package(eventbus REQUIRED)
target_link_libraries(your_target eventbus)
```

#### Manual Include

```cmake
# Add include directory
target_include_directories(your_target PRIVATE /path/to/eventbus/include)
```

## Examples

The `examples/` directory contains comprehensive examples demonstrating:

- **basic_sync**: Basic synchronous usage
- **async_exception_safety**: Exception handling in async mode
- **singleton_pattern**: Global singleton pattern
- **pointer_ownership**: Pointer ownership semantics
- **multi_publisher_single_subscriber**: Multiple publishers, single subscriber
- **sync_multi_subscribers**: Synchronous mode with multiple subscribers

Run examples with:
```bash
cmake .. -DEVENTBUS_BUILD_EXAMPLES=ON
make build_all_examples
```

## Architecture

### Core Components

- **EventBus**: Main event bus class with policy-based design
- **Event**: Type-safe event variant container
- **Subscription**: RAII subscription management
- **Dispatcher**: Execution policy implementation
- **Queue**: Event queuing policies

### Thread Safety

- Publishers and subscribers can operate concurrently
- Internal mutexes protect shared state
- Lock-and-copy strategy prevents deadlocks
- Exception isolation maintains system stability

### Memory Management

- Shared ownership for event data
- Automatic cleanup of cancelled subscriptions
- Configurable queue policies for backpressure

## Requirements

- **C++20** or later
- **CMake** 3.16+
- **Threading support** (std::thread, std::mutex)

## Supported Platforms

- Linux (GCC 11+, Clang 14+)
- macOS (Apple Clang, GCC)
- Windows (MSVC 2022+, MinGW)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure CI passes
5. Submit a pull request

### Code Style

This project uses clang-format for code formatting. Run:
```bash
clang-format -i include/**/*.h examples/**/*.cpp
```

## License

See LICENSE file for details.

## Roadmap

- [ ] Performance benchmarks
- [ ] Additional queue policies
- [ ] Event filtering and routing
- [ ] Metrics and monitoring
- [ ] Python bindings
