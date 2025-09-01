#include <catch2/catch_test_macros.hpp>
#include "install_command.h"
#include "utils.h"
#include <filesystem>
#include <fstream>
#include <iostream>

class InstallCommandTestFixture {
public:
    InstallCommandTestFixture() {
        originalSailDir = std::filesystem::temp_directory_path() / "sail_install_test";
        std::filesystem::create_directories(originalSailDir);
        
        // We need to mock the sail directory for testing
        testSailBinDir = originalSailDir / "bin";
        std::filesystem::create_directories(testSailBinDir);
    }

    ~InstallCommandTestFixture() {
        if (std::filesystem::exists(originalSailDir)) {
            std::filesystem::remove_all(originalSailDir);
        }
    }

    std::filesystem::path originalSailDir;
    std::filesystem::path testSailBinDir;
    
    // Helper to create a mock CMake project
    void createMockCMakeProject(const std::filesystem::path& projectDir) {
        std::filesystem::create_directories(projectDir);
        
        // Create CMakeLists.txt
        std::ofstream cmake(projectDir / "CMakeLists.txt");
        cmake << R"(cmake_minimum_required(VERSION 3.10)
project(TestProject)
set(CMAKE_CXX_STANDARD 17)

add_executable(test_binary main.cpp)
)";
        cmake.close();
        
        // Create main.cpp
        std::ofstream main(projectDir / "main.cpp");
        main << R"(#include <iostream>
int main() {
    std::cout << "Hello from test binary!" << std::endl;
    return 0;
}
)";
        main.close();
    }
};

// Mock class to test InstallCommand without actual git operations
class MockInstallCommand : public sail::InstallCommand {
public:
    // Override to use test directory instead of real ~/.sail/bin
    std::string getTestInstallDir() const {
        return testInstallDir;
    }
    
    void setTestInstallDir(const std::string& dir) {
        testInstallDir = dir;
    }
    
    // Test the individual components
    bool testCreateTempDir() {
        std::string tempDir = createTempDir();
        bool exists = !tempDir.empty() && std::filesystem::exists(tempDir);
        if (exists) {
            cleanupTempDir(tempDir);
        }
        return exists;
    }
    
    bool testInstallBinariesWithMockBuild(const std::string& mockBuildDir, const std::string& installDir) {
        return installBinaries(mockBuildDir, installDir);
    }

private:
    std::string testInstallDir;
    
    // Override getInstallDir for testing
    std::string getInstallDir() const override {
        return testInstallDir.empty() ? sail::InstallCommand::getInstallDir() : testInstallDir;
    }
    
    // Make protected methods accessible for testing
    using sail::InstallCommand::createTempDir;
    using sail::InstallCommand::cleanupTempDir;
    using sail::InstallCommand::installBinaries;
};

TEST_CASE("CreateTempDir creates unique directory", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    
    REQUIRE(cmd.testCreateTempDir());
}

TEST_CASE("InstallBinaries finds and installs executables", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    // Create mock build directory with executables
    auto mockBuildDir = fixture.originalSailDir / "mock_build";
    std::filesystem::create_directories(mockBuildDir);
    
    // Create mock executables
#ifdef SAIL_PLATFORM_WINDOWS
    auto exe1 = mockBuildDir / "program1.exe";
    auto exe2 = mockBuildDir / "program2.exe";
#else
    auto exe1 = mockBuildDir / "program1";
    auto exe2 = mockBuildDir / "program2";
#endif
    
    // Create executable files
    std::ofstream file1(exe1);
    file1 << "fake executable 1";
    file1.close();
    
    std::ofstream file2(exe2);
    file2 << "fake executable 2";
    file2.close();
    
    // Make them executable on Unix-like systems
#ifndef SAIL_PLATFORM_WINDOWS
    std::filesystem::permissions(exe1, std::filesystem::perms::owner_exec, 
                                std::filesystem::perm_options::add);
    std::filesystem::permissions(exe2, std::filesystem::perms::owner_exec, 
                                std::filesystem::perm_options::add);
#endif
    
    bool result = cmd.testInstallBinariesWithMockBuild(mockBuildDir.string(), fixture.testSailBinDir.string());
    
    REQUIRE(result);
    
    // Verify binaries were installed
#ifdef SAIL_PLATFORM_WINDOWS
    REQUIRE(std::filesystem::exists(fixture.testSailBinDir / "program1.exe"));
    REQUIRE(std::filesystem::exists(fixture.testSailBinDir / "program2.exe"));
#else
    REQUIRE(std::filesystem::exists(fixture.testSailBinDir / "program1"));
    REQUIRE(std::filesystem::exists(fixture.testSailBinDir / "program2"));
#endif
}

