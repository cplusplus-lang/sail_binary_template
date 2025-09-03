# Claude Code Configuration

This file contains configuration and instructions for Claude Code when working on the Sail project.

## Build Configuration

### Build Directories
- **Debug builds**: `build/debug/`
- **Release builds**: `build/release/`

### Build Commands

**NOTE**: The build system now requires explicit target selection. You must specify either `-DBUILD_BINARY=ON` for the main executable or `-DBUILD_TESTS=ON` for tests (or both).

#### Binary Only
- Debug: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_BINARY=ON && cmake --build build/debug`
- Release: `cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DBUILD_BINARY=ON && cmake --build build/release`

#### Tests Only
- Debug: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON && cmake --build build/debug`
- Release: `cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON && cmake --build build/release`

#### Both Binary and Tests
- Debug: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_BINARY=ON -DBUILD_TESTS=ON && cmake --build build/debug`
- Release: `cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DBUILD_BINARY=ON -DBUILD_TESTS=ON && cmake --build build/release`

### Testing

**NOTE**: Tests are only built when `-DBUILD_TESTS=ON` is specified during configuration.

- Run tests: `ctest --test-dir build/debug` or `ctest --test-dir build/release`
- Test executable: `build/debug/sail_tests` or `build/release/sail_tests`
- To build and run tests: First build with tests enabled, then run CTest or the test executable directly

### Linting and Type Checking
- Currently no specific lint or typecheck commands configured
- Build process includes compiler warnings and errors

## Project Structure

### Source Files
- Main executable: `src/main.cpp`
- Core library: `saillib` (excluding main.cpp)
- Headers: `include/`
- Tests: `tests/`

### Dependencies
- Catch2 v3.10.0 (for testing)
- Git (required system dependency)
- CMake 3.21+ required

### Key Features
- Package manager functionality
- Git repository cloning and management
- Cross-platform support (Windows, macOS, Linux)
- Command-line interface with install, new, and other commands

## Development Notes
- Use CMake for building
- Tests run with CTest
- GitHub Actions configured for cross-platform CI/CD
- C++17 standard required