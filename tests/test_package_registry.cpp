#include <catch2/catch_test_macros.hpp>
#include "package_registry.h"
#include <algorithm>
#include <memory>

class PackageRegistryTestFixture {
public:
    PackageRegistryTestFixture() {
        registry = std::make_unique<sail::PackageRegistry>();
    }

    std::unique_ptr<sail::PackageRegistry> registry;
};

TEST_CASE("PackageRegistry::lookupPackage returns URL for known package", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    std::string url = fixture.registry->lookupPackage("names");
    REQUIRE(url == "https://github.com/cplusplus-lang/names.git");
}

TEST_CASE("PackageRegistry::lookupPackage returns empty for unknown package", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    std::string url = fixture.registry->lookupPackage("nonexistent-package");
    REQUIRE(url.empty());
}

TEST_CASE("PackageRegistry::isPackageName recognizes valid names", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    REQUIRE(fixture.registry->isPackageName("names"));
    REQUIRE(fixture.registry->isPackageName("cppcheck"));
    REQUIRE(fixture.registry->isPackageName("vcpkg-tool"));
}

TEST_CASE("PackageRegistry::isPackageName rejects URLs", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    REQUIRE_FALSE(fixture.registry->isPackageName("https://github.com/user/repo.git"));
    REQUIRE_FALSE(fixture.registry->isPackageName("http://example.com/repo"));
    REQUIRE_FALSE(fixture.registry->isPackageName("git@github.com:user/repo.git"));
}

TEST_CASE("PackageRegistry::isPackageName rejects unknown names", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    REQUIRE_FALSE(fixture.registry->isPackageName("unknown-package-that-does-not-exist"));
}

TEST_CASE("PackageRegistry::getPackageInfo returns valid info", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    const auto* info = fixture.registry->getPackageInfo("names");
    REQUIRE(info != nullptr);
    REQUIRE(info->name == "names");
    REQUIRE(info->url == "https://github.com/cplusplus-lang/names.git");
    REQUIRE_FALSE(info->description.empty());
    REQUIRE(std::find(info->binaries.begin(), info->binaries.end(), "names") != info->binaries.end());
}

TEST_CASE("PackageRegistry::getPackageInfo returns null for unknown package", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    const auto* info = fixture.registry->getPackageInfo("nonexistent");
    REQUIRE(info == nullptr);
}

TEST_CASE("PackageRegistry::listPackages returns known packages", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->listPackages();
    REQUIRE_FALSE(packages.empty());
    
    // Should contain some expected packages
    REQUIRE(std::find(packages.begin(), packages.end(), "names") != packages.end());
    REQUIRE(std::find(packages.begin(), packages.end(), "cppcheck") != packages.end());
    REQUIRE(std::find(packages.begin(), packages.end(), "vcpkg-tool") != packages.end());
    
    // Should be sorted
    auto sortedPackages = packages;
    std::sort(sortedPackages.begin(), sortedPackages.end());
    REQUIRE(packages == sortedPackages);
}

TEST_CASE("PackageRegistry::getAllPackages returns detailed info", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->getAllPackages();
    REQUIRE_FALSE(packages.empty());
    
    // Find the names package
    auto it = std::find_if(packages.begin(), packages.end(),
                          [](const sail::PackageInfo& pkg) { return pkg.name == "names"; });
    
    REQUIRE(it != packages.end());
    REQUIRE(it->url == "https://github.com/cplusplus-lang/names.git");
    REQUIRE_FALSE(it->description.empty());
}

TEST_CASE("PackageRegistry packages are sorted by name", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->getAllPackages();
    REQUIRE_FALSE(packages.empty());
    
    for (size_t i = 1; i < packages.size(); ++i) {
        REQUIRE(packages[i-1].name < packages[i].name);
    }
}

TEST_CASE("PackageRegistry contains expected packages", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->listPackages();
    
    // Test for some specific packages that should be in the registry
    std::vector<std::string> expectedPackages = {
        "names", "cppcheck", "vcpkg-tool"
    };
    
    for (const auto& expected : expectedPackages) {
        REQUIRE(std::find(packages.begin(), packages.end(), expected) != packages.end());
        
        // Also verify the package has valid URL
        std::string url = fixture.registry->lookupPackage(expected);
        REQUIRE_FALSE(url.empty());
    }
}

TEST_CASE("PackageRegistry package URLs are valid", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->getAllPackages();
    
    for (const auto& pkg : packages) {
        REQUIRE_FALSE(pkg.url.empty());
        
        // URL should contain a protocol
        REQUIRE((pkg.url.find("://") != std::string::npos || 
                pkg.url.find("git@") == 0));
        
        REQUIRE_FALSE(pkg.description.empty());
    }
}

TEST_CASE("PackageRegistry package names are valid", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    auto packages = fixture.registry->getAllPackages();
    
    for (const auto& pkg : packages) {
        REQUIRE_FALSE(pkg.name.empty());
        
        // Package names should not contain spaces or special characters
        REQUIRE(pkg.name.find(' ') == std::string::npos);
        
        // Should only contain lowercase letters, numbers, and hyphens
        for (char c : pkg.name) {
            REQUIRE((std::islower(c) || std::isdigit(c) || c == '-' || c == '_'));
        }
    }
}

TEST_CASE("PackageRegistry specific package details", "[package_registry]") {
    PackageRegistryTestFixture fixture;
    // Test specific details of the names package
    const auto* namesInfo = fixture.registry->getPackageInfo("names");
    REQUIRE(namesInfo != nullptr);
    REQUIRE(namesInfo->name == "names");
    REQUIRE(namesInfo->url == "https://github.com/cplusplus-lang/names.git");
    REQUIRE(namesInfo->description.find("random names") != std::string::npos);
    REQUIRE(namesInfo->binaries.size() == 1);
    REQUIRE(namesInfo->binaries[0] == "names");
    
    // Test a static analysis tool  
    const auto* cppcheckInfo = fixture.registry->getPackageInfo("cppcheck");
    REQUIRE(cppcheckInfo != nullptr);
    REQUIRE(cppcheckInfo->name == "cppcheck");
    REQUIRE(cppcheckInfo->url == "https://github.com/danmar/cppcheck.git");
    REQUIRE(cppcheckInfo->binaries.size() == 1);
    REQUIRE(cppcheckInfo->binaries[0] == "cppcheck");
}