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

int InstallCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: install command requires a package name or URL\n";
        std::cerr << "Usage: sail install [OPTIONS] <package-name|git-url>\n";
        std::cerr << "       sail list                    # Show available packages\n";
        std::cerr << "Options:\n";
        std::cerr << "    --full-clone    Download complete repository with git history\n";
        return 1;
    }
    
    // Parse command line arguments
    bool fullClone = false;
    std::string packageInput;
    
    for (const auto& arg : args) {
        if (arg == "--full-clone") {
            fullClone = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Install a package from a Git repository or package name\n\n";
            std::cout << "Usage: sail install [OPTIONS] <package-name|git-url>\n\n";
            std::cout << "Arguments:\n";
            std::cout << "    <package-name|git-url>    Package name from registry or Git URL\n\n";
            std::cout << "Options:\n";
            std::cout << "    --full-clone              Download complete repository with git history\n";
            std::cout << "    -h, --help                Print help information\n\n";
            std::cout << "Examples:\n";
            std::cout << "    sail install names                                      # Install from registry\n";
            std::cout << "    sail install https://github.com/user/repo.git          # Install from URL (shallow)\n";
            std::cout << "    sail install --full-clone https://github.com/user/repo.git  # Install with full history\n";
            return 0;
        } else if (arg.substr(0, 2) == "--") {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            return 1;
        } else if (packageInput.empty()) {
            packageInput = arg;
        } else {
            std::cerr << "Error: Multiple package arguments specified\n";
            return 1;
        }
    }
    
    if (packageInput.empty()) {
        std::cerr << "Error: No package name or URL specified\n";
        return 1;
    }
    
    // Store the clone preference for use in cloneRepository
    m_fullClone = fullClone;
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

bool InstallCommand::cloneRepository(const std::string& url, const std::string& targetDir, bool shallow) {
    // Override shallow parameter with member variable preference
    // By default use shallow clone (like Cargo) unless --full-clone is specified
    bool useShallow = shallow && !m_fullClone;
    
    if (useShallow) {
        std::cout << "Note: Using shallow clone for faster download (source code only)" << std::endl;
        std::cout << "      Use --full-clone to download complete git history if needed" << std::endl;
    } else if (m_fullClone) {
        std::cout << "Note: Using full clone (complete git history)" << std::endl;
    }
    
    return GitUtils::clone(url, targetDir, useShallow);  // GitUtils: true = shallow, false = full
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