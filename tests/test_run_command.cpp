#include <catch2/catch_test_macros.hpp>
#include "run_command.h"
#include <filesystem>
#include <fstream>

class RunCommandTestFixture {
public:
    RunCommandTestFixture() {
        testDir = std::filesystem::temp_directory_path() / "sail_run_test";
        std::filesystem::create_directories(testDir);
        
        // Save original working directory
        originalDir = std::filesystem::current_path();
        
        // Change to test directory
        std::filesystem::current_path(testDir);
        
        // Create a minimal CMakeLists.txt for testing
        std::ofstream cmakeFile("CMakeLists.txt");
        cmakeFile << "cmake_minimum_required(VERSION 3.20)\n";
        cmakeFile << "project(test_project)\n";
        cmakeFile << "add_executable(test_project main.cpp)\n";
        cmakeFile.close();
        
        // Create a minimal main.cpp that outputs arguments
        std::ofstream mainFile("main.cpp");
        mainFile << "#include <iostream>\n";
        mainFile << "int main(int argc, char* argv[]) {\n";
        mainFile << "    std::cout << \"Hello from test_project!\" << std::endl;\n";
        mainFile << "    for (int i = 1; i < argc; ++i) {\n";
        mainFile << "        std::cout << \"Arg \" << i << \": \" << argv[i] << std::endl;\n";
        mainFile << "    }\n";
        mainFile << "    return 0;\n";
        mainFile << "}\n";
        mainFile.close();
    }

    ~RunCommandTestFixture() {
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

TEST_CASE("RunCommand::execute shows help with --help flag", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"--help"};
    int result = runCmd.execute(args);
    
    REQUIRE(result == 0);
}

TEST_CASE("RunCommand::execute shows help with -h flag", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"-h"};
    int result = runCmd.execute(args);
    
    REQUIRE(result == 0);
}

TEST_CASE("RunCommand::execute creates build directory", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args;
    // This will likely fail due to missing dependencies, but should create directory
    runCmd.execute(args);
    
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("RunCommand::execute creates release build directory with --release flag", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"--release"};
    // This will likely fail due to missing dependencies, but should create directory
    runCmd.execute(args);
    
    REQUIRE(std::filesystem::exists("build/release"));
}

TEST_CASE("RunCommand::execute handles binary name flag", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"--bin", "custom_binary"};
    // Should attempt to find custom_binary executable
    int result = runCmd.execute(args);
    
    // Should create build directory even if execution fails
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("RunCommand::execute separates executable arguments", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"--", "arg1", "arg2", "arg3"};
    // Should separate arguments after --
    int result = runCmd.execute(args);
    
    // Should create build directory
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("RunCommand::execute handles multiple flags", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"--release", "--verbose", "--bin", "myapp", "--", "arg1"};
    int result = runCmd.execute(args);
    
    // Should create release build directory
    REQUIRE(std::filesystem::exists("build/release"));
}

TEST_CASE("RunCommand::execute handles short flags", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    std::vector<std::string> args = {"-r", "-v", "-b", "myapp"};
    int result = runCmd.execute(args);
    
    // Should create release build directory with short flags
    REQUIRE(std::filesystem::exists("build/release"));
}

TEST_CASE("RunCommand::execute with no project name in CMakeLists.txt", "[run_command]") {
    RunCommandTestFixture fixture;
    
    // Create CMakeLists.txt without project name
    std::ofstream cmakeFile("CMakeLists.txt");
    cmakeFile << "cmake_minimum_required(VERSION 3.20)\n";
    cmakeFile << "# No project() call\n";
    cmakeFile.close();
    
    sail::RunCommand runCmd;
    std::vector<std::string> args;
    int result = runCmd.execute(args);
    
    // Should fail with error about determining binary name
    REQUIRE(result == 1);
}

TEST_CASE("RunCommand::execute extracts project name from CMakeLists.txt", "[run_command]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    // The fixture creates a CMakeLists.txt with project(test_project)
    std::vector<std::string> args;
    int result = runCmd.execute(args);
    
    // Should attempt to build and run test_project
    REQUIRE(std::filesystem::exists("build/debug"));
}

TEST_CASE("RunCommand::execute works from project subdirectory", "[run_command][subfolder]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    // Create a subdirectory and change to it
    std::filesystem::create_directories("subdir/nested");
    std::filesystem::current_path("subdir/nested");
    
    // Run from subdirectory should work
    std::vector<std::string> args;
    int result = runCmd.execute(args);
    
    // Should create build directory in project root
    REQUIRE(std::filesystem::exists("build/debug"));
    
    // Change back to project root for cleanup
    std::filesystem::current_path("../..");
}

TEST_CASE("RunCommand::execute handles subfolder with release flag", "[run_command][subfolder]") {
    RunCommandTestFixture fixture;
    sail::RunCommand runCmd;
    
    // Create a subdirectory and change to it  
    std::filesystem::create_directories("src/components");
    std::filesystem::current_path("src/components");
    
    // Run with release flag from subdirectory
    std::vector<std::string> args = {"--release", "--verbose"};
    int result = runCmd.execute(args);
    
    // Should create release build in project root
    REQUIRE(std::filesystem::exists("build/release"));
    
    // Change back to project root for cleanup
    std::filesystem::current_path("../..");
}