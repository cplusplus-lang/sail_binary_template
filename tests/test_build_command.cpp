#include <catch2/catch_test_macros.hpp>
#include "build_command.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>

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
        cmakeFile << "cmake_minimum_required(VERSION 3.21)\n";
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

class SailProjectTestFixture {
public:
    SailProjectTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_cmake_test";
        std::filesystem::create_directories(testDir);
        
        // Save original working directory
        originalDir = std::filesystem::current_path();
        
        // Change to test directory
        std::filesystem::current_path(testDir);
        
        // Create a Sail.toml file
        std::ofstream tomlFile("Sail.toml");
        tomlFile << "[package]\n";
        tomlFile << "name = \"test_project\"\n";
        tomlFile << "version = \"1.0.0\"\n";
        tomlFile << "description = \"A test project\"\n\n";
        tomlFile << "[dependencies]\n";
        tomlFile.close();
        
        // Create src directory with main.cpp
        std::filesystem::create_directories("src");
        std::ofstream mainFile("src/main.cpp");
        mainFile << "#include <iostream>\n";
        mainFile << "int main() { std::cout << \"Hello World\" << std::endl; return 0; }\n";
        mainFile.close();
    }

    ~SailProjectTestFixture() {
        // Restore original directory
        std::filesystem::current_path(originalDir);
        
        // Clean up test directory
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }
    
    bool fileContains(const std::string& filepath, const std::string& content) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find(content) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    std::filesystem::path testDir;
    std::filesystem::path originalDir;
};

TEST_CASE("BuildCommand generates CMake structure for binary project", "[build_command][cmake_generation]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Initially no CMake files should exist
    REQUIRE(!std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(!std::filesystem::exists("build/cmake"));
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // CMake files should be created
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake"));
    REQUIRE(std::filesystem::exists("build/cmake/CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake/SailToml.cmake"));
    
    // Check root CMakeLists.txt content
    REQUIRE(fixture.fileContains("CMakeLists.txt", "include(build/cmake/SailToml.cmake)"));
    REQUIRE(fixture.fileContains("CMakeLists.txt", "sail_parse_toml()"));
    REQUIRE(fixture.fileContains("CMakeLists.txt", "project(${SAIL_PROJECT_NAME}"));
    
    // Check build CMakeLists.txt content for binary
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "add_executable(${SAIL_PROJECT_NAME}"));
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "file(GLOB_RECURSE PROJECT_SOURCES \"src/*.cpp\")"));
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "add_executable(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES}"));
}

TEST_CASE("BuildCommand generates CMake structure for library project", "[build_command][cmake_generation]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create include directory to make it a library project
    std::filesystem::create_directories("include");
    std::ofstream headerFile("include/test_project.h");
    headerFile << "#pragma once\nvoid hello();\n";
    headerFile.close();
    
    // Create library source file
    std::ofstream libFile("src/lib.cpp");
    libFile << "#include \"test_project.h\"\n#include <iostream>\nvoid hello() { std::cout << \"Hello from lib\" << std::endl; }\n";
    libFile.close();
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // CMake files should be created
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake/CMakeLists.txt"));
    
    // Check build CMakeLists.txt content for library
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "add_library(${SAIL_PROJECT_NAME}"));
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "file(GLOB_RECURSE PROJECT_SOURCES \"src/*.cpp\")"));
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "add_library(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES}"));
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "target_include_directories"));
}

TEST_CASE("BuildCommand TOML parsing module contains correct patterns", "[build_command][toml_parsing]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    std::vector<std::string> args;
    buildCmd.execute(args);
    
    // Check SailToml.cmake content
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "function(sail_parse_toml)"));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "file(READ"));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "SAIL_PROJECT_NAME"));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "SAIL_PROJECT_VERSION"));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "PARENT_SCOPE"));
    
    // Check that regex patterns look under [package] section
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "\\\\[package\\\\]"));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "name = \\\"([^\\\"]+)\\\""));
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "version = \\\"([^\\\"]+)\\\""));
}

TEST_CASE("BuildCommand skips CMake generation on second run", "[build_command][cmake_generation]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // First run - should generate CMake files
    std::vector<std::string> args;
    buildCmd.execute(args);
    
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake/SailToml.cmake"));
    
    // Second run - should not fail and should work normally
    int result = buildCmd.execute(args);
    
    // Files should still exist and build should succeed
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake/SailToml.cmake"));
}

