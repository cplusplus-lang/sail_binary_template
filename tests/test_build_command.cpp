#include <catch2/catch_test_macros.hpp>
#include "build_command.h"
#include <filesystem>
#include <fstream>

class BuildCommandTestFixture {
public:
    BuildCommandTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_build_test";
        std::filesystem::create_directories(testDir);
        
        // Save original working directory
        originalDir = std::filesystem::current_path();
        
        // Change to test directory
        std::filesystem::current_path(testDir);
        
        // Create a minimal CMakeLists.txt for testing
        std::ofstream cmakeFile("CMakeLists.txt");
        cmakeFile << "cmake_minimum_required(VERSION 3.20)\n";
        cmakeFile << "project(test_project)\n";
        cmakeFile << "add_executable(test_app main.cpp)\n";
        cmakeFile.close();
        
        // Create a minimal main.cpp
        std::ofstream mainFile("main.cpp");
        mainFile << "#include <iostream>\n";
        mainFile << "int main() { std::cout << \"Hello World\" << std::endl; return 0; }\n";
        mainFile.close();
    }

    ~BuildCommandTestFixture() {
        // Restore original directory
        std::filesystem::current_path(originalDir);
        
        // Clean up test directory
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::filesystem::path testDir;
    std::filesystem::path originalDir;
};

TEST_CASE("BuildCommand::execute creates debug build directory", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // Build may fail due to missing dependencies, but directory should be created
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("BuildCommand::execute creates release build directory with --release flag", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args = {"--release"};
    int result = buildCmd.execute(args);
    
    // Build may fail due to missing dependencies, but directory should be created
    REQUIRE(std::filesystem::exists("build/release"));
}

TEST_CASE("BuildCommand::execute handles help flag", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args = {"--help"};
    int result = buildCmd.execute(args);
    
    REQUIRE(result == 0);
}

TEST_CASE("BuildCommand::execute handles -h flag", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args = {"-h"};
    int result = buildCmd.execute(args);
    
    REQUIRE(result == 0);
}

TEST_CASE("BuildCommand::execute recognizes release flags", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Test --release flag
    std::vector<std::string> releaseArgs = {"--release"};
    buildCmd.execute(releaseArgs);
    REQUIRE(std::filesystem::exists("build/release"));
    
    // Clean up and test -r flag
    std::filesystem::remove_all("build");
    
    std::vector<std::string> shortArgs = {"-r"};
    buildCmd.execute(shortArgs);
    REQUIRE(std::filesystem::exists("build/release"));
}

TEST_CASE("BuildCommand::execute handles verbose flag", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args = {"--verbose"};
    int result = buildCmd.execute(args);
    
    // Should create debug directory (default) even with verbose flag
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("BuildCommand::execute copies CPM.cmake when it exists", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create target/cmake directory and CPM.cmake file
    std::filesystem::create_directories("target/cmake");
    std::ofstream cpmFile("target/cmake/CPM.cmake");
    cpmFile << "# CPM.cmake content\n";
    cpmFile.close();
    
    std::vector<std::string> args;
    buildCmd.execute(args);
    
    REQUIRE(std::filesystem::exists("build/debug/CPM.cmake"));
}

TEST_CASE("BuildCommand::execute handles missing CPM.cmake gracefully", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Don't create CPM.cmake file
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // Should still create build directory even without CPM.cmake
    REQUIRE(std::filesystem::exists("build/debug"));
    REQUIRE_FALSE(std::filesystem::exists("build/debug/CPM.cmake"));
}

TEST_CASE("BuildCommand::execute handles multiple flags", "[build_command]") {
    BuildCommandTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args = {"--release", "--verbose"};
    buildCmd.execute(args);
    
    REQUIRE(std::filesystem::exists("build/release"));
}