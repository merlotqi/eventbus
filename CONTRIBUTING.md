# Contributing to EventBus

Thank you for your interest in contributing to EventBus! This document provides guidelines and information for contributors.

## Development Setup

### Prerequisites

- CMake 3.16+
- C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Git

### Building from Source

```bash
# Clone the repository
git clone https://github.com/merlotqi/eventbus.git
cd eventbus

# Create build directory
mkdir build && cd build

# Configure with examples (for development)
cmake .. -DEVENTBUS_BUILD_EXAMPLES=ON

# Build
make

# Run tests/examples
./examples/basic_sync/basic_sync
```

## Development Workflow

1. **Fork** the repository on GitHub
2. **Clone** your fork locally
3. **Create** a feature branch: `git checkout -b feature/your-feature-name`
4. **Make** your changes
5. **Test** your changes: `cmake .. -DEVENTBUS_BUILD_EXAMPLES=ON && make build_all_examples`
6. **Format** code: `clang-format -i include/**/*.h examples/**/*.cpp`
7. **Commit** your changes: `git commit -m "Add your feature"`
8. **Push** to your fork: `git push origin feature/your-feature-name`
9. **Create** a Pull Request

## Code Style

This project uses clang-format for consistent code formatting. The style configuration is in `.clang-format`.

### Formatting

```bash
# Format all source files
find include examples -name "*.h" -o -name "*.cpp" | xargs clang-format -i

# Check formatting without modifying files
find include examples -name "*.h" -o -name "*.cpp" | xargs clang-format --dry-run --Werror
```

### Naming Conventions

- **Classes**: `CamelCase`
- **Functions/Methods**: `camelBack`
- **Variables**: `camelBack`
- **Constants**: `UPPER_CASE`
- **Namespaces**: `lower_case`

## Testing

### Running Examples

All examples serve as integration tests. Build and run them to ensure functionality:

```bash
cmake .. -DEVENTBUS_BUILD_EXAMPLES=ON
make build_all_examples
# Run individual examples to verify they work
```

### Adding New Examples

When adding new functionality, create a corresponding example in `examples/`:

1. Create a new directory: `examples/your_feature/`
2. Add `CMakeLists.txt` and `main.cpp`
3. Update `examples/CMakeLists.txt` to include your example
4. Update `examples/README.md`

## Pull Request Guidelines

### Before Submitting

- [ ] Code compiles without warnings
- [ ] All examples build and run successfully
- [ ] Code is properly formatted
- [ ] Documentation is updated if needed
- [ ] No breaking changes without justification

### PR Description

Include:
- Description of changes
- Motivation for the changes
- Any breaking changes
- Testing instructions

### Review Process

1. CI checks must pass (build, format, static analysis)
2. At least one maintainer review
3. All discussions resolved
4. Squash commits if requested

## Architecture Guidelines

### Core Principles

- **Header-only**: Keep the library header-only for simplicity
- **Zero-cost abstractions**: Use templates and policies effectively
- **Thread safety**: Ensure concurrent operations are safe
- **Exception safety**: Handle errors gracefully
- **Modern C++**: Use C++20 features appropriately

### Adding New Features

1. **Start with examples**: Create examples demonstrating the feature
2. **Consider policies**: Use policy-based design for flexibility
3. **Thread safety**: Ensure new features are thread-safe
4. **Documentation**: Update README and code comments
5. **Testing**: Add examples that thoroughly test the feature

### Breaking Changes

- Clearly document breaking changes
- Provide migration guides
- Consider deprecation warnings for major versions

## Issue Reporting

When reporting bugs:

- Use the issue templates
- Include minimal reproducible examples
- Specify compiler, OS, and CMake versions
- Include build logs if applicable

## Code of Conduct

This project follows a standard code of conduct. Be respectful and constructive in all interactions.

## Getting Help

- Check existing issues and documentation first
- Use discussions for questions
- Join community chats if available

Thank you for contributing to EventBus! 🎉