TEST_CASE("BuildCommand handles missing Sail.toml", "[build_command][error_handling]") {
    BuildCommandTestFixture fixture; // This creates a regular CMakeLists.txt, not a Sail project
    sail::BuildCommand buildCmd;
    
    // Remove the Sail.toml that would be in a Sail project
    // (BuildCommandTestFixture doesn't create one)
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // Should still work with regular CMake project
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("BuildCommand TOML parsing handles complex project names", "[build_command][toml_parsing]") {
    SailProjectTestFixture fixture;
    
    // Override the Sail.toml with a more complex name
    std::ofstream tomlFile("Sail.toml");
    tomlFile << "[package]\n";
    tomlFile << "name = \"my-complex_project123\"\n";
    tomlFile << "version = \"2.1.0-alpha\"\n";
    tomlFile << "description = \"A project with @#$ special chars!\"\n\n";
    tomlFile << "[dependencies]\n";
    tomlFile << "some_lib = \"1.0\"\n";
    tomlFile.close();
    
    sail::BuildCommand buildCmd;
    std::vector<std::string> args;
    buildCmd.execute(args);
    
    // Should create CMake files successfully
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/cmake/SailToml.cmake"));
    
    // TOML parsing should handle the complex name
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "SAIL_PROJECT_NAME"));
}

TEST_CASE("BuildCommand::execute works from project subdirectory", "[build_command][subfolder]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create a subdirectory and change to it
    std::filesystem::create_directories("subdir/nested");
    std::filesystem::current_path("subdir/nested");
    
    // Build from subdirectory should work
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // Should have created build directory in project root
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/debug"));
    
    // Change back to project root for cleanup
    std::filesystem::current_path("../..");
}

TEST_CASE("BuildCommand::execute handles deep nested subfolder", "[build_command][subfolder]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create a deeply nested directory structure
    std::filesystem::create_directories("src/modules/utils/tests");
    std::filesystem::current_path("src/modules/utils/tests");
    
    // Build from deep subdirectory should work
    std::vector<std::string> args = {"--release"};
    int result = buildCmd.execute(args);
    
    // Should create release build in project root
    REQUIRE(std::filesystem::exists("CMakeLists.txt"));
    REQUIRE(std::filesystem::exists("build/release"));
    
    // Change back to project root for cleanup
    std::filesystem::current_path("../../../..");
}

TEST_CASE("BuildCommand validates C++ standard from Sail.toml", "[build_command][cpp_standard]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create Sail.toml with valid C++ standard
    std::ofstream tomlFile("Sail.toml");
    tomlFile << "[package]\n";
    tomlFile << "name = \"test_project\"\n";
    tomlFile << "version = \"1.0.0\"\n";
    tomlFile << "standard = \"20\"\n\n";
    tomlFile << "[dependencies]\n";
    tomlFile.close();
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    REQUIRE(result == 0);
    REQUIRE(std::filesystem::exists("build/cmake/CMakeLists.txt"));
    
    // Check that generated CMakeLists.txt uses the correct standard
    REQUIRE(fixture.fileContains("build/cmake/CMakeLists.txt", "set(CMAKE_CXX_STANDARD ${SAIL_CPP_STANDARD})"));
}

TEST_CASE("BuildCommand rejects invalid C++ standard from Sail.toml", "[build_command][cpp_standard]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create Sail.toml with invalid C++ standard
    std::ofstream tomlFile("Sail.toml");
    tomlFile << "[package]\n";
    tomlFile << "name = \"test_project\"\n";
    tomlFile << "version = \"1.0.0\"\n";
    tomlFile << "standard = \"99\"\n\n";
    tomlFile << "[dependencies]\n";
    tomlFile.close();
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    // Should fail with invalid standard
    REQUIRE(result != 0);
}

TEST_CASE("BuildCommand uses default C++ standard when none specified", "[build_command][cpp_standard]") {
    SailProjectTestFixture fixture;
    sail::BuildCommand buildCmd;
    
    // Create Sail.toml without standard field
    std::ofstream tomlFile("Sail.toml");
    tomlFile << "[package]\n";
    tomlFile << "name = \"test_project\"\n";
    tomlFile << "version = \"1.0.0\"\n\n";
    tomlFile << "[dependencies]\n";
    tomlFile.close();
    
    std::vector<std::string> args;
    int result = buildCmd.execute(args);
    
    REQUIRE(result == 0);
    
    // Should use default standard (17) - check for the warning path since no standard was specified
    REQUIRE(fixture.fileContains("build/cmake/SailToml.cmake", "set(SAIL_CPP_STANDARD \"17\" PARENT_SCOPE)"));
}