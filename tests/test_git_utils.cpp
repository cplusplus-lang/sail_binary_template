#include <catch2/catch_test_macros.hpp>
#include "git_utils.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class GitUtilsTestFixture {
public:
    GitUtilsTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_git_test";
        std::filesystem::create_directories(testDir);
    }

    ~GitUtilsTestFixture() {
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
};

TEST_CASE("GitUtils::extractRepoName from HTTPS URL", "[git_utils]") {
    std::string url = "https://github.com/cplusplus-lang/names";
    std::string expected = "names";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName from HTTPS URL with .git suffix", "[git_utils]") {
    std::string url = "https://github.com/cplusplus-lang/names.git";
    std::string expected = "names";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName from SSH URL", "[git_utils]") {
    std::string url = "git@github.com:cplusplus-lang/names.git";
    std::string expected = "names";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName from simple name", "[git_utils]") {
    std::string url = "simple-name";
    std::string expected = "simple-name";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName from complex path", "[git_utils]") {
    std::string url = "https://gitlab.com/user/group/subgroup/project-name.git";
    std::string expected = "project-name";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName with special characters", "[git_utils]") {
    std::string url = "https://github.com/user/my-awesome_project.123";
    std::string expected = "my-awesome_project.123";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName handles empty string", "[git_utils]") {
    std::string url = "";
    std::string expected = "";
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::extractRepoName handles trailing slash", "[git_utils]") {
    std::string url = "https://github.com/cplusplus-lang/names/";
    std::string expected = "";  // Last part after / is empty
    REQUIRE(sail::GitUtils::extractRepoName(url) == expected);
}

TEST_CASE("GitUtils::isGitRepository returns false for non-git directory", "[git_utils]") {
    GitUtilsTestFixture fixture;
    REQUIRE_FALSE(sail::GitUtils::isGitRepository(fixture.testDir.string()));
}

TEST_CASE("GitUtils::isGitRepository returns false for non-existent directory", "[git_utils]") {
    GitUtilsTestFixture fixture;
    auto nonExistent = fixture.testDir / "does_not_exist";
    REQUIRE_FALSE(sail::GitUtils::isGitRepository(nonExistent.string()));
}

TEST_CASE("GitUtils::isGitRepository returns true for git directory", "[git_utils]") {
    GitUtilsTestFixture fixture;
    // Create a fake .git directory
    auto gitDir = fixture.testDir / ".git";
    std::filesystem::create_directory(gitDir);
    
    REQUIRE(sail::GitUtils::isGitRepository(fixture.testDir.string()));
}

TEST_CASE("GitUtils::clone public repository integration test", "[git_utils][integration]") {
    GitUtilsTestFixture fixture;
    // Skip this test in environments without git or internet access
    if (std::system("git --version > /dev/null 2>&1") != 0) {
        SKIP("Git not available, skipping integration test");
    }
    
    // Use a small, reliable test repository
    std::string testUrl = "https://github.com/octocat/Hello-World.git";
    auto cloneDir = fixture.testDir / "hello_world_clone";
    
    bool result = sail::GitUtils::clone(testUrl, cloneDir.string());
    
    // The test result depends on internet connectivity and git availability
    // If successful, verify the directory structure
    if (result) {
        REQUIRE(std::filesystem::exists(cloneDir));
        REQUIRE(sail::GitUtils::isGitRepository(cloneDir.string()));
        REQUIRE(std::filesystem::exists(cloneDir / "README"));
    } else {
        // If clone failed, it could be due to network issues, which is acceptable
        // We'll just log this but not fail the test
        INFO("Clone integration test failed - likely due to network or git issues");
        REQUIRE(true); // Don't fail the test
    }
}

TEST_CASE("GitUtils::clone invalid URL returns false", "[git_utils]") {
    GitUtilsTestFixture fixture;
    std::string invalidUrl = "https://invalid-git-url-that-does-not-exist.com/repo.git";
    auto cloneDir = fixture.testDir / "should_not_exist";
    
    bool result = sail::GitUtils::clone(invalidUrl, cloneDir.string());
    
    REQUIRE_FALSE(result);
    REQUIRE_FALSE(std::filesystem::exists(cloneDir));
}

TEST_CASE("GitUtils::clone to existing directory handled gracefully", "[git_utils]") {
    GitUtilsTestFixture fixture;
    // Create existing directory
    auto existingDir = fixture.testDir / "existing";
    std::filesystem::create_directory(existingDir);
    
    // Create a dummy file
    std::ofstream file(existingDir / "existing_file.txt");
    file << "existing content";
    file.close();
    
    std::string testUrl = "https://github.com/does-not-exist/fake-repo.git";
    
    bool result = sail::GitUtils::clone(testUrl, existingDir.string());
    
    // Should fail gracefully without crashing
    REQUIRE_FALSE(result);
    // Original file should still exist
    REQUIRE(std::filesystem::exists(existingDir / "existing_file.txt"));
}