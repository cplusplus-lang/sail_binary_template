#include "build_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <fstream>
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
    
    // Find project root and change to it
    std::string projectRoot = Utils::findProjectRoot();
    std::filesystem::path originalDir = std::filesystem::current_path();
    if (originalDir != std::filesystem::absolute(projectRoot)) {
        try {
            std::filesystem::current_path(projectRoot);
            std::cout << "Changed to project root: " << projectRoot << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not change to project root: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // Determine build type (default is debug)
    bool release = isReleaseBuild(args);
    bool verbose = isVerbose(args);
    
    std::string buildDir = getBuildDir(release);
    
    // Validate C++ standard if Sail.toml exists
    if (std::filesystem::exists("Sail.toml")) {
        std::string cppStandard = getCppStandardFromToml();
        if (!Utils::isValidCppStandard(cppStandard)) {
            std::cerr << "Error: Invalid C++ standard '" << cppStandard << "' in Sail.toml\n";
            std::cerr << "Valid options are: 98, 03, 11, 14, 17, 20, 23, 26\n";
            return 1;
        }
    }
    
    // Ensure CMake structure exists before building
    if (!ensureCMakeStructure()) {
        std::cerr << "Error: Failed to set up CMake project structure\n";
        return 1;
    }
    
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

bool BuildCommand::ensureCMakeStructure() const {
    // Check if Sail.toml exists - if not, this is a regular CMake project
    if (!std::filesystem::exists("Sail.toml")) {
        // No Sail.toml found - assume this is a regular CMake project, skip generation
        return true;
    }
    
    // Only generate CMake files if they don't exist or are outdated
    bool needsGeneration = false;
    
    if (!std::filesystem::exists("CMakeLists.txt")) {
        needsGeneration = true;
    }
    
    if (!std::filesystem::exists("build/cmake")) {
        needsGeneration = true;
    }
    
    if (needsGeneration) {
        std::cout << "Setting up CMake project structure...\n";
        
        // Create build/cmake directory
        if (!createBuildCMakeDirectory()) {
            return false;
        }
        
        // Create root CMakeLists.txt
        if (!createRootCMakeListsFile()) {
            return false;
        }
        
        std::cout << "CMake project structure created.\n";
    }
    
    return true;
}

bool BuildCommand::createRootCMakeListsFile() const {
    std::ofstream file("CMakeLists.txt");
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create CMakeLists.txt\n";
        return false;
    }
    
    file << "# Forwarding CMakeLists.txt - DO NOT EDIT\n";
    file << "# The actual CMake configuration is in build/cmake/CMakeLists.txt\n\n";
    file << "cmake_minimum_required(VERSION 3.21)\n\n";
    file << "# Parse Sail.toml for project name and version\n";
    file << "include(build/cmake/SailToml.cmake)\n";
    file << "sail_parse_toml()\n\n";
    file << "project(${SAIL_PROJECT_NAME} VERSION ${SAIL_PROJECT_VERSION})\n\n";
    file << "include(build/cmake/CMakeLists.txt)\n";
    
    file.close();
    return true;
}

bool BuildCommand::createBuildCMakeDirectory() const {
    std::string buildCmakeDir = "build/cmake";
    
    if (!Utils::createDirectoryRecursive(buildCmakeDir)) {
        std::cerr << "Error: Could not create build/cmake directory\n";
        return false;
    }
    
    // Create the TOML parsing module
    if (!createSailTomlModule(buildCmakeDir)) {
        return false;
    }
    
    // Create the actual CMakeLists.txt
    std::string projectName = getProjectNameFromToml();
    bool isBin = !isLibraryProject();
    
    if (!createBuildCMakeListsFile(buildCmakeDir, projectName, isBin)) {
        return false;
    }
    
    return true;
}

bool BuildCommand::createSailTomlModule(const std::string& buildCmakeDir) const {
    std::string moduleFile = buildCmakeDir + "/SailToml.cmake";
    std::ofstream file(moduleFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create SailToml.cmake\n";
        return false;
    }
    
    file << "# Sail TOML Parser Module\n";
    file << "# Parses Sail.toml file for project name and version\n\n";
    file << "function(sail_parse_toml)\n";
    file << "    # Check if Sail.toml exists\n";
    file << "    if(NOT EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/Sail.toml\")\n";
    file << "        message(FATAL_ERROR \"Sail.toml not found in project root\")\n";
    file << "    endif()\n\n";
    file << "    # Read the TOML file\n";
    file << "    file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/Sail.toml\" SAIL_TOML_CONTENT)\n\n";
    file << "    # Parse project name (look under [package] section)\n";
    file << "    string(REGEX MATCH \"\\\\[package\\\\][^\\\\[]*name = \\\"([^\\\"]+)\\\"\" _ \"${SAIL_TOML_CONTENT}\")\n";
    file << "    if(NOT CMAKE_MATCH_1)\n";
    file << "        message(FATAL_ERROR \"Could not parse project name from Sail.toml\")\n";
    file << "    endif()\n";
    file << "    set(SAIL_PROJECT_NAME \"${CMAKE_MATCH_1}\" PARENT_SCOPE)\n\n";
    file << "    # Parse project version (look under [package] section)\n";
    file << "    string(REGEX MATCH \"\\\\[package\\\\][^\\\\[]*version = \\\"([^\\\"]+)\\\"\" _ \"${SAIL_TOML_CONTENT}\")\n";
    file << "    if(NOT CMAKE_MATCH_1)\n";
    file << "        message(WARNING \"Could not parse project version from Sail.toml, using 1.0.0\")\n";
    file << "        set(SAIL_PROJECT_VERSION \"1.0.0\" PARENT_SCOPE)\n";
    file << "    else()\n";
    file << "        set(SAIL_PROJECT_VERSION \"${CMAKE_MATCH_1}\" PARENT_SCOPE)\n";
    file << "    endif()\n\n";
    file << "    # Parse C++ standard (look under [package] section)\n";
    file << "    string(REGEX MATCH \"\\\\[package\\\\][^\\\\[]*standard = \\\"([^\\\"]+)\\\"\" _ \"${SAIL_TOML_CONTENT}\")\n";
    file << "    if(NOT CMAKE_MATCH_1)\n";
    file << "        message(WARNING \"Could not parse C++ standard from Sail.toml, using 17\")\n";
    file << "        set(SAIL_CPP_STANDARD \"17\" PARENT_SCOPE)\n";
    file << "    else()\n";
    file << "        set(SAIL_CPP_STANDARD \"${CMAKE_MATCH_1}\" PARENT_SCOPE)\n";
    file << "        # Validate the C++ standard\n";
    file << "        set(VALID_STANDARDS 98 03 11 14 17 20 23 26)\n";
    file << "        if(NOT \"${CMAKE_MATCH_1}\" IN_LIST VALID_STANDARDS)\n";
    file << "            message(FATAL_ERROR \"Invalid C++ standard '${CMAKE_MATCH_1}'. Valid options are: ${VALID_STANDARDS}\")\n";
    file << "        endif()\n";
    file << "    endif()\n\n";
    file << "    # Debug output (optional)\n";
    file << "    message(STATUS \"Sail project: ${SAIL_PROJECT_NAME} v${SAIL_PROJECT_VERSION}\")\n";
    file << "endfunction()\n";
    
    file.close();
    return true;
}

bool BuildCommand::createBuildCMakeListsFile(const std::string& buildCmakeDir, const std::string& projectName, bool isBin) const {
    std::string cmakeFile = buildCmakeDir + "/CMakeLists.txt";
    std::ofstream file(cmakeFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create build/cmake/CMakeLists.txt\n";
        return false;
    }
    
    file << "# Use project name and C++ standard from Sail.toml\n";
    file << "set(CMAKE_CXX_STANDARD ${SAIL_CPP_STANDARD})\n";
    file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    
    if (isBin) {
        file << "# Collect all source files from src directory\n";
        file << "file(GLOB_RECURSE PROJECT_SOURCES \"src/*.cpp\")\n\n";
        file << "add_executable(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES})\n\n";
        file << "# Add include directory if it exists\n";
        file << "if(EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/include\")\n";
        file << "    target_include_directories(${SAIL_PROJECT_NAME} PRIVATE include)\n";
        file << "endif()\n";
    } else {
        file << "# Collect all source files from src directory\n";
        file << "file(GLOB_RECURSE PROJECT_SOURCES \"src/*.cpp\")\n\n";
        file << "add_library(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES})\n\n";
        file << "target_include_directories(${SAIL_PROJECT_NAME} PUBLIC include)\n";
    }
    
    file.close();
    return true;
}

bool BuildCommand::isLibraryProject() const {
    // Check if this is a library project by looking for include directory
    return std::filesystem::exists("include");
}

std::string BuildCommand::getProjectNameFromToml() const {
    // Simple TOML parsing to get project name
    std::ifstream file("Sail.toml");
    if (!file.is_open()) {
        return "unknown_project";
    }
    
    std::string line;
    bool inPackageSection = false;
    
    while (std::getline(file, line)) {
        if (line.find("[package]") != std::string::npos) {
            inPackageSection = true;
            continue;
        }
        
        if (inPackageSection && line.find("[") != std::string::npos && line.find("[package]") == std::string::npos) {
            // We've left the package section
            break;
        }
        
        if (inPackageSection && line.find("name = \"") != std::string::npos) {
            size_t start = line.find("name = \"") + 8;
            size_t end = line.find("\"", start);
            if (end != std::string::npos) {
                return line.substr(start, end - start);
            }
        }
    }
    
    return "unknown_project";
}

std::string BuildCommand::getCppStandardFromToml() const {
    // Simple TOML parsing to get C++ standard
    std::ifstream file("Sail.toml");
    if (!file.is_open()) {
        return "17"; // default
    }
    
    std::string line;
    bool inPackageSection = false;
    
    while (std::getline(file, line)) {
        if (line.find("[package]") != std::string::npos) {
            inPackageSection = true;
            continue;
        }
        
        if (inPackageSection && line.find("[") != std::string::npos && line.find("[package]") == std::string::npos) {
            // We've left the package section
            break;
        }
        
        if (inPackageSection && line.find("standard = \"") != std::string::npos) {
            size_t start = line.find("standard = \"") + 12;
            size_t end = line.find("\"", start);
            if (end != std::string::npos) {
                return line.substr(start, end - start);
            }
        }
    }
    
    return "17"; // default
}

} // namespace sail