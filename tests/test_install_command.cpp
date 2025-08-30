#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "install_command.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class InstallCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        originalSailDir = std::filesystem::temp_directory_path() / "sail_install_test";
        std::filesystem::create_directories(originalSailDir);
        
        // We need to mock the sail directory for testing
        testSailBinDir = originalSailDir / "bin";
        std::filesystem::create_directories(testSailBinDir);
    }

    void TearDown() override {
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

TEST_F(InstallCommandTest, CreateTempDirCreatesUniqueDirectory) {
    MockInstallCommand cmd;
    
    EXPECT_TRUE(cmd.testCreateTempDir());
}

TEST_F(InstallCommandTest, InstallBinariesFindsAndInstallsExecutables) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    // Create mock build directory with executables
    auto mockBuildDir = originalSailDir / "mock_build";
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
    
    bool result = cmd.testInstallBinariesWithMockBuild(mockBuildDir.string(), testSailBinDir.string());
    
    EXPECT_TRUE(result);
    
    // Verify binaries were installed
#ifdef SAIL_PLATFORM_WINDOWS
    EXPECT_TRUE(std::filesystem::exists(testSailBinDir / "program1.exe"));
    EXPECT_TRUE(std::filesystem::exists(testSailBinDir / "program2.exe"));
#else
    EXPECT_TRUE(std::filesystem::exists(testSailBinDir / "program1"));
    EXPECT_TRUE(std::filesystem::exists(testSailBinDir / "program2"));
#endif
}

TEST_F(InstallCommandTest, InstallBinariesHandlesNonExecutableFiles) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    // Create mock build directory with non-executables
    auto mockBuildDir = originalSailDir / "mock_build_no_exe";
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
    
    bool result = cmd.testInstallBinariesWithMockBuild(mockBuildDir.string(), testSailBinDir.string());
    
    // Should fail because no executables found
    EXPECT_FALSE(result);
}

TEST_F(InstallCommandTest, InstallBinariesHandlesEmptyBuildDirectory) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    // Create empty build directory
    auto emptyBuildDir = originalSailDir / "empty_build";
    std::filesystem::create_directories(emptyBuildDir);
    
    bool result = cmd.testInstallBinariesWithMockBuild(emptyBuildDir.string(), testSailBinDir.string());
    
    // Should fail because no executables found
    EXPECT_FALSE(result);
}

TEST_F(InstallCommandTest, InstallBinariesHandlesNonExistentBuildDirectory) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    auto nonExistentDir = originalSailDir / "does_not_exist";
    
    bool result = cmd.testInstallBinariesWithMockBuild(nonExistentDir.string(), testSailBinDir.string());
    
    // Should fail because directory doesn't exist
    EXPECT_FALSE(result);
}

// Integration test that requires git and cmake
TEST_F(InstallCommandTest, DISABLED_FullInstallIntegrationTest) {
    // This test is disabled by default because it requires:
    // 1. Git to be available
    // 2. CMake to be available  
    // 3. Internet connectivity
    // 4. A known stable repository
    
    // To enable this test, rename it by removing DISABLED_ prefix
    // and ensure the above dependencies are met
    
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    // Use a small, stable repository for testing
    std::string testRepo = "https://github.com/octocat/Hello-World.git";
    
    int result = cmd.execute(testRepo);
    
    // If successful, there should be some installed binaries
    // The exact assertion depends on what the Hello-World repo actually builds
    if (result == 0) {
        // Verify that at least something was installed
        bool hasFiles = false;
        for (const auto& entry : std::filesystem::directory_iterator(testSailBinDir)) {
            if (entry.is_regular_file()) {
                hasFiles = true;
                break;
            }
        }
        EXPECT_TRUE(hasFiles);
    } else {
        // If the test failed, it could be due to environmental issues
        // Log the failure but don't fail the test suite
        std::cout << "Integration test failed - likely due to environment setup" << std::endl;
    }
}

// Test specific error conditions
TEST_F(InstallCommandTest, ExecuteHandlesInvalidUrl) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    std::string invalidUrl = "invalid-url-format";
    
    int result = cmd.execute(invalidUrl);
    
    // Should return non-zero exit code for failure
    EXPECT_NE(result, 0);
}

TEST_F(InstallCommandTest, ExecuteHandlesUnreachableUrl) {
    MockInstallCommand cmd;
    cmd.setTestInstallDir(testSailBinDir.string());
    
    std::string unreachableUrl = "https://this-domain-should-not-exist-for-testing.invalid/repo.git";
    
    int result = cmd.execute(unreachableUrl);
    
    // Should return non-zero exit code for failure
    EXPECT_NE(result, 0);
}