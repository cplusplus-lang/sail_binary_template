#include "package_registry.h"
#include <algorithm>

namespace sail {

PackageRegistry::PackageRegistry() {
    initializeBuiltinPackages();
}

std::string PackageRegistry::lookupPackage(const std::string& packageName) const {
    auto it = packages.find(packageName);
    if (it != packages.end()) {
        return it->second.url;
    }
    return ""; // Not found
}

bool PackageRegistry::isPackageName(const std::string& input) const {
    // Check if it's a URL (contains :// or starts with git@)
    if (input.find("://") != std::string::npos || 
        input.find("git@") == 0) {
        return false;
    }
    
    // Check if it's a package name we know about
    return packages.find(input) != packages.end();
}

const PackageInfo* PackageRegistry::getPackageInfo(const std::string& packageName) const {
    auto it = packages.find(packageName);
    if (it != packages.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PackageRegistry::listPackages() const {
    std::vector<std::string> result;
    result.reserve(packages.size());
    
    for (const auto& pair : packages) {
        result.push_back(pair.first);
    }
    
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<PackageInfo> PackageRegistry::getAllPackages() const {
    std::vector<PackageInfo> result;
    result.reserve(packages.size());
    
    for (const auto& pair : packages) {
        result.push_back(pair.second);
    }
    
    // Sort by name
    std::sort(result.begin(), result.end(), 
              [](const PackageInfo& a, const PackageInfo& b) {
                  return a.name < b.name;
              });
    
    return result;
}

void PackageRegistry::initializeBuiltinPackages() {
    // Only packages that produce actual binary executables
    packages["names"] = PackageInfo{
        "names",
        "https://github.com/cplusplus-lang/names.git",
        "Generate random names in adjective-noun format",
        {"names"}
    };
    
    packages["vcpkg-tool"] = PackageInfo{
        "vcpkg-tool",
        "https://github.com/Microsoft/vcpkg-tool.git",
        "C++ Library Manager for Windows, Linux, and MacOS",
        {"vcpkg"}
    };
    
    packages["cppcheck"] = PackageInfo{
        "cppcheck",
        "https://github.com/danmar/cppcheck.git", 
        "Static analysis tool for C/C++ code",
        {"cppcheck"}
    };
}

} // namespace sail