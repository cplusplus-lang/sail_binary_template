#include <catch2/catch_test_macros.hpp>
#include "init_command.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class InitCommandTestFixture {
public:
    InitCommandTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_init_test";
        std::filesystem::create_directories(testDir);
        originalCwd = std::filesystem::current_path();
        std::filesystem::current_path(testDir);
    }

    ~InitCommandTestFixture() {
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

    void createSourceFile(const std::string& path, const std::string& content) {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        std::ofstream file(path);
        file << content;
    }
};

TEST_CASE("InitCommand creates binary project with correct structure", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check directory structure
    REQUIRE(std::filesystem::exists("src"));
    REQUIRE(std::filesystem::exists("src/main.cpp"));
    REQUIRE(std::filesystem::exists("Sail.toml"));
    REQUIRE(std::filesystem::exists(".gitignore"));
    
    // Check main.cpp content
    REQUIRE(fixture.fileContains("src/main.cpp", "Hello, world!"));
    REQUIRE(fixture.fileContains("src/main.cpp", "int main()"));
}

TEST_CASE("InitCommand creates library project with correct structure", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--lib"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check directory structure
    REQUIRE(std::filesystem::exists("src"));
    REQUIRE(std::filesystem::exists("include"));
    REQUIRE(std::filesystem::exists("src/lib.cpp"));
    REQUIRE(std::filesystem::exists("Sail.toml"));
    REQUIRE(std::filesystem::exists(".gitignore"));
    
    // Check that header file is created with directory name
    std::string dirName = std::filesystem::current_path().filename().string();
    std::string headerFile = "include/" + dirName + ".h";
    REQUIRE(std::filesystem::exists(headerFile));
    
    // Check file contents
    REQUIRE(fixture.fileContains(headerFile, "namespace " + dirName));
    REQUIRE(fixture.fileContains("src/lib.cpp", "namespace " + dirName));
}

TEST_CASE("InitCommand uses custom name when specified", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--name", "custom_project"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check Sail.toml content
    REQUIRE(fixture.fileContains("Sail.toml", "name = \"custom_project\""));
    REQUIRE(fixture.fileContains("Sail.toml", "[package]"));
    REQUIRE(fixture.fileContains("Sail.toml", "version = \"1.0.0\""));
    REQUIRE(fixture.fileContains("Sail.toml", "standard = \"17\""));
    REQUIRE(fixture.fileContains("Sail.toml", "[dependencies]"));
    // Should NOT contain description
    REQUIRE_FALSE(fixture.fileContains("Sail.toml", "description"));
}

TEST_CASE("InitCommand fails when Sail.toml already exists", "[init_command]") {
    InitCommandTestFixture fixture;
    
    // Create existing Sail.toml
    std::ofstream sailToml("Sail.toml");
    sailToml << "[package]\nname = \"existing\"";
    sailToml.close();
    
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("InitCommand preserves existing source files", "[init_command]") {
    InitCommandTestFixture fixture;
    
    // Create existing source files
    fixture.createSourceFile("src/existing.cpp", "#include <iostream>\nint main() { return 42; }");
    fixture.createSourceFile("include/existing.h", "#pragma once\nvoid existing_function();");
    
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that existing files are preserved
    REQUIRE(std::filesystem::exists("src/existing.cpp"));
    REQUIRE(std::filesystem::exists("include/existing.h"));
    REQUIRE(fixture.fileContains("src/existing.cpp", "return 42;"));
    REQUIRE(fixture.fileContains("include/existing.h", "existing_function"));
    
    // Check that new main.cpp is NOT created when existing sources exist
    REQUIRE(!std::filesystem::exists("src/main.cpp"));
    
    // But Sail.toml should still be created
    REQUIRE(std::filesystem::exists("Sail.toml"));
}

TEST_CASE("InitCommand creates new source files when none exist", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that new source files are created
    REQUIRE(std::filesystem::exists("src/main.cpp"));
    REQUIRE(fixture.fileContains("src/main.cpp", "Hello, world!"));
}

TEST_CASE("InitCommand preserves existing .gitignore", "[init_command]") {
    InitCommandTestFixture fixture;
    
    // Create existing .gitignore
    std::ofstream gitignore(".gitignore");
    gitignore << "*.o\n*.exe\ncustom_ignore/";
    gitignore.close();
    
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that existing .gitignore is preserved
    REQUIRE(fixture.fileContains(".gitignore", "*.o"));
    REQUIRE(fixture.fileContains(".gitignore", "custom_ignore/"));
    
    // Should NOT contain the default Sail content
    REQUIRE(!fixture.fileContains(".gitignore", "build/"));
}

TEST_CASE("InitCommand creates .gitignore when none exists", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that .gitignore is created with default content
    REQUIRE(std::filesystem::exists(".gitignore"));
    REQUIRE(fixture.fileContains(".gitignore", "build/"));
    REQUIRE(fixture.fileContains(".gitignore", "CMakeLists.txt"));
}

TEST_CASE("InitCommand sanitizes project names", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--name", "my-project@123"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that name is sanitized in Sail.toml
    REQUIRE(fixture.fileContains("Sail.toml", "name = \"my-project_123\""));
}

TEST_CASE("InitCommand handles help option", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--help"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    // Should not create any files when showing help
    REQUIRE(!std::filesystem::exists("Sail.toml"));
}

TEST_CASE("InitCommand rejects unknown options", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--unknown"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("InitCommand rejects --name without value", "[init_command]") {
    InitCommandTestFixture fixture;
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--name"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("InitCommand library with existing sources", "[init_command]") {
    InitCommandTestFixture fixture;
    
    // Create existing source files
    fixture.createSourceFile("src/mylib.cpp", "void existing_function() {}");
    
    sail::InitCommand cmd;
    std::vector<std::string> args = {"--lib"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Check that existing files are preserved
    REQUIRE(std::filesystem::exists("src/mylib.cpp"));
    REQUIRE(fixture.fileContains("src/mylib.cpp", "existing_function"));
    
    // Should not create lib.cpp when existing sources exist
    REQUIRE(!std::filesystem::exists("src/lib.cpp"));
    
    // But include directory and Sail.toml should be created
    REQUIRE(std::filesystem::exists("include"));
    REQUIRE(std::filesystem::exists("Sail.toml"));
}