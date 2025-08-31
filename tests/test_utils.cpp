#include <catch2/catch_test_macros.hpp>
#include "utils.h"
#include <filesystem>
#include <fstream>

class UtilsTestFixture {
public:
    UtilsTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_test";
        std::filesystem::create_directories(testDir);
    }
    
    ~UtilsTestFixture() {
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
};

TEST_CASE("Utils::getHomeDirectory returns non-empty string", "[utils]") {
    std::string homeDir = sail::Utils::getHomeDirectory();
    REQUIRE_FALSE(homeDir.empty());
    REQUIRE(std::filesystem::exists(homeDir));
}

TEST_CASE("Utils::getSailDirectory returns correct path", "[utils]") {
    std::string sailDir = sail::Utils::getSailDirectory();
    REQUIRE_FALSE(sailDir.empty());
    
    std::string homeDir = sail::Utils::getHomeDirectory();
    REQUIRE(sailDir.find(homeDir) == 0); // sailDir should start with homeDir
    
#ifdef SAIL_PLATFORM_WINDOWS
    REQUIRE(sailDir.find("\\.sail") != std::string::npos);
#else
    REQUIRE(sailDir.find("/.sail") != std::string::npos);
#endif
}

TEST_CASE("Utils::getSailBinDirectory returns correct path", "[utils]") {
    std::string binDir = sail::Utils::getSailBinDirectory();
    REQUIRE_FALSE(binDir.empty());
    
    std::string sailDir = sail::Utils::getSailDirectory();
    REQUIRE(binDir.find(sailDir) == 0); // binDir should start with sailDir
    
#ifdef SAIL_PLATFORM_WINDOWS
    REQUIRE(binDir.find("\\bin") != std::string::npos);
#else
    REQUIRE(binDir.find("/bin") != std::string::npos);
#endif
}

TEST_CASE("Utils::createDirectoryRecursive creates directory", "[utils]") {
    UtilsTestFixture fixture;
    auto subDir = fixture.testDir / "level1" / "level2" / "level3";
    std::string subDirStr = subDir.string();
    
    REQUIRE_FALSE(std::filesystem::exists(subDir));
    REQUIRE(sail::Utils::createDirectoryRecursive(subDirStr));
    REQUIRE(std::filesystem::exists(subDir));
    REQUIRE(std::filesystem::is_directory(subDir));
}

TEST_CASE("Utils::createDirectoryRecursive handles existing directory", "[utils]") {
    UtilsTestFixture fixture;
    std::string testDirStr = fixture.testDir.string();
    REQUIRE(sail::Utils::createDirectoryRecursive(testDirStr)); // Should not fail on existing dir
}

TEST_CASE("Utils::fileExists returns true for existing file", "[utils]") {
    UtilsTestFixture fixture;
    auto testFile = fixture.testDir / "test_file.txt";
    std::ofstream file(testFile);
    file << "test content";
    file.close();
    
    REQUIRE(sail::Utils::fileExists(testFile.string()));
}

TEST_CASE("Utils::fileExists returns false for non-existing file", "[utils]") {
    UtilsTestFixture fixture;
    auto testFile = fixture.testDir / "non_existing_file.txt";
    REQUIRE_FALSE(sail::Utils::fileExists(testFile.string()));
}

TEST_CASE("Utils::fileExists returns false for directory", "[utils]") {
    UtilsTestFixture fixture;
    REQUIRE_FALSE(sail::Utils::fileExists(fixture.testDir.string()));
}

TEST_CASE("Utils::directoryExists returns true for existing directory", "[utils]") {
    UtilsTestFixture fixture;
    REQUIRE(sail::Utils::directoryExists(fixture.testDir.string()));
}

TEST_CASE("Utils::directoryExists returns false for non-existing directory", "[utils]") {
    UtilsTestFixture fixture;
    auto nonExisting = fixture.testDir / "non_existing";
    REQUIRE_FALSE(sail::Utils::directoryExists(nonExisting.string()));
}

TEST_CASE("Utils::directoryExists returns false for file", "[utils]") {
    UtilsTestFixture fixture;
    auto testFile = fixture.testDir / "test_file.txt";
    std::ofstream file(testFile);
    file << "test content";
    file.close();
    
    REQUIRE_FALSE(sail::Utils::directoryExists(testFile.string()));
}

