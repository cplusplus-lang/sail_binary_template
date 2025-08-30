#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cli.h"
#include <sstream>
#include <iostream>
#include <chrono>

// Test fixture for CLI testing
class CLITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Capture cout for testing output
        original_cout = std::cout.rdbuf();
        std::cout.rdbuf(captured_cout.rdbuf());
        
        // Capture cerr for testing error output
        original_cerr = std::cerr.rdbuf();
        std::cerr.rdbuf(captured_cerr.rdbuf());
    }

    void TearDown() override {
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

TEST_F(CLITest, NoArgumentsShowsHelp) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail"});
    
    int result = cli.run(1, args.data());
    
    EXPECT_EQ(result, 1);
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("Sail - C++ Package Manager"));
    EXPECT_THAT(output, ::testing::HasSubstr("USAGE:"));
    EXPECT_THAT(output, ::testing::HasSubstr("install"));
}

TEST_F(CLITest, HelpFlagShowsHelp) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "--help"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 0);
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("Sail - C++ Package Manager"));
    EXPECT_THAT(output, ::testing::HasSubstr("USAGE:"));
    EXPECT_THAT(output, ::testing::HasSubstr("install"));
}

TEST_F(CLITest, ShortHelpFlagShowsHelp) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "-h"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 0);
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("Sail - C++ Package Manager"));
    EXPECT_THAT(output, ::testing::HasSubstr("USAGE:"));
    EXPECT_THAT(output, ::testing::HasSubstr("install"));
}

TEST_F(CLITest, VersionFlagShowsVersion) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "--version"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 0);
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("sail 1.0.0"));
}

TEST_F(CLITest, ShortVersionFlagShowsVersion) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "-V"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 0);
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("sail 1.0.0"));
}

TEST_F(CLITest, UnknownCommandShowsError) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "unknown-command"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 1);
    
    std::string error_output = captured_cerr.str();
    EXPECT_THAT(error_output, ::testing::HasSubstr("Unknown command: unknown-command"));
    
    std::string output = captured_cout.str();
    EXPECT_THAT(output, ::testing::HasSubstr("USAGE:"));
}

TEST_F(CLITest, InstallCommandWithoutArgumentsShowsError) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "install"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 1);
    
    std::string error_output = captured_cerr.str();
    EXPECT_THAT(error_output, ::testing::HasSubstr("install command requires a package name or URL"));
    EXPECT_THAT(error_output, ::testing::HasSubstr("Usage: sail install <package-name|git-url>"));
}

TEST_F(CLITest, InstallCommandWithInvalidUrlFails) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "install", "invalid-url"});
    
    int result = cli.run(3, args.data());
    
    // Should return non-zero for failure (actual behavior depends on InstallCommand implementation)
    EXPECT_NE(result, 0);
}

TEST_F(CLITest, HelpContainsExpectedSections) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "--help"});
    
    cli.run(2, args.data());
    
    std::string output = captured_cout.str();
    
    // Check that all expected sections are present
    EXPECT_THAT(output, ::testing::HasSubstr("Sail - C++ Package Manager"));
    EXPECT_THAT(output, ::testing::HasSubstr("USAGE:"));
    EXPECT_THAT(output, ::testing::HasSubstr("OPTIONS:"));
    EXPECT_THAT(output, ::testing::HasSubstr("SUBCOMMANDS:"));
    EXPECT_THAT(output, ::testing::HasSubstr("EXAMPLES:"));
    
    // Check specific options
    EXPECT_THAT(output, ::testing::HasSubstr("-h, --help"));
    EXPECT_THAT(output, ::testing::HasSubstr("-V, --version"));
    
    // Check subcommands
    EXPECT_THAT(output, ::testing::HasSubstr("install"));
    EXPECT_THAT(output, ::testing::HasSubstr("list"));
    
    // Check examples
    EXPECT_THAT(output, ::testing::HasSubstr("sail install names"));
    EXPECT_THAT(output, ::testing::HasSubstr("sail list"));
}

TEST_F(CLITest, MultipleArgumentsToInstall) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "install", "https://github.com/test/repo.git", "extra-arg"});
    
    int result = cli.run(4, args.data());
    
    // Should still process the first URL and ignore extra arguments
    // The actual result depends on the URL validity
    EXPECT_NE(result, 0); // Will fail because it's a fake URL
}

// Cross-platform behavior tests
TEST_F(CLITest, CrossPlatformPathHandling) {
    sail::CLI cli;
    
    // Test that the CLI can be instantiated and basic operations work
    // regardless of the platform
    std::vector<char*> args = makeArgs({"sail", "--version"});
    
    int result = cli.run(2, args.data());
    
    EXPECT_EQ(result, 0);
    EXPECT_FALSE(captured_cout.str().empty());
}

// Performance test for CLI startup
TEST_F(CLITest, CLIStartupPerformance) {
    sail::CLI cli;
    std::vector<char*> args = makeArgs({"sail", "--help"});
    
    auto start = std::chrono::high_resolution_clock::now();
    cli.run(2, args.data());
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // CLI should start quickly (less than 1 second even on slow systems)
    EXPECT_LT(duration.count(), 1000);
}

// Test main function integration
TEST(MainIntegration, MainFunctionExists) {
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
    EXPECT_EQ(result, 0);
}