#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace sail {

struct PackageInfo {
    std::string name;
    std::string url;
    std::string description;
    std::vector<std::string> binaries;  // Expected binary names
};

class PackageRegistry {
public:
    PackageRegistry();
    
    // Lookup package by name, returns URL if found
    std::string lookupPackage(const std::string& packageName) const;
    
    // Check if a string is a package name (not a URL)
    bool isPackageName(const std::string& input) const;
    
    // Get package info if available
    const PackageInfo* getPackageInfo(const std::string& packageName) const;
    
    // List all available packages
    std::vector<std::string> listPackages() const;
    
    // Get all package information
    std::vector<PackageInfo> getAllPackages() const;

private:
    std::unordered_map<std::string, PackageInfo> packages;
    
    void initializeBuiltinPackages();
};

} // namespace sail