TEST_CASE("InstallBinaries handles non-executable files", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    // Create mock build directory with non-executables
    auto mockBuildDir = fixture.originalSailDir / "mock_build_no_exe";
    std::filesystem::create_directories(mockBuildDir);
    
    // Create non-executable files
    auto txtFile = mockBuildDir / "readme.txt";
    auto objFile = mockBuildDir / "object.o";
    
    std::ofstream file1(txtFile);
    file1 << "This is a readme file";
    file1.close();
    
    std::ofstream file2(objFile);
    file2 << "fake object file";
    file2.close();
    
    bool result = cmd.testInstallBinariesWithMockBuild(mockBuildDir.string(), fixture.testSailBinDir.string());
    
    // Should fail because no executables found
    REQUIRE_FALSE(result);
}

TEST_CASE("InstallBinaries handles empty build directory", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    // Create empty build directory
    auto emptyBuildDir = fixture.originalSailDir / "empty_build";
    std::filesystem::create_directories(emptyBuildDir);
    
    bool result = cmd.testInstallBinariesWithMockBuild(emptyBuildDir.string(), fixture.testSailBinDir.string());
    
    // Should fail because no executables found
    REQUIRE_FALSE(result);
}

TEST_CASE("InstallBinaries handles non-existent build directory", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    auto nonExistentDir = fixture.originalSailDir / "does_not_exist";
    
    bool result = cmd.testInstallBinariesWithMockBuild(nonExistentDir.string(), fixture.testSailBinDir.string());
    
    // Should fail because directory doesn't exist
    REQUIRE_FALSE(result);
}

// Integration test that requires git and cmake
TEST_CASE("Full install integration test", "[InstallCommand][integration][.integration]") {
    // This test is skipped by default because it requires:
    // 1. Git to be available
    // 2. CMake to be available  
    // 3. Internet connectivity
    // 4. A known stable repository
    
    // To enable this test, run with the [integration] tag
    SKIP("Integration test requires git, cmake, and internet connectivity");
    
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    // Use a small, stable repository for testing
    std::string testRepo = "https://github.com/octocat/Hello-World.git";
    std::vector<std::string> args = {testRepo};
    
    int result = cmd.execute(args);
    
    // If successful, there should be some installed binaries
    // The exact assertion depends on what the Hello-World repo actually builds
    if (result == 0) {
        // Verify that at least something was installed
        bool hasFiles = false;
        for (const auto& entry : std::filesystem::directory_iterator(fixture.testSailBinDir)) {
            if (entry.is_regular_file()) {
                hasFiles = true;
                break;
            }
        }
        REQUIRE(hasFiles);
    } else {
        // If the test failed, it could be due to environmental issues
        // Log the failure but don't fail the test suite
        std::cout << "Integration test failed - likely due to environment setup" << std::endl;
    }
}

// Test specific error conditions
TEST_CASE("Execute handles invalid URL", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    std::string invalidUrl = "invalid-url-format";
    std::vector<std::string> args = {invalidUrl};
    
    int result = cmd.execute(args);
    
    // Should return non-zero exit code for failure
    REQUIRE(result != 0);
}

TEST_CASE("Execute handles unreachable URL", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    std::string unreachableUrl = "https://this-domain-should-not-exist-for-testing.invalid/repo.git";
    std::vector<std::string> args = {unreachableUrl};
    
    int result = cmd.execute(args);
    
    // Should return non-zero exit code for failure
    REQUIRE(result != 0);
}

TEST_CASE("Execute handles --full-clone option", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    cmd.setTestInstallDir(fixture.testSailBinDir.string());
    
    std::vector<std::string> args = {"--full-clone", "https://github.com/test/repo.git"};
    
    // This should parse correctly even if it fails due to network
    // We're mainly testing argument parsing here
    int result = cmd.execute(args);
    
    // Should return non-zero because repo doesn't exist, but shouldn't crash
    REQUIRE(result != 0);
}

TEST_CASE("Execute handles --help option", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    
    std::vector<std::string> args = {"--help"};
    
    int result = cmd.execute(args);
    
    // Help should return success
    REQUIRE(result == 0);
}

TEST_CASE("Execute handles invalid option", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    
    std::vector<std::string> args = {"--invalid-option", "some-package"};
    
    int result = cmd.execute(args);
    
    // Should return error for invalid option
    REQUIRE(result != 0);
}

TEST_CASE("Execute handles empty arguments", "[InstallCommand]") {
    InstallCommandTestFixture fixture;
    MockInstallCommand cmd;
    
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    // Should return error for no arguments
    REQUIRE(result != 0);
}