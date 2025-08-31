#include "build_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>

namespace sail {

int BuildCommand::execute(const std::vector<std::string>& args) {
    // Check for help flag
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            printBuildHelp();
            return 0;
        }
    }
    
    // Determine build type (default is debug)
    bool release = isReleaseBuild(args);
    bool verbose = isVerbose(args);
    
    std::string buildDir = getBuildDir(release);
    
    std::cout << "Building " << (release ? "release" : "debug") << " configuration...\n";
    
    // Create build directory
    try {
        std::filesystem::create_directories(buildDir);
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to create build directory: " << e.what() << std::endl;
        return 1;
    }
    
    // Copy CPM.cmake if it exists
    if (!copyCPMFile(buildDir)) {
        std::cerr << "Warning: Could not copy CPM.cmake to build directory\n";
    }
    
    // Configure with CMake
    int configResult = runCMakeConfigure(buildDir, release);
    if (configResult != 0) {
        std::cerr << "Error: CMake configuration failed\n";
        return configResult;
    }
    
    // Build with CMake
    int buildResult = runCMakeBuild(buildDir, verbose);
    if (buildResult != 0) {
        std::cerr << "Error: CMake build failed\n";
        return buildResult;
    }
    
    std::cout << "Build completed successfully in " << buildDir << std::endl;
    return 0;
}

bool BuildCommand::isReleaseBuild(const std::vector<std::string>& args) const {
    for (const auto& arg : args) {
        if (arg == "--release" || arg == "-r") {
            return true;
        }
    }
    return false;
}

bool BuildCommand::isVerbose(const std::vector<std::string>& args) const {
    for (const auto& arg : args) {
        if (arg == "--verbose" || arg == "-v") {
            return true;
        }
    }
    return false;
}

std::string BuildCommand::getBuildDir(bool release) const {
    return release ? "build/release" : "build/debug";
}

bool BuildCommand::copyCPMFile(const std::string& buildDir) const {
    std::string sourcePath = "target/cmake/CPM.cmake";
    std::string destPath = buildDir + "/CPM.cmake";
    
    try {
        if (std::filesystem::exists(sourcePath)) {
            std::filesystem::copy_file(sourcePath, destPath, 
                std::filesystem::copy_options::overwrite_existing);
            return true;
        }
    } catch (const std::exception&) {
        // Ignore copy errors, just return false
    }
    
    return false;
}

int BuildCommand::runCMakeConfigure(const std::string& buildDir, bool release) const {
    std::string buildType = release ? "Release" : "Debug";
    std::string command = "cmake -B " + buildDir + " -DCMAKE_BUILD_TYPE=" + buildType;
    
    std::cout << "Configuring: " << command << std::endl;
    return std::system(command.c_str());
}

int BuildCommand::runCMakeBuild(const std::string& buildDir, bool verbose) const {
    std::string command = "cmake --build " + buildDir;
    if (verbose) {
        command += " --verbose";
    }
    
    std::cout << "Building: " << command << std::endl;
    return std::system(command.c_str());
}

void BuildCommand::printBuildHelp() const {
    std::cout << "Build the current project\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail build [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help       Print help information\n";
    std::cout << "    -r, --release    Build artifacts in release mode, with optimizations\n";
    std::cout << "    -v, --verbose    Use verbose output\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail build              # Build in debug mode (default)\n";
    std::cout << "    sail build --release    # Build in release mode\n";
    std::cout << "    sail build -v           # Build with verbose output\n\n";
    std::cout << "Build artifacts will be placed in:\n";
    std::cout << "    build/debug/    (for debug builds)\n";
    std::cout << "    build/release/  (for release builds)\n";
}

} // namespace sail