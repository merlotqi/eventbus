# EventBus Examples

This directory contains comprehensive examples demonstrating various EventBus usage patterns and features.

## Directory Structure

```
examples/
├── basic_sync/                    - Basic synchronous EventBus usage
├── async_exception_safety/       - Exception safety in asynchronous mode
├── singleton_pattern/            - Global EventBus singleton pattern
├── multi_events/                 - Multiple event types with selective subscription
├── pointer_ownership/            - Pointer ownership semantics in events
├── multi_publisher_single_subscriber/ - Multiple publishers, single subscriber
├── sync_multi_subscribers/       - Synchronous mode with multiple subscribers
├── CMakeLists.txt               - Master build file for all examples
└── README.md                    - This file
```

## Building Examples

### Build All Examples
```bash
cd examples
mkdir build && cd build
cmake ..
make build_all_examples
```

### Build Individual Example
```bash
cd examples/basic_sync
mkdir build && cd build
cmake ..
make
```

## Example Descriptions

### 1. basic_sync
**Focus**: Basic synchronous EventBus usage
- Simple event publishing and subscription
- Synchronous blocking execution
- Single event type

### 2. async_exception_safety
**Focus**: Exception handling in asynchronous mode
- Multiple handlers with some throwing exceptions
- Exception isolation and error logging
- Asynchronous execution with error recovery

### 3. singleton_pattern
**Focus**: Global EventBus singleton
- No constructor parameters needed
- Automatic global EventBus access
- Thread-safe singleton implementation

### 4. multi_events
**Focus**: Multiple event types
- Different publishers for different event types
- Selective subscription based on event type
- Type-safe event handling

### 5. pointer_ownership
**Focus**: Pointer ownership in events
- `shared_ptr` for shared ownership
- `unique_ptr` for ownership transfer
- Raw pointers for borrowing
- Memory management best practices

### 6. multi_publisher_single_subscriber
**Focus**: Multiple publishers, single subscriber
- Different business logic components as publishers
- Centralized logging/monitoring subscriber
- Statistics and metrics collection

### 7. sync_multi_subscribers
**Focus**: Synchronous mode with multiple subscribers
- Blocking publish waits for all subscribers
- Execution order control
- Synchronous event processing

## Running Examples

After building, run individual examples:

```bash
# From the respective build directory
./basic_sync
./async_exception_safety
./singleton_pattern
# etc.
```

## Key Concepts Demonstrated

- **Execution Modes**: Synchronous vs Asynchronous
- **Singleton Pattern**: Global EventBus access
- **Exception Safety**: Error handling and recovery
- **Pointer Semantics**: Ownership and lifetime management
- **Type Safety**: Compile-time event type checking
- **Subscription Management**: Automatic cleanup
- **Thread Safety**: Concurrent access patterns

## Learning Path

Start with `basic_sync` for fundamentals, then explore:
1. `multi_events` - Multiple event types
2. `sync_multi_subscribers` - Synchronous processing
3. `async_exception_safety` - Error handling
4. `singleton_pattern` - Global access patterns
5. `pointer_ownership` - Advanced memory management
6. `multi_publisher_single_subscriber` - Enterprise patterns

Each example includes detailed comments explaining the concepts and patterns used.
