#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "utils.h"
#include <filesystem>
#include <fstream>

class UtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "sail_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
};

TEST_F(UtilsTest, GetHomeDirectoryReturnsNonEmptyString) {
    std::string homeDir = sail::Utils::getHomeDirectory();
    EXPECT_FALSE(homeDir.empty());
    EXPECT_TRUE(std::filesystem::exists(homeDir));
}

TEST_F(UtilsTest, GetSailDirectoryReturnsCorrectPath) {
    std::string sailDir = sail::Utils::getSailDirectory();
    EXPECT_FALSE(sailDir.empty());
    
    std::string homeDir = sail::Utils::getHomeDirectory();
    EXPECT_TRUE(sailDir.find(homeDir) == 0); // sailDir should start with homeDir
    
#ifdef SAIL_PLATFORM_WINDOWS
    EXPECT_TRUE(sailDir.find("\\.sail") != std::string::npos);
#else
    EXPECT_TRUE(sailDir.find("/.sail") != std::string::npos);
#endif
}

TEST_F(UtilsTest, GetSailBinDirectoryReturnsCorrectPath) {
    std::string binDir = sail::Utils::getSailBinDirectory();
    EXPECT_FALSE(binDir.empty());
    
    std::string sailDir = sail::Utils::getSailDirectory();
    EXPECT_TRUE(binDir.find(sailDir) == 0); // binDir should start with sailDir
    
#ifdef SAIL_PLATFORM_WINDOWS
    EXPECT_TRUE(binDir.find("\\bin") != std::string::npos);
#else
    EXPECT_TRUE(binDir.find("/bin") != std::string::npos);
#endif
}

TEST_F(UtilsTest, CreateDirectoryRecursiveCreatesDirectory) {
    auto subDir = testDir / "level1" / "level2" / "level3";
    std::string subDirStr = subDir.string();
    
    EXPECT_FALSE(std::filesystem::exists(subDir));
    EXPECT_TRUE(sail::Utils::createDirectoryRecursive(subDirStr));
    EXPECT_TRUE(std::filesystem::exists(subDir));
    EXPECT_TRUE(std::filesystem::is_directory(subDir));
}

TEST_F(UtilsTest, CreateDirectoryRecursiveHandlesExistingDirectory) {
    std::string testDirStr = testDir.string();
    EXPECT_TRUE(sail::Utils::createDirectoryRecursive(testDirStr)); // Should not fail on existing dir
}

TEST_F(UtilsTest, FileExistsReturnsTrueForExistingFile) {
    auto testFile = testDir / "test_file.txt";
    std::ofstream file(testFile);
    file << "test content";
    file.close();
    
    EXPECT_TRUE(sail::Utils::fileExists(testFile.string()));
}

TEST_F(UtilsTest, FileExistsReturnsFalseForNonExistingFile) {
    auto testFile = testDir / "non_existing_file.txt";
    EXPECT_FALSE(sail::Utils::fileExists(testFile.string()));
}

TEST_F(UtilsTest, FileExistsReturnsFalseForDirectory) {
    EXPECT_FALSE(sail::Utils::fileExists(testDir.string()));
}

TEST_F(UtilsTest, DirectoryExistsReturnsTrueForExistingDirectory) {
    EXPECT_TRUE(sail::Utils::directoryExists(testDir.string()));
}

TEST_F(UtilsTest, DirectoryExistsReturnsFalseForNonExistingDirectory) {
    auto nonExisting = testDir / "non_existing";
    EXPECT_FALSE(sail::Utils::directoryExists(nonExisting.string()));
}

TEST_F(UtilsTest, DirectoryExistsReturnsFalseForFile) {
    auto testFile = testDir / "test_file.txt";
    std::ofstream file(testFile);
    file << "test content";
    file.close();
    
    EXPECT_FALSE(sail::Utils::directoryExists(testFile.string()));
}

TEST_F(UtilsTest, GetTemporaryDirectoryReturnsValidPath) {
    std::string tempDir = sail::Utils::getTemporaryDirectory();
    EXPECT_FALSE(tempDir.empty());
    EXPECT_TRUE(std::filesystem::exists(tempDir));
    EXPECT_TRUE(std::filesystem::is_directory(tempDir));
}