TEST_CASE("Utils::getTemporaryDirectory returns valid path", "[utils]") {
    std::string tempDir = sail::Utils::getTemporaryDirectory();
    REQUIRE_FALSE(tempDir.empty());
    REQUIRE(std::filesystem::exists(tempDir));
    REQUIRE(std::filesystem::is_directory(tempDir));
}

TEST_CASE("Utils::copyFile successfully copies to destination", "[utils]") {
    UtilsTestFixture fixture;
    auto sourceFile = fixture.testDir / "source.txt";
    auto destFile = fixture.testDir / "destination.txt";
    
    // Create source file
    std::ofstream source(sourceFile);
    source << "test content for copying";
    source.close();
    
    REQUIRE(sail::Utils::copyFile(sourceFile.string(), destFile.string()));
    REQUIRE(std::filesystem::exists(destFile));
    
    // Verify content
    std::ifstream dest(destFile);
    std::string content((std::istreambuf_iterator<char>(dest)),
                        std::istreambuf_iterator<char>());
    REQUIRE(content == "test content for copying");
}

TEST_CASE("Utils::copyFile overwrites existing file", "[utils]") {
    UtilsTestFixture fixture;
    auto sourceFile = fixture.testDir / "source.txt";
    auto destFile = fixture.testDir / "destination.txt";
    
    // Create source file
    std::ofstream source(sourceFile);
    source << "new content";
    source.close();
    
    // Create existing destination file
    std::ofstream existing(destFile);
    existing << "old content";
    existing.close();
    
    REQUIRE(sail::Utils::copyFile(sourceFile.string(), destFile.string()));
    
    // Verify content was overwritten
    std::ifstream dest(destFile);
    std::string content((std::istreambuf_iterator<char>(dest)),
                        std::istreambuf_iterator<char>());
    REQUIRE(content == "new content");
}

TEST_CASE("Utils::isExecutable detects executable files", "[utils]") {
    UtilsTestFixture fixture;
#ifdef SAIL_PLATFORM_WINDOWS
    auto exeFile = fixture.testDir / "test.exe";
    std::ofstream exe(exeFile);
    exe << "fake executable";
    exe.close();
    
    REQUIRE(sail::Utils::isExecutable(exeFile.string()));
    
    auto nonExeFile = fixture.testDir / "test.txt";
    std::ofstream txt(nonExeFile);
    txt << "text file";
    txt.close();
    
    REQUIRE_FALSE(sail::Utils::isExecutable(nonExeFile.string()));
#else
    auto testFile = fixture.testDir / "test_executable";
    std::ofstream file(testFile);
    file << "#!/bin/bash\necho test";
    file.close();
    
    // Make file executable
    std::filesystem::permissions(testFile, std::filesystem::perms::owner_exec, 
                                std::filesystem::perm_options::add);
    
    REQUIRE(sail::Utils::isExecutable(testFile.string()));
    
    // Test non-executable file
    auto nonExeFile = fixture.testDir / "non_executable";
    std::ofstream nonExe(nonExeFile);
    nonExe << "not executable";
    nonExe.close();
    
    REQUIRE_FALSE(sail::Utils::isExecutable(nonExeFile.string()));
#endif
}

TEST_CASE("Utils::makeExecutable works on Unix-like systems", "[utils]") {
    UtilsTestFixture fixture;
#ifndef SAIL_PLATFORM_WINDOWS
    auto testFile = fixture.testDir / "make_executable_test";
    std::ofstream file(testFile);
    file << "#!/bin/bash\necho test";
    file.close();
    
    // Initially should not be executable (depending on umask)
    sail::Utils::makeExecutable(testFile.string());
    
    // After making executable, should be detectable
    REQUIRE(sail::Utils::isExecutable(testFile.string()));
#else
    // On Windows, this test doesn't apply as makeExecutable is a no-op
    REQUIRE(true); // Just pass the test
#endif
}

TEST_CASE("Utils::removeDirectory deletes directory and contents", "[utils]") {
    UtilsTestFixture fixture;
    auto subDir = fixture.testDir / "to_remove";
    auto subFile = subDir / "file.txt";
    
    std::filesystem::create_directory(subDir);
    std::ofstream file(subFile);
    file << "content";
    file.close();
    
    REQUIRE(std::filesystem::exists(subDir));
    REQUIRE(std::filesystem::exists(subFile));
    
    sail::Utils::removeDirectory(subDir.string());
    
    REQUIRE_FALSE(std::filesystem::exists(subDir));
    REQUIRE_FALSE(std::filesystem::exists(subFile));
}