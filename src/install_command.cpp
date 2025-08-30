#include "install_command.h"
#include "utils.h"
#include "git_utils.h"
#include "cmake_builder.h"
#include "package_registry.h"
#include <iostream>
#include <filesystem>
#include <random>
#include <sstream>

namespace sail {

int InstallCommand::execute(const std::string& packageInput) {
    PackageRegistry registry;
    std::string packageUrl = packageInput;
    std::string packageName;
    
    // Check if input is a package name or URL
    if (registry.isPackageName(packageInput)) {
        packageName = packageInput;
        packageUrl = registry.lookupPackage(packageInput);
        if (packageUrl.empty()) {
            std::cerr << "Error: Unknown package '" << packageInput << "'" << std::endl;
            std::cerr << "Available packages: ";
            auto packages = registry.listPackages();
            for (size_t i = 0; i < packages.size(); ++i) {
                std::cerr << packages[i];
                if (i < packages.size() - 1) std::cerr << ", ";
            }
            std::cerr << std::endl;
            return 1;
        }
        std::cout << "Installing package '" << packageName << "' from: " << packageUrl << std::endl;
    } else if (packageInput.find("://") != std::string::npos || packageInput.find("git@") == 0) {
        std::cout << "Installing package from: " << packageUrl << std::endl;
        // Extract package name from URL for display
        packageName = GitUtils::extractRepoName(packageUrl);
    } else {
        // Not a known package name and not a URL - show error
        std::cerr << "Error: '" << packageInput << "' is not a known package name or valid Git URL" << std::endl;
        std::cerr << "Available packages: ";
        auto packages = registry.listPackages();
        for (size_t i = 0; i < packages.size(); ++i) {
            std::cerr << packages[i];
            if (i < packages.size() - 1) std::cerr << ", ";
        }
        std::cerr << std::endl;
        std::cerr << "Or use: sail install <git-url>" << std::endl;
        return 1;
    }
    
    // Get install directory and create if necessary
    std::string installDir = getInstallDir();
    if (installDir.empty()) {
        std::cerr << "Error: Could not determine install directory" << std::endl;
        return 1;
    }
    
    if (!Utils::createDirectoryRecursive(installDir)) {
        std::cerr << "Error: Could not create install directory: " << installDir << std::endl;
        return 1;
    }
    
    // Create temporary directory for the build
    std::string tempDir = createTempDir();
    if (tempDir.empty()) {
        std::cerr << "Error: Could not create temporary directory" << std::endl;
        return 1;
    }
    
    std::string repoName = GitUtils::extractRepoName(packageUrl);
    std::string sourceDir = tempDir + "/" + repoName;
    std::string buildDir = tempDir + "/" + repoName + "-build";
    
    bool success = false;
    
    do {
        // Clone the repository
        if (!cloneRepository(packageUrl, sourceDir)) {
            std::cerr << "Error: Failed to clone repository" << std::endl;
            break;
        }
        
        // Build the project
        if (!buildProject(sourceDir, buildDir)) {
            std::cerr << "Error: Failed to build project" << std::endl;
            break;
        }
        
        // Install binaries
        if (!installBinaries(buildDir, installDir)) {
            std::cerr << "Error: Failed to install binaries" << std::endl;
            break;
        }
        
        success = true;
        std::cout << "Successfully installed " << (packageName.empty() ? repoName : packageName) << " to " << installDir << std::endl;
        std::cout << "Make sure " << installDir << " is in your PATH to use the installed binaries." << std::endl;
        
    } while (false);
    
    // Cleanup temporary directory
    cleanupTempDir(tempDir);
    
    return success ? 0 : 1;
}

std::string InstallCommand::getInstallDir() const {
    return Utils::getSailBinDirectory();
}

std::string InstallCommand::createTempDir() const {
    std::string tempBase = Utils::getTemporaryDirectory();
    
    // Create unique temporary directory name
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::stringstream ss;
    ss << tempBase << "/sail-" << dis(gen);
    std::string tempDir = ss.str();
    
    if (!Utils::createDirectoryRecursive(tempDir)) {
        return "";
    }
    
    return tempDir;
}

bool InstallCommand::cloneRepository(const std::string& url, const std::string& targetDir) {
    return GitUtils::clone(url, targetDir);
}

bool InstallCommand::buildProject(const std::string& sourceDir, const std::string& buildDir) {
    // Check if CMakeLists.txt exists
    std::string cmakeFile = sourceDir + "/CMakeLists.txt";
    if (!Utils::fileExists(cmakeFile)) {
        std::cerr << "Error: No CMakeLists.txt found in the repository" << std::endl;
        return false;
    }
    
    CMakeBuilder builder;
    
    // Configure the project
    if (!builder.configure(sourceDir, buildDir)) {
        std::cerr << "Error: CMake configuration failed" << std::endl;
        return false;
    }
    
    // Build the project
    if (!builder.build(buildDir)) {
        std::cerr << "Error: Build failed" << std::endl;
        return false;
    }
    
    return true;
}

bool InstallCommand::installBinaries(const std::string& buildDir, const std::string& installDir) {
    CMakeBuilder builder;
    std::vector<std::string> executables = builder.findExecutables(buildDir);
    
    if (executables.empty()) {
        std::cerr << "Error: No executables found in build directory" << std::endl;
        return false;
    }
    
    std::cout << "Found " << executables.size() << " executable(s)" << std::endl;
    
    bool allSuccess = true;
    for (const std::string& executable : executables) {
        std::string filename = std::filesystem::path(executable).filename().string();
        std::string destination = installDir + "/" + filename;
        
        std::cout << "Installing " << filename << "..." << std::endl;
        
        if (!Utils::copyFile(executable, destination)) {
            std::cerr << "Error: Failed to copy " << executable << " to " << destination << std::endl;
            allSuccess = false;
            continue;
        }
        
        // Make sure the binary is executable
        Utils::makeExecutable(destination);
        
        std::cout << "Installed " << filename << " to " << destination << std::endl;
    }
    
    return allSuccess;
}

void InstallCommand::cleanupTempDir(const std::string& tempDir) {
    std::cout << "Cleaning up temporary files..." << std::endl;
    Utils::removeDirectory(tempDir);
}

} // namespace sail