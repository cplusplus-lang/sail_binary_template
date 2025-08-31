# Claude Code Configuration

This file contains configuration and instructions for Claude Code when working on the Sail project.

## Build Configuration

### Build Directories
- **Debug builds**: `build/debug/`
- **Release builds**: `build/release/`

### Build Commands
- Debug: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build/debug`
- Release: `cmake -B build/release -DCMAKE_BUILD_TYPE=Release && cmake --build build/release`

### Testing
- Run tests: `ctest --test-dir build/debug` or `ctest --test-dir build/release`
- Test executable: `build/debug/sail_tests` or `build/release/sail_tests`

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
- CMake 3.20+ required

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