# Sail Project - Claude Development Guide

## Project Overview

This project is based on the **cpp-best-practices/cmake_template** and should maintain consistency with that template. Any modifications should be made for very good reasons and implemented in a modular, isolated manner to avoid diverging from the template structure.

## Package Layout

```
sail/
├── CMakeLists.txt              # Main CMake configuration
├── CMakePresets.json          # CMake presets for different build configurations
├── ProjectOptions.cmake       # Project-specific CMake options
├── Dependencies.cmake         # Dependency management
├── README.md                  # Project documentation
├── README_building.md         # Build instructions
├── README_dependencies.md     # Dependency setup
├── README_docker.md          # Docker setup
├── LICENSE                   # MIT License
├── .clang-format            # Clang format configuration
├── .clang-tidy              # Clang tidy configuration
├── .cmake-format.yaml       # CMake format configuration
├── gcovr.cfg                # Code coverage configuration
├── .gitignore               # Git ignore rules
├── .github/                 # GitHub Actions CI/CD
├── .devcontainer/           # VS Code devcontainer setup
├── cmake/                   # CMake modules and utilities
│   ├── CPM.cmake            # CPM package manager
│   ├── StaticAnalyzers.cmake
│   ├── Sanitizers.cmake
│   ├── Tests.cmake
│   ├── Hardening.cmake
│   └── ...                  # Other CMake utilities
├── configured_files/        # CMake configured files
├── include/                 # Public headers
│   └── sail/
│       └── sample_library.hpp
├── src/                     # Source code
│   ├── CMakeLists.txt
│   └── sample_library.cpp
├── apps/                    # Application executables
│   ├── CMakeLists.txt
│   └── sail.cpp            # Main application
├── test/                    # Unit tests
│   ├── CMakeLists.txt
│   ├── tests.cpp           # Main test file
│   └── constexpr_tests.cpp # Constexpr tests
├── fuzz_test/              # Fuzz testing
│   ├── CMakeLists.txt
│   └── fuzz_tester.cpp
├── build/                  # Build artifacts (generated)
└── out/                    # CMake preset build outputs
```

## Build Commands

### Basic Build Process

1. **Configure the project:**
   ```bash
   cmake -S . -B ./build
   ```

2. **Build the project:**
   ```bash
   cmake --build ./build
   ```

### Using CMake Presets (CMake 3.21+)

**Debug builds:**
```bash
# Unix-like systems (Linux/macOS)
cmake --preset unixlike-clang-debug
cmake --build out/build/unixlike-clang-debug

# Or with GCC
cmake --preset unixlike-gcc-debug
cmake --build out/build/unixlike-gcc-debug
```

**Release builds:**
```bash
# Unix-like systems
cmake --preset unixlike-clang-release
cmake --build out/build/unixlike-clang-release
```

### Testing Commands

**Run all tests:**
```bash
cd ./build
ctest -C Debug
```

**Run tests with preset:**
```bash
ctest --preset test-unixlike-clang-debug
```

**Run specific tests (using Catch2):**
```bash
./build/test/tests [test-name]
./build/test/tests --list-test-names-only
```

### Development Commands

**Static analysis (when ENABLE_DEVELOPER_MODE=ON):**
- clang-tidy runs automatically during build
- cppcheck runs automatically during build

**Code formatting:**
```bash
# Format all files (if clang-format is available)
find src include apps test -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

**Run application:**
```bash
./build/apps/sail
```

## Development Guidelines

### Template Adherence

This project follows the **cpp-best-practices/cmake_template**. When making changes:

1. **Maintain Template Structure**: Keep the existing directory structure and CMake organization
2. **Modular Changes**: Implement new features as isolated modules when possible
3. **Template Compatibility**: Ensure changes don't break the template's core functionality
4. **Documentation**: Document any deviations from the template with clear reasoning

### Code Standards

- **C++ Standard**: C++23 (set in CMakeLists.txt:9)
- **Formatting**: Use `.clang-format` configuration
- **Analysis**: Address clang-tidy warnings
- **Testing**: Write tests for new functionality using Catch2
- **Dependencies**: Manage through CPM in `Dependencies.cmake`

### Key Features from Template

- **Developer Mode**: Address Sanitizer, Undefined Behavior Sanitizer, warnings as errors
- **Static Analysis**: clang-tidy and cppcheck integration  
- **Testing Framework**: Catch2 for unit tests
- **Package Manager**: CPM for dependency management
- **CI/CD**: GitHub Actions with comprehensive testing matrix
- **Cross-platform**: Windows (MSVC/Clang) and Unix-like (GCC/Clang) support

### When to Modify Template Structure

Only modify the template structure when:
- Adding genuinely new functionality that cannot fit existing patterns
- Fixing bugs in the template itself
- Adapting to project-specific requirements that cannot be accommodated otherwise

Always document the reasoning and keep changes minimal and well-isolated.

## Common Tasks

- **Add new library**: Create in `src/` with corresponding header in `include/sail/`
- **Add new executable**: Create in `apps/` directory
- **Add new test**: Add to `test/tests.cpp` or create new test file
- **Add dependency**: Update `Dependencies.cmake` using CPM
- **Update build options**: Modify `ProjectOptions.cmake`

## Environment

- **Build System**: CMake 3.21+
- **Default Generator**: Ninja (specified in presets)
- **Supported Compilers**: GCC, Clang, MSVC
- **Package Manager**: CPM (CMake Package Manager)
- **Test Framework**: Catch2
- **Documentation**: Doxygen (optional)