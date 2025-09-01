#include <catch2/catch_test_macros.hpp>
#include "new_command.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class NewCommandTestFixture {
public:
    NewCommandTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_new_test";
        std::filesystem::create_directories(testDir);
        originalCwd = std::filesystem::current_path();
        std::filesystem::current_path(testDir);
    }

    ~NewCommandTestFixture() {
        std::filesystem::current_path(originalCwd);
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
    std::filesystem::path originalCwd;
    
    bool fileContains(const std::filesystem::path& filePath, const std::string& content) {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find(content) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
};

TEST_CASE("NewCommand creates binary project with correct structure", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"test_project"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check directory structure - CMakeLists.txt is now created by 'sail build'
    REQUIRE(std::filesystem::exists("test_project"));
    REQUIRE(std::filesystem::exists("test_project/src"));
    REQUIRE(std::filesystem::exists("test_project/src/main.cpp"));
    REQUIRE(std::filesystem::exists("test_project/Sail.toml"));
    REQUIRE(std::filesystem::exists("test_project/.gitignore"));
    
    // CMakeLists.txt should NOT exist after 'sail new'
    REQUIRE(!std::filesystem::exists("test_project/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("test_project/build"));
}

TEST_CASE("NewCommand creates library project with correct structure", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"test_lib", "--lib"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check directory structure - CMakeLists.txt is now created by 'sail build'
    REQUIRE(std::filesystem::exists("test_lib"));
    REQUIRE(std::filesystem::exists("test_lib/src"));
    REQUIRE(std::filesystem::exists("test_lib/include"));
    REQUIRE(std::filesystem::exists("test_lib/src/lib.cpp"));
    REQUIRE(std::filesystem::exists("test_lib/include/test_lib.h"));
    REQUIRE(std::filesystem::exists("test_lib/Sail.toml"));
    REQUIRE(std::filesystem::exists("test_lib/.gitignore"));
    
    // CMakeLists.txt should NOT exist after 'sail new'
    REQUIRE(!std::filesystem::exists("test_lib/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("test_lib/build"));
}

TEST_CASE("NewCommand does not create CMake files", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"test_project"};
    
    int result = cmd.execute(args);
    REQUIRE(result == 0);
    
    // Verify that NO CMake files are created by 'sail new'
    REQUIRE(!std::filesystem::exists("test_project/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("test_project/build"));
    REQUIRE(!std::filesystem::exists("test_project/build/cmake"));
    REQUIRE(!std::filesystem::exists("test_project/build/cmake/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("test_project/build/cmake/SailToml.cmake"));
    
    // But essential project files should exist
    REQUIRE(std::filesystem::exists("test_project/Sail.toml"));
    REQUIRE(std::filesystem::exists("test_project/src/main.cpp"));
    REQUIRE(std::filesystem::exists("test_project/.gitignore"));
}

TEST_CASE("main.cpp contains hello world", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"test_project"};
    
    cmd.execute(args);
    
    // Check main.cpp content
    REQUIRE(fixture.fileContains("test_project/src/main.cpp", "Hello, world!"));
    REQUIRE(fixture.fileContains("test_project/src/main.cpp", "int main()"));
    REQUIRE(fixture.fileContains("test_project/src/main.cpp", "return 0;"));
}

TEST_CASE("Sail.toml contains correct content", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"test_project"};
    
    cmd.execute(args);
    
    // Check Sail.toml content
    REQUIRE(fixture.fileContains("test_project/Sail.toml", "[package]"));
    REQUIRE(fixture.fileContains("test_project/Sail.toml", "name = \"test_project\""));
    REQUIRE(fixture.fileContains("test_project/Sail.toml", "version = \"1.0.0\""));
    REQUIRE(fixture.fileContains("test_project/Sail.toml", "standard = \"17\""));
    REQUIRE(fixture.fileContains("test_project/Sail.toml", "[dependencies]"));
    // Should NOT contain description
    REQUIRE_FALSE(fixture.fileContains("test_project/Sail.toml", "description"));
}

TEST_CASE("NewCommand rejects existing directory", "[new_command]") {
    NewCommandTestFixture fixture;
    // Create existing directory
    std::filesystem::create_directories("existing_project");
    
    sail::NewCommand cmd;
    std::vector<std::string> args = {"existing_project"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("NewCommand handles empty args", "[new_command]") {
    sail::NewCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("NewCommand sanitizes project names", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"my-project@123"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Should sanitize the name and create directory
    REQUIRE(std::filesystem::exists("my-project_123"));
    REQUIRE(fixture.fileContains("my-project_123/Sail.toml", "name = \"my-project_123\""));
}

TEST_CASE("Created project has essential files only", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"essential_project"};
    
    int result = cmd.execute(args);
    REQUIRE(result == 0);
    
    // Check essential files exist
    REQUIRE(std::filesystem::exists("essential_project/Sail.toml"));
    REQUIRE(std::filesystem::exists("essential_project/src/main.cpp"));
    REQUIRE(std::filesystem::exists("essential_project/.gitignore"));
    
    // Check CMake files do NOT exist (created by build command)
    REQUIRE(!std::filesystem::exists("essential_project/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("essential_project/build"));
}

TEST_CASE("Created library project has essential files only", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"mylib", "--lib"};
    
    int result = cmd.execute(args);
    REQUIRE(result == 0);
    
    // Check essential library files exist
    REQUIRE(std::filesystem::exists("mylib/Sail.toml"));
    REQUIRE(std::filesystem::exists("mylib/include/mylib.h"));
    REQUIRE(std::filesystem::exists("mylib/src/lib.cpp"));
    REQUIRE(std::filesystem::exists("mylib/.gitignore"));
    
    // Check CMake files do NOT exist (created by build command)
    REQUIRE(!std::filesystem::exists("mylib/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("mylib/build"));
    
    // Check file contents
    REQUIRE(fixture.fileContains("mylib/include/mylib.h", "namespace mylib"));
    REQUIRE(fixture.fileContains("mylib/src/lib.cpp", "namespace mylib"));
}

TEST_CASE("NewCommand creates project in subdirectory", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"subdir/nested_project"};
    
    int result = cmd.execute(args);
    REQUIRE(result == 0);
    
    // Check directory structure
    REQUIRE(std::filesystem::exists("subdir/nested_project"));
    REQUIRE(std::filesystem::exists("subdir/nested_project/src/main.cpp"));
    REQUIRE(std::filesystem::exists("subdir/nested_project/Sail.toml"));
    REQUIRE(std::filesystem::exists("subdir/nested_project/.gitignore"));
    
    // CMake files should NOT exist
    REQUIRE(!std::filesystem::exists("subdir/nested_project/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("subdir/nested_project/build"));
    
    // Check project name in Sail.toml (should be extracted from path)
    REQUIRE(fixture.fileContains("subdir/nested_project/Sail.toml", "name = \"nested_project\""));
}

TEST_CASE("NewCommand handles paths with special characters", "[new_command]") {
    NewCommandTestFixture fixture;
    sail::NewCommand cmd;
    std::vector<std::string> args = {"projects/my-awesome@project"};
    
    int result = cmd.execute(args);
    REQUIRE(result == 0);
    
    // Should sanitize only the filename, not the directory path
    REQUIRE(std::filesystem::exists("projects/my-awesome_project"));
    REQUIRE(fixture.fileContains("projects/my-awesome_project/Sail.toml", "name = \"my-awesome_project\""));
    
    // CMake files should NOT exist
    REQUIRE(!std::filesystem::exists("projects/my-awesome_project/CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("projects/my-awesome_project/build"));
}
