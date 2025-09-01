#include <catch2/catch_test_macros.hpp>
#include "cli.h"
#include <sstream>
#include <iostream>
#include <chrono>

// Test fixture for CLI testing
class CLITestFixture {
public:
    CLITestFixture() {
        // Capture cout for testing output
        original_cout = std::cout.rdbuf();
        std::cout.rdbuf(captured_cout.rdbuf());
        
        // Capture cerr for testing error output
        original_cerr = std::cerr.rdbuf();
        std::cerr.rdbuf(captured_cerr.rdbuf());
    }

    ~CLITestFixture() {
        // Restore original streams
        std::cout.rdbuf(original_cout);
        std::cerr.rdbuf(original_cerr);
    }

    std::stringstream captured_cout;
    std::stringstream captured_cerr;
    std::streambuf* original_cout;
    std::streambuf* original_cerr;
    
    // Helper to create argv-style arguments
    std::vector<char*> makeArgs(const std::vector<std::string>& args) {
        argv_storage.clear();
        argv_storage.reserve(args.size());
        
        for (const auto& arg : args) {
            argv_storage.emplace_back(arg);
        }
        
        argv_ptrs.clear();
        argv_ptrs.reserve(args.size());
        for (auto& arg : argv_storage) {
            argv_ptrs.push_back(arg.data());
        }
        
        return argv_ptrs;
    }
    
private:
    std::vector<std::string> argv_storage;
    std::vector<char*> argv_ptrs;
};

TEST_CASE("No arguments shows help", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail"});
    
    int result = cli.run(1, args.data());
    
    REQUIRE(result == 1);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("Sail - C++ Package Manager") != std::string::npos);
    REQUIRE(output.find("USAGE:") != std::string::npos);
    REQUIRE(output.find("install") != std::string::npos);
}

TEST_CASE("Help flag shows help", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "--help"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 0);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("Sail - C++ Package Manager") != std::string::npos);
    REQUIRE(output.find("USAGE:") != std::string::npos);
    REQUIRE(output.find("install") != std::string::npos);
}

TEST_CASE("Short help flag shows help", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "-h"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 0);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("Sail - C++ Package Manager") != std::string::npos);
    REQUIRE(output.find("USAGE:") != std::string::npos);
    REQUIRE(output.find("install") != std::string::npos);
}

TEST_CASE("Version flag shows version", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "--version"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 0);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("sail 1.0.0") != std::string::npos);
}

TEST_CASE("Short version flag shows version", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "-V"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 0);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("sail 1.0.0") != std::string::npos);
}

TEST_CASE("Unknown command shows error", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "unknown-command"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 1);
    
    std::string error_output = fixture.captured_cerr.str();
    REQUIRE(error_output.find("Unknown command: unknown-command") != std::string::npos);
    
    std::string output = fixture.captured_cout.str();
    REQUIRE(output.find("USAGE:") != std::string::npos);
}

TEST_CASE("Install command without arguments shows error", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "install"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 1);
    
    std::string error_output = fixture.captured_cerr.str();
    REQUIRE(error_output.find("install command requires a package name or URL") != std::string::npos);
    REQUIRE(error_output.find("Usage: sail install [OPTIONS] <package-name|git-url>") != std::string::npos);
}

TEST_CASE("Install command with invalid url fails", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "install", "invalid-url"});
    
    int result = cli.run(3, args.data());
    
    // Should return non-zero for failure (actual behavior depends on InstallCommand implementation)
    REQUIRE(result != 0);
}

TEST_CASE("Help contains expected sections", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "--help"});
    
    cli.run(2, args.data());
    
    std::string output = fixture.captured_cout.str();
    
    // Check that all expected sections are present
    REQUIRE(output.find("Sail - C++ Package Manager") != std::string::npos);
    REQUIRE(output.find("USAGE:") != std::string::npos);
    REQUIRE(output.find("OPTIONS:") != std::string::npos);
    REQUIRE(output.find("SUBCOMMANDS:") != std::string::npos);
    REQUIRE(output.find("EXAMPLES:") != std::string::npos);
    
    // Check specific options
    REQUIRE(output.find("-h, --help") != std::string::npos);
    REQUIRE(output.find("-V, --version") != std::string::npos);
    
    // Check subcommands
    REQUIRE(output.find("install") != std::string::npos);
    REQUIRE(output.find("list") != std::string::npos);
    
    // Check examples
    REQUIRE(output.find("sail install names") != std::string::npos);
    REQUIRE(output.find("sail list") != std::string::npos);
}

TEST_CASE("Multiple arguments to install", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "install", "https://github.com/test/repo.git", "extra-arg"});
    
    int result = cli.run(4, args.data());
    
    // Should still process the first URL and ignore extra arguments
    // The actual result depends on the URL validity
    REQUIRE(result != 0); // Will fail because it's a fake URL
}

// Cross-platform behavior tests
TEST_CASE("Cross-platform path handling", "[CLI]") {
    CLITestFixture fixture;
    sail::CLI cli;
    
    // Test that the CLI can be instantiated and basic operations work
    // regardless of the platform
    std::vector<char*> args = fixture.makeArgs({"sail", "--version"});
    
    int result = cli.run(2, args.data());
    
    REQUIRE(result == 0);
    REQUIRE_FALSE(fixture.captured_cout.str().empty());
}

// Performance test for CLI startup
TEST_CASE("CLI startup performance", "[CLI][performance]") {
    CLITestFixture fixture;
    sail::CLI cli;
    std::vector<char*> args = fixture.makeArgs({"sail", "--help"});
    
    auto start = std::chrono::high_resolution_clock::now();
    cli.run(2, args.data());
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // CLI should start quickly (less than 1 second even on slow systems)
    REQUIRE(duration.count() < 1000);
}

// Test main function integration
TEST_CASE("Main function integration", "[main]") {
    // This is a simple test to ensure the main function can be called
    // We can't easily test the actual main() function without more complex setup,
    // but we can verify the CLI integration works
    
    sail::CLI cli;
    std::vector<std::string> args_vec = {"sail", "--version"};
    std::vector<char*> args;
    
    for (auto& arg : args_vec) {
        args.push_back(arg.data());
    }
    
    int result = cli.run(static_cast<int>(args.size()), args.data());
    REQUIRE(result == 0);
}