TEST_F(UtilsTest, CopyFileSuccessfullyCopiesToDestination) {
    auto sourceFile = testDir / "source.txt";
    auto destFile = testDir / "destination.txt";
    
    // Create source file
    std::ofstream source(sourceFile);
    source << "test content for copying";
    source.close();
    
    EXPECT_TRUE(sail::Utils::copyFile(sourceFile.string(), destFile.string()));
    EXPECT_TRUE(std::filesystem::exists(destFile));
    
    // Verify content
    std::ifstream dest(destFile);
    std::string content((std::istreambuf_iterator<char>(dest)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "test content for copying");
}

TEST_F(UtilsTest, CopyFileOverwritesExistingFile) {
    auto sourceFile = testDir / "source.txt";
    auto destFile = testDir / "destination.txt";
    
    // Create source file
    std::ofstream source(sourceFile);
    source << "new content";
    source.close();
    
    // Create existing destination file
    std::ofstream existing(destFile);
    existing << "old content";
    existing.close();
    
    EXPECT_TRUE(sail::Utils::copyFile(sourceFile.string(), destFile.string()));
    
    // Verify content was overwritten
    std::ifstream dest(destFile);
    std::string content((std::istreambuf_iterator<char>(dest)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "new content");
}

TEST_F(UtilsTest, IsExecutableDetectsExecutableFiles) {
#ifdef SAIL_PLATFORM_WINDOWS
    auto exeFile = testDir / "test.exe";
    std::ofstream exe(exeFile);
    exe << "fake executable";
    exe.close();
    
    EXPECT_TRUE(sail::Utils::isExecutable(exeFile.string()));
    
    auto nonExeFile = testDir / "test.txt";
    std::ofstream txt(nonExeFile);
    txt << "text file";
    txt.close();
    
    EXPECT_FALSE(sail::Utils::isExecutable(nonExeFile.string()));
#else
    auto testFile = testDir / "test_executable";
    std::ofstream file(testFile);
    file << "#!/bin/bash\necho test";
    file.close();
    
    // Make file executable
    std::filesystem::permissions(testFile, std::filesystem::perms::owner_exec, 
                                std::filesystem::perm_options::add);
    
    EXPECT_TRUE(sail::Utils::isExecutable(testFile.string()));
    
    // Test non-executable file
    auto nonExeFile = testDir / "non_executable";
    std::ofstream nonExe(nonExeFile);
    nonExe << "not executable";
    nonExe.close();
    
    EXPECT_FALSE(sail::Utils::isExecutable(nonExeFile.string()));
#endif
}

TEST_F(UtilsTest, MakeExecutableWorksOnUnixLikeSystems) {
#ifndef SAIL_PLATFORM_WINDOWS
    auto testFile = testDir / "make_executable_test";
    std::ofstream file(testFile);
    file << "#!/bin/bash\necho test";
    file.close();
    
    // Initially should not be executable (depending on umask)
    sail::Utils::makeExecutable(testFile.string());
    
    // After making executable, should be detectable
    EXPECT_TRUE(sail::Utils::isExecutable(testFile.string()));
#else
    // On Windows, this test doesn't apply as makeExecutable is a no-op
    EXPECT_TRUE(true); // Just pass the test
#endif
}

TEST_F(UtilsTest, RemoveDirectoryDeletesDirectoryAndContents) {
    auto subDir = testDir / "to_remove";
    auto subFile = subDir / "file.txt";
    
    std::filesystem::create_directory(subDir);
    std::ofstream file(subFile);
    file << "content";
    file.close();
    
    EXPECT_TRUE(std::filesystem::exists(subDir));
    EXPECT_TRUE(std::filesystem::exists(subFile));
    
    sail::Utils::removeDirectory(subDir.string());
    
    EXPECT_FALSE(std::filesystem::exists(subDir));
    EXPECT_FALSE(std::filesystem::exists(subFile));
}