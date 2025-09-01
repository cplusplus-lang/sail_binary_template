# Sail - C++ Package Manager

[![Cross-Platform Tests](https://github.com/yourusername/sail/actions/workflows/cross-platform-tests.yml/badge.svg)](https://github.com/yourusername/sail/actions/workflows/cross-platform-tests.yml)

Sail is a C++ package manager inspired by Rust's Cargo, designed to simplify the installation of C++ packages built with CMake from Git repositories.

## Features

- 🚀 **Project Creation**: Create new C++ projects with modern structure
- 🔨 **Build System**: Build projects in debug or release mode with CMake
- 🏃 **Run Command**: Build and run projects with a single command
- 📦 **Package Installation**: Install C++ packages from Git repositories
- 🌍 **Cross-Platform**: Works on Windows, macOS, and Linux
- 🔧 **CMake Integration**: Automatically builds CMake projects with forwarding structure
- 📦 **Binary Management**: Installs executables to `~/.sail/bin`
- 🛡️ **Robust Error Handling**: Graceful failure recovery and detailed error messages
- 🧪 **Comprehensive Testing**: 96 test cases with cross-platform validation

## Quick Start

### Prerequisites

- **CMake** (3.21 or higher)
- **Git**
- **C++17 compatible compiler** (GCC, Clang, or MSVC)

### Installation

Clone and build Sail:

```bash
git clone https://github.com/yourusername/sail.git
cd sail
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make  # or cmake --build . on Windows
```

### Basic Usage

```bash
# Create a new project
./sail new my_project
./sail new my_lib --lib

# Build your project  
./sail build              # Debug build (default)
./sail build --release    # Release build
./sail build --verbose    # Verbose output

# Build and run your project
./sail run                # Build and run
./sail run --release      # Build and run in release mode
./sail run -- --help      # Pass arguments to your program

# Install a package by name (from built-in registry)
./sail install names

# Install a package from any Git URL
./sail install https://github.com/user/repository.git

# List available packages
./sail list

# Show help
./sail --help

# Show version
./sail --version
```

## Installation Directory

Sail installs binaries to:
- **Unix/Linux/macOS**: `~/.sail/bin`
- **Windows**: `%USERPROFILE%\.sail\bin`

Make sure to add this directory to your `PATH` environment variable to use installed binaries.

### Adding to PATH

**Bash/Zsh (Linux/macOS):**
```bash
echo 'export PATH="$HOME/.sail/bin:$PATH"' >> ~/.bashrc
# or ~/.zshrc for Zsh
source ~/.bashrc
```

**PowerShell (Windows):**
```powershell
$env:PATH += ";$env:USERPROFILE\.sail\bin"
# To make it permanent:
[Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";$env:USERPROFILE\.sail\bin", "User")
```

## Usage Examples

### Creating Projects

```bash
# Create a new binary project
sail new hello_world
cd hello_world
sail run  # Build and run the project

# Create a new library project
sail new my_library --lib
cd my_library
sail build --release  # Build the library

# Projects use a forwarding CMakeLists.txt structure:
# - Root CMakeLists.txt: Minimal forwarding file
# - build/cmake/CMakeLists.txt: Actual build configuration
```

### Building Projects

```bash
# Build in debug mode (default)
sail build

# Build in release mode with optimizations
sail build --release

# Build with verbose output
sail build --verbose

# Build artifacts are placed in:
# - build/debug/ (for debug builds)  
# - build/release/ (for release builds)
```

### Running Projects

```bash
# Build and run the project
sail run

# Build and run in release mode
sail run --release

# Run with arguments passed to your program
sail run -- arg1 arg2 --flag

# Run a specific binary
sail run --bin my_binary_name

# The run command automatically:
# 1. Builds the project if needed
# 2. Finds the executable
# 3. Runs it with provided arguments
```

### Installing Packages

```bash
# Install from built-in package registry
sail install names
sail install cppcheck
sail install vcpkg-tool

# List all available packages
sail list

# Install from GitHub HTTPS URL
sail install https://github.com/cplusplus-lang/names

# Install from GitHub SSH URL
sail install git@github.com:username/repository.git

# Install from any Git repository
sail install https://gitlab.com/user/project.git
```

### Built-in Package Registry

Sail includes a curated registry of C++ packages that provide command-line tools:

| Package | Description | Binaries |
|---------|-------------|----------|
| **names** | Generate random names in adjective-noun format | `names` |
| **cppcheck** | Static analysis tool for C/C++ code | `cppcheck` |
| **vcpkg-tool** | C++ Library Manager for Windows, Linux, and macOS | `vcpkg` |

*Note: The registry only includes packages that install actual binary executables. For header-only libraries or packages that only provide libraries (like json, fmt, spdlog, etc.), install directly from their Git URLs.*

### Using Installed Packages

After installation, binaries are available in your PATH:

```bash
# Example: using the 'names' package
names 3
# Output: 3 random adjective-noun combinations
# mighty-swing
# needy-star  
# abusive-collar

# Example: using cppcheck for static analysis
cppcheck --enable=all src/

# Example: using vcpkg for package management
vcpkg search json
```

## How It Works

1. **Clone**: Downloads the Git repository to a temporary directory
2. **Configure**: Runs `cmake` to configure the project
3. **Build**: Compiles the project using `cmake --build`
4. **Install**: Copies all executables to `~/.sail/bin`
5. **Cleanup**: Removes temporary build files

## Requirements for Packages

For a C++ project to be installable with Sail, it must:

- Have a `CMakeLists.txt` file in the root directory
- Build one or more executable targets
- Be compatible with CMake 3.10 or higher

## Development

### Building from Source

```bash
git clone https://github.com/yourusername/sail.git
cd sail

# Debug build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Release build
mkdir build-release && cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Running Tests

Sail includes comprehensive tests built with Catch2:

```bash
# Build and run all tests
cd build
ctest --output-on-failure

# Run specific test categories
ctest -R "utils"
ctest -R "git_utils"
ctest -R "install_command"
ctest -R "new_command"
ctest -R "build_command"
ctest -R "run_command"

# Run tests verbosely
ctest --verbose
```

### Test Coverage

- **96 total tests** across all components
- **100% pass rate** on supported platforms  
- **Cross-platform validation** on Windows, macOS, and Linux
- **Unit tests** for individual components
- **Integration tests** for end-to-end functionality
- **Mock tests** for isolated component testing
- **Registry tests** for package lookup and validation

## Architecture

### Core Components

- **CLI**: Command-line interface and argument parsing
- **NewCommand**: Project creation with modern CMake structure
- **BuildCommand**: Project building with debug/release modes
- **RunCommand**: Build-and-run functionality with argument passing
- **InstallCommand**: Package installation from Git repositories
- **PackageRegistry**: Built-in registry for common C++ packages
- **GitUtils**: Git operations (clone, repository detection)
- **CMakeBuilder**: CMake configuration and building
- **Utils**: Cross-platform file system utilities

### Directory Structure

```
sail/
├── CMakeLists.txt          # Main CMake configuration
├── README.md               # This file
├── include/                # Header files
│   ├── cli.h
│   ├── new_command.h
│   ├── build_command.h
│   ├── run_command.h
│   ├── install_command.h
│   ├── package_registry.h
│   ├── git_utils.h
│   ├── cmake_builder.h
│   └── utils.h
├── src/                    # Source files
│   ├── main.cpp
│   ├── cli.cpp
│   ├── new_command.cpp
│   ├── build_command.cpp
│   ├── run_command.cpp
│   ├── install_command.cpp
│   ├── package_registry.cpp
│   ├── git_utils.cpp
│   ├── cmake_builder.cpp
│   └── utils.cpp
├── tests/                  # Test files
│   ├── test_cli.cpp
│   ├── test_new_command.cpp
│   ├── test_build_command.cpp
│   ├── test_run_command.cpp
│   ├── test_install_command.cpp
│   ├── test_package_registry.cpp
│   ├── test_git_utils.cpp
│   ├── test_utils.cpp
│   └── test_main.cpp
└── .github/workflows/      # CI/CD configuration
    └── cross-platform-tests.yml
```

## Supported Platforms

| Platform | Status | Compiler | Notes |
|----------|--------|----------|-------|
| **Linux** | ✅ Supported | GCC, Clang | Tested on Ubuntu |
| **macOS** | ✅ Supported | Clang | Tested on macOS 14+ |
| **Windows** | ✅ Supported | MSVC, MinGW | Tested on Windows 11 |

## Error Handling

Sail provides detailed error messages for common issues:

- **Invalid URLs**: Clear message when Git URLs are malformed
- **Missing CMakeLists.txt**: Warns when project isn't CMake-compatible
- **Build failures**: Shows CMake/compiler errors
- **Network issues**: Handles clone failures gracefully
- **Permission errors**: Clear messages for directory creation issues

## Troubleshooting

### Common Issues

**"Git is required but not installed"**
- Install Git from [git-scm.com](https://git-scm.com/)

**"CMake is required but not installed"**
- Install CMake from [cmake.org](https://cmake.org/)

**"No executables found in build directory"**
- The target project may not build executables
- Check if the CMakeLists.txt defines `add_executable()` targets

**"Could not create install directory"**
- Check permissions for your home directory
- On Unix: `chmod 755 ~`

### Debug Mode

For detailed output during installation:

```bash
# Enable verbose CMake output
export CMAKE_VERBOSE_MAKEFILE=1
./sail install <repository-url>
```

## Contributing

We welcome contributions! Please see our contributing guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`ctest`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Code Style

- Follow existing code formatting
- Use meaningful variable and function names
- Add comments for complex logic
- Ensure cross-platform compatibility
- Write tests for new features

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by [Cargo](https://doc.rust-lang.org/cargo/) (Rust's package manager)
- Built with [CMake](https://cmake.org/) and [Google Test](https://github.com/google/googletest)
- Thanks to the C++ community for feedback and contributions

## Changelog

### v1.0.0 (Current)
- ✅ Initial release
- ✅ Basic install command
- ✅ Cross-platform support
- ✅ Comprehensive test suite
- ✅ GitHub Actions CI/CD

### v2.0.0 (Current)
- ✅ Project creation with `sail new`
- ✅ Build system with `sail build`
- ✅ Run command with `sail run`
- ✅ Forwarding CMakeLists.txt structure
- ✅ 96 comprehensive tests
- ✅ Catch2 test framework migration

### Planned Features  
- 🔄 Package versioning support
- 🔄 Dependency resolution
- 🔄 Local package registry
- 🔄 Uninstall command
- 🔄 Update command
- 🔄 Configuration file support

---

**Made with ❤️ for the C++ community**