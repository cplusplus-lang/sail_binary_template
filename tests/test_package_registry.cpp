#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "package_registry.h"

class PackageRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        registry = std::make_unique<sail::PackageRegistry>();
    }

    std::unique_ptr<sail::PackageRegistry> registry;
};

TEST_F(PackageRegistryTest, LookupKnownPackageReturnsUrl) {
    std::string url = registry->lookupPackage("names");
    EXPECT_EQ(url, "https://github.com/cplusplus-lang/names.git");
}

TEST_F(PackageRegistryTest, LookupUnknownPackageReturnsEmpty) {
    std::string url = registry->lookupPackage("nonexistent-package");
    EXPECT_TRUE(url.empty());
}

TEST_F(PackageRegistryTest, IsPackageNameRecognizesValidNames) {
    EXPECT_TRUE(registry->isPackageName("names"));
    EXPECT_TRUE(registry->isPackageName("cppcheck"));
    EXPECT_TRUE(registry->isPackageName("vcpkg-tool"));
}

TEST_F(PackageRegistryTest, IsPackageNameRejectsUrls) {
    EXPECT_FALSE(registry->isPackageName("https://github.com/user/repo.git"));
    EXPECT_FALSE(registry->isPackageName("http://example.com/repo"));
    EXPECT_FALSE(registry->isPackageName("git@github.com:user/repo.git"));
}

TEST_F(PackageRegistryTest, IsPackageNameRejectsUnknownNames) {
    EXPECT_FALSE(registry->isPackageName("unknown-package-that-does-not-exist"));
}

TEST_F(PackageRegistryTest, GetPackageInfoReturnsValidInfo) {
    const auto* info = registry->getPackageInfo("names");
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->name, "names");
    EXPECT_EQ(info->url, "https://github.com/cplusplus-lang/names.git");
    EXPECT_FALSE(info->description.empty());
    EXPECT_THAT(info->binaries, ::testing::Contains("names"));
}

TEST_F(PackageRegistryTest, GetPackageInfoReturnsNullForUnknownPackage) {
    const auto* info = registry->getPackageInfo("nonexistent");
    EXPECT_EQ(info, nullptr);
}

TEST_F(PackageRegistryTest, ListPackagesReturnsKnownPackages) {
    auto packages = registry->listPackages();
    EXPECT_FALSE(packages.empty());
    
    // Should contain some expected packages
    EXPECT_THAT(packages, ::testing::Contains("names"));
    EXPECT_THAT(packages, ::testing::Contains("cppcheck"));
    EXPECT_THAT(packages, ::testing::Contains("vcpkg-tool"));
    
    // Should be sorted
    auto sortedPackages = packages;
    std::sort(sortedPackages.begin(), sortedPackages.end());
    EXPECT_EQ(packages, sortedPackages);
}

TEST_F(PackageRegistryTest, GetAllPackagesReturnsDetailedInfo) {
    auto packages = registry->getAllPackages();
    EXPECT_FALSE(packages.empty());
    
    // Find the names package
    auto it = std::find_if(packages.begin(), packages.end(),
                          [](const sail::PackageInfo& pkg) { return pkg.name == "names"; });
    
    ASSERT_NE(it, packages.end());
    EXPECT_EQ(it->url, "https://github.com/cplusplus-lang/names.git");
    EXPECT_FALSE(it->description.empty());
}

TEST_F(PackageRegistryTest, PackagesAreSortedByName) {
    auto packages = registry->getAllPackages();
    EXPECT_FALSE(packages.empty());
    
    for (size_t i = 1; i < packages.size(); ++i) {
        EXPECT_LT(packages[i-1].name, packages[i].name) 
            << "Package " << packages[i-1].name << " should come before " << packages[i].name;
    }
}

TEST_F(PackageRegistryTest, RegistryContainsExpectedPackages) {
    auto packages = registry->listPackages();
    
    // Test for some specific packages that should be in the registry
    std::vector<std::string> expectedPackages = {
        "names", "cppcheck", "vcpkg-tool"
    };
    
    for (const auto& expected : expectedPackages) {
        EXPECT_THAT(packages, ::testing::Contains(expected))
            << "Expected package '" << expected << "' not found in registry";
        
        // Also verify the package has valid URL
        std::string url = registry->lookupPackage(expected);
        EXPECT_FALSE(url.empty()) << "Package '" << expected << "' has no URL";
    }
}

TEST_F(PackageRegistryTest, PackageUrlsAreValid) {
    auto packages = registry->getAllPackages();
    
    for (const auto& pkg : packages) {
        EXPECT_FALSE(pkg.url.empty()) << "Package '" << pkg.name << "' has empty URL";
        
        // URL should contain a protocol
        EXPECT_TRUE(pkg.url.find("://") != std::string::npos || 
                   pkg.url.find("git@") == 0)
            << "Package '" << pkg.name << "' URL '" << pkg.url << "' doesn't look like a valid Git URL";
        
        EXPECT_FALSE(pkg.description.empty()) 
            << "Package '" << pkg.name << "' has empty description";
    }
}

TEST_F(PackageRegistryTest, PackageNamesAreValid) {
    auto packages = registry->getAllPackages();
    
    for (const auto& pkg : packages) {
        EXPECT_FALSE(pkg.name.empty()) << "Found package with empty name";
        
        // Package names should not contain spaces or special characters
        EXPECT_EQ(pkg.name.find(' '), std::string::npos) 
            << "Package name '" << pkg.name << "' contains spaces";
        
        // Should only contain lowercase letters, numbers, and hyphens
        for (char c : pkg.name) {
            EXPECT_TRUE(std::islower(c) || std::isdigit(c) || c == '-' || c == '_')
                << "Package name '" << pkg.name << "' contains invalid character '" << c << "'";
        }
    }
}

TEST_F(PackageRegistryTest, SpecificPackageDetails) {
    // Test specific details of the names package
    const auto* namesInfo = registry->getPackageInfo("names");
    ASSERT_NE(namesInfo, nullptr);
    EXPECT_EQ(namesInfo->name, "names");
    EXPECT_EQ(namesInfo->url, "https://github.com/cplusplus-lang/names.git");
    EXPECT_THAT(namesInfo->description, ::testing::HasSubstr("random names"));
    EXPECT_EQ(namesInfo->binaries.size(), 1);
    EXPECT_EQ(namesInfo->binaries[0], "names");
    
    // Test a static analysis tool  
    const auto* cppcheckInfo = registry->getPackageInfo("cppcheck");
    ASSERT_NE(cppcheckInfo, nullptr);
    EXPECT_EQ(cppcheckInfo->name, "cppcheck");
    EXPECT_EQ(cppcheckInfo->url, "https://github.com/danmar/cppcheck.git");
    EXPECT_EQ(cppcheckInfo->binaries.size(), 1);
    EXPECT_EQ(cppcheckInfo->binaries[0], "cppcheck");
}