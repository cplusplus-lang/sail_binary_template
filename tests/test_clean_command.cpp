#include <catch2/catch_test_macros.hpp>
#include "clean_command.h"
#include "utils.h"
#include <filesystem>
#include <fstream>

class CleanCommandTestFixture {
public:
    CleanCommandTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_clean_test";
        std::filesystem::create_directories(testDir);
        originalCwd = std::filesystem::current_path();
        std::filesystem::current_path(testDir);
        
        // Create a basic Sail project structure
        createSailProject();
    }

    ~CleanCommandTestFixture() {
        std::filesystem::current_path(originalCwd);
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
    std::filesystem::path originalCwd;
    
    void createSailProject() {
        // Create Sail.toml
        std::ofstream sailToml("Sail.toml");
        sailToml << "[package]\n";
        sailToml << "name = \"test_project\"\n";
        sailToml << "version = \"1.0.0\"\n";
        sailToml << "standard = \"17\"\n\n";
        sailToml << "[dependencies]\n";
        sailToml.close();
        
        // Create src directory and main.cpp
        std::filesystem::create_directories("src");
        std::ofstream mainCpp("src/main.cpp");
        mainCpp << "#include <iostream>\nint main() { return 0; }\n";
        mainCpp.close();
    }
    
    void createBuildDirectories() {
        std::filesystem::create_directories("build/debug");
        std::filesystem::create_directories("build/release");
        std::filesystem::create_directories("build/doc");
        
        // Create some dummy build files
        std::ofstream debugFile("build/debug/test_project");
        debugFile << "debug binary";
        debugFile.close();
        
        std::ofstream releaseFile("build/release/test_project");
        releaseFile << "release binary";
        releaseFile.close();
        
        std::ofstream docFile("build/doc/index.html");
        docFile << "<html>docs</html>";
        docFile.close();
        
        // Create generated CMakeLists.txt
        std::ofstream cmakeFile("CMakeLists.txt");
        cmakeFile << "# Generated CMakeLists.txt\n";
        cmakeFile.close();
    }
    
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

TEST_CASE("CleanCommand removes entire build directory by default", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build"));
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    REQUIRE(std::filesystem::exists("build/release/test_project"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build"));
    REQUIRE_FALSE(std::filesystem::exists("CMakeLists.txt"));
}

TEST_CASE("CleanCommand removes only debug build with --debug", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    REQUIRE(std::filesystem::exists("build/release/test_project"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--debug"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build/debug"));
    REQUIRE(std::filesystem::exists("build/release/test_project"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt")); // Should not remove when targeting specific builds
}

TEST_CASE("CleanCommand removes only release build with --release", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    REQUIRE(std::filesystem::exists("build/release/test_project"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--release"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    REQUIRE_FALSE(std::filesystem::exists("build/release"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt")); // Should not remove when targeting specific builds
}

TEST_CASE("CleanCommand removes both debug and release with --debug --release", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    REQUIRE(std::filesystem::exists("build/release/test_project"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--debug", "--release"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build/debug"));
    REQUIRE_FALSE(std::filesystem::exists("build/release"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt")); // Should not remove when targeting specific builds
}

TEST_CASE("CleanCommand removes only documentation with --doc", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build/doc/index.html"));
    REQUIRE(std::filesystem::exists("build/debug/test_project"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--doc"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build/doc"));
    REQUIRE(std::filesystem::exists("build/debug/test_project")); // Other build artifacts should remain
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
}

TEST_CASE("CleanCommand dry run shows what would be removed", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    REQUIRE(std::filesystem::exists("build"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--dry-run"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    // Files should still exist after dry run
    REQUIRE(std::filesystem::exists("build"));
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
}

TEST_CASE("CleanCommand handles non-existent build directory gracefully", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    // Don't create build directories
    REQUIRE_FALSE(std::filesystem::exists("build"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0); // Should succeed even if nothing to clean
}

TEST_CASE("CleanCommand handles help option", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--help"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
}

TEST_CASE("CleanCommand rejects unknown options", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--unknown"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("CleanCommand rejects --quiet and --verbose together", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--quiet", "--verbose"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result != 0);
}

TEST_CASE("CleanCommand works with verbose output", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--verbose"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build"));
}

TEST_CASE("CleanCommand works with quiet output", "[clean_command]") {
    CleanCommandTestFixture fixture;
    fixture.createBuildDirectories();
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--quiet"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(std::filesystem::exists("build"));
}

TEST_CASE("CleanCommand works outside project directory", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    // Create a subdirectory and change to it
    std::filesystem::create_directories("subdir");
    std::filesystem::current_path("subdir");
    
    // Create build directories in project root
    std::filesystem::create_directories("../build/debug");
    std::ofstream debugFile("../build/debug/test_project");
    debugFile << "debug binary";
    debugFile.close();
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0);
    // Should have found project root and cleaned it
    REQUIRE_FALSE(std::filesystem::exists("../build"));
}

TEST_CASE("CleanCommand handles doc cleaning when doc directory doesn't exist", "[clean_command]") {
    CleanCommandTestFixture fixture;
    
    // Don't create doc directory
    REQUIRE_FALSE(std::filesystem::exists("build/doc"));
    
    sail::CleanCommand cmd;
    std::vector<std::string> args = {"--doc"};
    
    int result = cmd.execute(args);
    
    REQUIRE(result == 0); // Should succeed even if doc directory doesn't exist
}