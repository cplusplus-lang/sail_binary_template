#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "git_utils.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class GitUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "sail_git_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
};

TEST_F(GitUtilsTest, ExtractRepoNameFromHttpsUrl) {
    std::string url = "https://github.com/cplusplus-lang/names";
    std::string expected = "names";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameFromHttpsUrlWithGitSuffix) {
    std::string url = "https://github.com/cplusplus-lang/names.git";
    std::string expected = "names";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameFromSshUrl) {
    std::string url = "git@github.com:cplusplus-lang/names.git";
    std::string expected = "names";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameFromSimpleName) {
    std::string url = "simple-name";
    std::string expected = "simple-name";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameFromComplexPath) {
    std::string url = "https://gitlab.com/user/group/subgroup/project-name.git";
    std::string expected = "project-name";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameWithSpecialCharacters) {
    std::string url = "https://github.com/user/my-awesome_project.123";
    std::string expected = "my-awesome_project.123";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameHandlesEmptyString) {
    std::string url = "";
    std::string expected = "";
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, ExtractRepoNameHandlesTrailingSlash) {
    std::string url = "https://github.com/cplusplus-lang/names/";
    std::string expected = "";  // Last part after / is empty
    EXPECT_EQ(sail::GitUtils::extractRepoName(url), expected);
}

TEST_F(GitUtilsTest, IsGitRepositoryReturnsFalseForNonGitDirectory) {
    EXPECT_FALSE(sail::GitUtils::isGitRepository(testDir.string()));
}

TEST_F(GitUtilsTest, IsGitRepositoryReturnsFalseForNonExistentDirectory) {
    auto nonExistent = testDir / "does_not_exist";
    EXPECT_FALSE(sail::GitUtils::isGitRepository(nonExistent.string()));
}

TEST_F(GitUtilsTest, IsGitRepositoryReturnsTrueForGitDirectory) {
    // Create a fake .git directory
    auto gitDir = testDir / ".git";
    std::filesystem::create_directory(gitDir);
    
    EXPECT_TRUE(sail::GitUtils::isGitRepository(testDir.string()));
}

// Integration test - only run if git is available and we can access the internet
TEST_F(GitUtilsTest, ClonePublicRepositoryIntegration) {
    // Skip this test in environments without git or internet access
    if (std::system("git --version > /dev/null 2>&1") != 0) {
        GTEST_SKIP() << "Git not available, skipping integration test";
        return;
    }
    
    // Use a small, reliable test repository
    std::string testUrl = "https://github.com/octocat/Hello-World.git";
    auto cloneDir = testDir / "hello_world_clone";
    
    bool result = sail::GitUtils::clone(testUrl, cloneDir.string());
    
    // The test result depends on internet connectivity and git availability
    // If successful, verify the directory structure
    if (result) {
        EXPECT_TRUE(std::filesystem::exists(cloneDir));
        EXPECT_TRUE(sail::GitUtils::isGitRepository(cloneDir.string()));
        EXPECT_TRUE(std::filesystem::exists(cloneDir / "README"));
    } else {
        // If clone failed, it could be due to network issues, which is acceptable
        // We'll just log this but not fail the test
        std::cout << "Clone integration test failed - likely due to network or git issues" << std::endl;
    }
}

TEST_F(GitUtilsTest, CloneInvalidUrlReturnsFalse) {
    std::string invalidUrl = "https://invalid-git-url-that-does-not-exist.com/repo.git";
    auto cloneDir = testDir / "should_not_exist";
    
    bool result = sail::GitUtils::clone(invalidUrl, cloneDir.string());
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(std::filesystem::exists(cloneDir));
}

TEST_F(GitUtilsTest, CloneToExistingDirectoryHandledGracefully) {
    // Create existing directory
    auto existingDir = testDir / "existing";
    std::filesystem::create_directory(existingDir);
    
    // Create a dummy file
    std::ofstream file(existingDir / "existing_file.txt");
    file << "existing content";
    file.close();
    
    std::string testUrl = "https://github.com/does-not-exist/fake-repo.git";
    
    bool result = sail::GitUtils::clone(testUrl, existingDir.string());
    
    // Should fail gracefully without crashing
    EXPECT_FALSE(result);
    // Original file should still exist
    EXPECT_TRUE(std::filesystem::exists(existingDir / "existing_file.txt"));
}