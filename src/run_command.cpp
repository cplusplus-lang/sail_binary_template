#include "run_command.h"
#include "build_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <sstream>

namespace sail {

int RunCommand::execute(const std::vector<std::string>& args) {
    // Check for help flag
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            printRunHelp();
            return 0;
        }
    }
    
    // Find project root and change to it
    std::string projectRoot = Utils::findProjectRoot();
    std::filesystem::path originalDir = std::filesystem::current_path();
    if (originalDir != std::filesystem::absolute(projectRoot)) {
        try {
            std::filesystem::current_path(projectRoot);
            std::cout << "Changed to project root: " << projectRoot << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not change to project root: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // Determine build configuration
    bool release = isReleaseBuild(args);
    bool verbose = isVerbose(args);
    std::string binaryName = getBinaryName(args);
    std::vector<std::string> execArgs = getExecutableArgs(args);
    
    // If no binary name specified, use project name
    if (binaryName.empty()) {
        binaryName = getProjectName();
        if (binaryName.empty()) {
            std::cerr << "Error: Could not determine binary name. Use --bin <name> or ensure CMakeLists.txt has a project name.\n";
            return 1;
        }
    }
    
    std::cout << "Building " << (release ? "release" : "debug") << " configuration...\n";
    
    // First, build the project
    int buildResult = buildProject(release, verbose);
    if (buildResult != 0) {
        std::cerr << "Error: Build failed\n";
        return buildResult;
    }
    
    // Find the executable
    std::string buildDir = getBuildDir(release);
    std::string executablePath = findExecutable(buildDir, binaryName);
    
    if (executablePath.empty()) {
        std::cerr << "Error: Could not find executable '" << binaryName << "' in " << buildDir << std::endl;
        return 1;
    }
    
    std::cout << "Running `" << executablePath;
    for (const auto& arg : execArgs) {
        std::cout << " " << arg;
    }
    std::cout << "`\n";
    
    // Run the executable
    return runExecutable(executablePath, execArgs);
}

bool RunCommand::isReleaseBuild(const std::vector<std::string>& args) const {
    for (const auto& arg : args) {
        if (arg == "--release" || arg == "-r") {
            return true;
        }
    }
    return false;
}

bool RunCommand::isVerbose(const std::vector<std::string>& args) const {
    for (const auto& arg : args) {
        if (arg == "--verbose" || arg == "-v") {
            return true;
        }
    }
    return false;
}

std::string RunCommand::getBinaryName(const std::vector<std::string>& args) const {
    for (size_t i = 0; i < args.size(); ++i) {
        if ((args[i] == "--bin" || args[i] == "-b") && i + 1 < args.size()) {
            return args[i + 1];
        }
    }
    return "";
}

std::vector<std::string> RunCommand::getExecutableArgs(const std::vector<std::string>& args) const {
    std::vector<std::string> execArgs;
    bool foundSeparator = false;
    
    for (const auto& arg : args) {
        if (foundSeparator) {
            execArgs.push_back(arg);
        } else if (arg == "--") {
            foundSeparator = true;
        }
    }
    
    return execArgs;
}

std::string RunCommand::getBuildDir(bool release) const {
    return release ? "build/release" : "build/debug";
}

std::string RunCommand::findExecutable(const std::string& buildDir, const std::string& binaryName) const {
    // Common executable locations in CMake build directories
    std::vector<std::string> possiblePaths = {
        buildDir + "/" + binaryName,
        buildDir + "/" + binaryName + ".exe",
        buildDir + "/Debug/" + binaryName,
        buildDir + "/Debug/" + binaryName + ".exe",
        buildDir + "/Release/" + binaryName,
        buildDir + "/Release/" + binaryName + ".exe",
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    return "";
}

int RunCommand::buildProject(bool release, bool verbose) const {
    BuildCommand buildCmd;
    std::vector<std::string> buildArgs;
    
    if (release) {
        buildArgs.push_back("--release");
    }
    if (verbose) {
        buildArgs.push_back("--verbose");
    }
    
    return buildCmd.execute(buildArgs);
}

int RunCommand::runExecutable(const std::string& executablePath, const std::vector<std::string>& execArgs) const {
    std::stringstream command;
    
    // Quote the executable path in case it contains spaces
    command << "\"" << executablePath << "\"";
    
    // Add arguments
    for (const auto& arg : execArgs) {
        // Simple quoting - wrap in quotes if contains spaces
        if (arg.find(' ') != std::string::npos) {
            command << " \"" << arg << "\"";
        } else {
            command << " " << arg;
        }
    }
    
    return std::system(command.str().c_str());
}

std::string RunCommand::getProjectName() const {
    // First try to get project name from Sail.toml
    std::ifstream sailFile("Sail.toml");
    if (sailFile) {
        std::string line;
        while (std::getline(sailFile, line)) {
            // Look for name = "project_name"
            size_t namePos = line.find("name = \"");
            if (namePos != std::string::npos) {
                size_t start = namePos + 8; // length of "name = \""
                size_t end = line.find("\"", start);
                if (end != std::string::npos) {
                    return line.substr(start, end - start);
                }
            }
        }
    }
    
    // Fall back to CMakeLists.txt for backwards compatibility
    std::ifstream file("CMakeLists.txt");
    if (!file) {
        return "";
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Look for project() declaration
        size_t pos = line.find("project(");
        if (pos != std::string::npos) {
            // Extract project name - handle both ${SAIL_PROJECT_NAME} and literal names
            size_t start = pos + 8; // length of "project("
            size_t end = line.find_first_of(" \t)", start);
            if (end != std::string::npos) {
                std::string projectDecl = line.substr(start, end - start);
                // If it's a CMake variable, we can't resolve it here, so try Sail.toml again
                if (projectDecl.find("${") != std::string::npos) {
                    // Already tried Sail.toml above, return empty if we couldn't parse it
                    return "";
                }
                return projectDecl;
            }
        }
    }
    
    return "";
}

void RunCommand::printRunHelp() const {
    std::cout << "Run the main binary of the local package (including all dependencies)\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail run [OPTIONS] [-- <args>...]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help           Print help information\n";
    std::cout << "    -r, --release        Build and run artifacts in release mode, with optimizations\n";
    std::cout << "    -v, --verbose        Use verbose output\n";
    std::cout << "    -b, --bin <NAME>     Name of the binary to run\n\n";
    std::cout << "ARGS:\n";
    std::cout << "    <args>...            Arguments to pass to the binary\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail run                        # Run the main binary\n";
    std::cout << "    sail run --release              # Run in release mode\n";
    std::cout << "    sail run --bin myapp            # Run specific binary\n";
    std::cout << "    sail run -- --help              # Pass --help to the binary\n";
    std::cout << "    sail run -- arg1 arg2           # Pass arguments to the binary\n";
}

} // namespace sail