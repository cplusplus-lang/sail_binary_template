#include "cmake_builder.h"
#include "utils.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace sail {

bool CMakeBuilder::configure(const std::string& sourceDir, const std::string& buildDir) {
    if (!Utils::createDirectoryRecursive(buildDir)) {
        std::cerr << "Failed to create build directory: " << buildDir << std::endl;
        return false;
    }
    
    std::string command = "cmake -S \"" + sourceDir + "\" -B \"" + buildDir + "\" -DCMAKE_BUILD_TYPE=Release";
    
    std::cout << "Configuring CMake project..." << std::endl;
    return runCommand(command);
}

bool CMakeBuilder::build(const std::string& buildDir) {
    std::string command = "cmake --build \"" + buildDir + "\" --config Release";
    
    std::cout << "Building project..." << std::endl;
    return runCommand(command);
}

std::vector<std::string> CMakeBuilder::findExecutables(const std::string& buildDir) {
    std::vector<std::string> executables;
    
    if (!Utils::directoryExists(buildDir)) {
        return executables;
    }
    
    findExecutablesRecursive(buildDir, executables);
    return executables;
}

bool CMakeBuilder::runCommand(const std::string& command, const std::string& workingDir) {
    std::string fullCommand = command;
    
    if (!workingDir.empty()) {
#ifdef SAIL_PLATFORM_WINDOWS
        fullCommand = "cd /d \"" + workingDir + "\" && " + command;
#else
        fullCommand = "cd \"" + workingDir + "\" && " + command;
#endif
    }
    
    int result = std::system(fullCommand.c_str());
    return result == 0;
}

void CMakeBuilder::findExecutablesRecursive(const std::string& dir, std::vector<std::string>& executables) {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                
                // Skip certain directories and file patterns
                std::string filename = entry.path().filename().string();
                std::string parentDir = entry.path().parent_path().filename().string();
                
                // Skip CMake generated files and common build artifacts
                if (filename.find("cmake") != std::string::npos ||
                    filename.find("CMake") != std::string::npos ||
                    parentDir == "CMakeFiles" ||
                    filename.find(".cmake") != std::string::npos ||
                    filename.find("Makefile") != std::string::npos ||
                    filename.find(".o") == filename.size() - 2 ||
                    filename.find(".obj") == filename.size() - 4) {
                    continue;
                }
                
                // Check if file is executable
                if (Utils::isExecutable(path)) {
                    executables.push_back(path);
                }
#ifdef SAIL_PLATFORM_WINDOWS
                // On Windows, also check files without .exe extension in build dirs
                else if (parentDir == "Release" || parentDir == "Debug" || 
                         parentDir == "RelWithDebInfo" || parentDir == "MinSizeRel") {
                    // This might be an executable without .exe extension, try it anyway
                    executables.push_back(path);
                }
#endif
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching for executables in " << dir << ": " << e.what() << std::endl;
    }
}

} // namespace sail