#include "utils.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef SAIL_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

namespace sail {

std::string Utils::getHomeDirectory() {
#ifdef SAIL_PLATFORM_WINDOWS
    char* userProfile = nullptr;
    size_t len = 0;
    if (_dupenv_s(&userProfile, &len, "USERPROFILE") == 0 && userProfile != nullptr) {
        std::string home(userProfile);
        free(userProfile);
        return home;
    }
    return "";
#else
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home);
    }
    
    // Fallback to getpwuid
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }
    
    return "";
#endif
}

std::string Utils::getSailDirectory() {
    std::string home = getHomeDirectory();
    if (home.empty()) {
        return "";
    }
    
#ifdef SAIL_PLATFORM_WINDOWS
    return home + "\\.sail";
#else
    return home + "/.sail";
#endif
}

std::string Utils::getSailBinDirectory() {
    std::string sailDir = getSailDirectory();
    if (sailDir.empty()) {
        return "";
    }
    
#ifdef SAIL_PLATFORM_WINDOWS
    return sailDir + "\\bin";
#else
    return sailDir + "/bin";
#endif
}

bool Utils::createDirectoryRecursive(const std::string& path) {
    try {
        // create_directories returns false if directory already exists
        // but that should be considered success, not failure
        std::filesystem::create_directories(path);
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory " << path << ": " << e.what() << std::endl;
        return false;
    }
}

bool Utils::fileExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

bool Utils::directoryExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

std::string Utils::getTemporaryDirectory() {
    return std::filesystem::temp_directory_path().string();
}

void Utils::removeDirectory(const std::string& path) {
    try {
        std::filesystem::remove_all(path);
    } catch (const std::exception& e) {
        std::cerr << "Error removing directory " << path << ": " << e.what() << std::endl;
    }
}

bool Utils::copyFile(const std::string& source, const std::string& destination) {
    try {
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying file from " << source << " to " << destination << ": " << e.what() << std::endl;
        return false;
    }
}

bool Utils::isExecutable(const std::string& path) {
#ifdef SAIL_PLATFORM_WINDOWS
    // On Windows, check for .exe extension
    return path.size() >= 4 && path.substr(path.size() - 4) == ".exe";
#else
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        return (fileStat.st_mode & S_IXUSR) != 0;
    }
    return false;
#endif
}

void Utils::makeExecutable(const std::string& path) {
#ifndef SAIL_PLATFORM_WINDOWS
    chmod(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

std::string Utils::findProjectRoot(const std::string& startPath) {
    std::filesystem::path currentPath = std::filesystem::absolute(startPath);
    
    // Search upwards for project indicators
    while (currentPath != currentPath.root_path()) {
        // Check for Sail project
        if (std::filesystem::exists(currentPath / "Sail.toml")) {
            return currentPath.string();
        }
        
        // Check for regular CMake project
        if (std::filesystem::exists(currentPath / "CMakeLists.txt")) {
            return currentPath.string();
        }
        
        // Move up one directory
        currentPath = currentPath.parent_path();
    }
    
    // If no project root found, return the original starting path
    return std::filesystem::absolute(startPath).string();
}

} // namespace sail