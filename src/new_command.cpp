#include "new_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

namespace sail {

int NewCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: new command requires a project name\n";
        printUsage();
        return 1;
    }
    
    // Check for help first
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }
    
    std::string projectName;
    bool isBin = true;
    
    // Parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--lib") {
            isBin = false;
        } else if (args[i] == "--bin") {
            isBin = true;
        } else if (args[i].substr(0, 2) == "--") {
            std::cerr << "Error: Unknown option '" << args[i] << "'\n";
            printUsage();
            return 1;
        } else if (projectName.empty()) {
            projectName = args[i];
        } else {
            std::cerr << "Error: Multiple project names specified: '" << projectName << "' and '" << args[i] << "'\n";
            printUsage();
            return 1;
        }
    }
    
    if (projectName.empty()) {
        std::cerr << "Error: new command requires a project name\n";
        printUsage();
        return 1;
    }
    
    // Handle path separators and extract actual project name
    std::filesystem::path inputPath(projectName);
    std::string actualProjectName = inputPath.filename().string();
    std::string sanitizedName = sanitizeProjectName(actualProjectName);
    
    if (sanitizedName != actualProjectName) {
        std::cout << "Note: Project name sanitized to '" << sanitizedName << "'\n";
        actualProjectName = sanitizedName;
    }
    
    // Build the full path, replacing the filename with the sanitized name if needed
    std::filesystem::path projectPath;
    if (inputPath.has_parent_path()) {
        projectPath = inputPath.parent_path() / actualProjectName;
    } else {
        projectPath = actualProjectName;
    }
    
    // Make path absolute
    if (projectPath.is_relative()) {
        projectPath = std::filesystem::current_path() / projectPath;
    }
    
    // Check if directory already exists
    if (std::filesystem::exists(projectPath)) {
        std::cerr << "Error: Directory '" << projectName << "' already exists\n";
        return 1;
    }
    
    std::cout << "Creating " << (isBin ? "binary" : "library") << " project '" << actualProjectName << "'...\n";
    
    if (!createProject(actualProjectName, projectPath.string(), isBin)) {
        std::cerr << "Error: Failed to create project\n";
        return 1;
    }
    
    std::cout << "Created " << (isBin ? "binary" : "library") << " project '" << actualProjectName << "'\n";
    return 0;
}

bool NewCommand::createProject(const std::string& projectName, const std::string& projectPath, bool isBin) {
    // Create project directory
    if (!Utils::createDirectoryRecursive(projectPath)) {
        std::cerr << "Error: Could not create directory: " << projectPath << std::endl;
        return false;
    }
    
    // Create src directory
    std::string srcDir = projectPath + "/src";
    if (!Utils::createDirectoryRecursive(srcDir)) {
        std::cerr << "Error: Could not create src directory\n";
        return false;
    }
    
    // Create CMakeLists.txt
    if (!createCMakeListsFile(projectPath, projectName, isBin)) {
        return false;
    }
    
    // Create source file
    if (!createSourceFile(projectPath, projectName, isBin)) {
        return false;
    }
    
    // Create Sail.toml
    if (!createSailTomlFile(projectPath, projectName)) {
        return false;
    }
    
    // Create .gitignore
    if (!createGitIgnoreFile(projectPath)) {
        return false;
    }
    
    return true;
}

bool NewCommand::createCMakeListsFile(const std::string& projectPath, const std::string& projectName, bool isBin) {
    std::string cmakeFile = projectPath + "/CMakeLists.txt";
    std::ofstream file(cmakeFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create CMakeLists.txt\n";
        return false;
    }
    
    file << "cmake_minimum_required(VERSION 3.20)\n";
    file << "project(" << projectName << " VERSION 1.0.0)\n\n";
    file << "set(CMAKE_CXX_STANDARD 17)\n";
    file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    
    if (isBin) {
        file << "add_executable(" << projectName << "\n";
        file << "    src/main.cpp\n";
        file << ")\n";
    } else {
        file << "add_library(" << projectName << "\n";
        file << "    src/lib.cpp\n";
        file << ")\n\n";
        file << "target_include_directories(" << projectName << " PUBLIC include)\n";
    }
    
    file.close();
    return true;
}

bool NewCommand::createSourceFile(const std::string& projectPath, const std::string& projectName, bool isBin) {
    std::string sourceFile;
    std::ofstream file;
    
    if (isBin) {
        sourceFile = projectPath + "/src/main.cpp";
        file.open(sourceFile);
        
        if (!file.is_open()) {
            std::cerr << "Error: Could not create main.cpp\n";
            return false;
        }
        
        file << "#include <iostream>\n\n";
        file << "int main() {\n";
        file << "    std::cout << \"Hello, world!\" << std::endl;\n";
        file << "    return 0;\n";
        file << "}\n";
    } else {
        // Create include directory for library
        std::string includeDir = projectPath + "/include";
        if (!Utils::createDirectoryRecursive(includeDir)) {
            std::cerr << "Error: Could not create include directory\n";
            return false;
        }
        
        // Create header file
        std::string headerFile = projectPath + "/include/" + projectName + ".h";
        std::ofstream headerStream(headerFile);
        if (!headerStream.is_open()) {
            std::cerr << "Error: Could not create header file\n";
            return false;
        }
        
        headerStream << "#pragma once\n\n";
        headerStream << "namespace " << projectName << " {\n\n";
        headerStream << "void hello();\n\n";
        headerStream << "} // namespace " << projectName << "\n";
        headerStream.close();
        
        // Create source file
        sourceFile = projectPath + "/src/lib.cpp";
        file.open(sourceFile);
        
        if (!file.is_open()) {
            std::cerr << "Error: Could not create lib.cpp\n";
            return false;
        }
        
        file << "#include \"" << projectName << ".h\"\n";
        file << "#include <iostream>\n\n";
        file << "namespace " << projectName << " {\n\n";
        file << "void hello() {\n";
        file << "    std::cout << \"Hello, world!\" << std::endl;\n";
        file << "}\n\n";
        file << "} // namespace " << projectName << "\n";
    }
    
    file.close();
    return true;
}

bool NewCommand::createSailTomlFile(const std::string& projectPath, const std::string& projectName) {
    std::string tomlFile = projectPath + "/Sail.toml";
    std::ofstream file(tomlFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create Sail.toml\n";
        return false;
    }
    
    file << "[package]\n";
    file << "name = \"" << projectName << "\"\n";
    file << "version = \"1.0.0\"\n";
    file << "description = \"A new Sail project\"\n\n";
    file << "[dependencies]\n";
    
    file.close();
    return true;
}

bool NewCommand::createGitIgnoreFile(const std::string& projectPath) {
    std::string gitignoreFile = projectPath + "/.gitignore";
    std::ofstream file(gitignoreFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create .gitignore\n";
        return false;
    }
    
    file << "# Build directories\n";
    file << "build/\n";
    file << "cmake-build-*/\n\n";
    file << "# IDE files\n";
    file << ".vscode/\n";
    file << ".clangd/\n";
    file << "*.swp\n";
    file << "*.swo\n\n";
    file << "# OS files\n";
    file << ".DS_Store\n";
    file << "Thumbs.db\n";
    
    file.close();
    return true;
}

void NewCommand::printUsage() const {
    std::cout << "Create a new Sail project\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail new [OPTIONS] <NAME>\n\n";
    std::cout << "ARGS:\n";
    std::cout << "    <NAME>    Name of the project to create\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "        --bin    Create a binary target (default)\n";
    std::cout << "        --lib    Create a library target\n";
    std::cout << "    -h, --help   Print help information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail new hello_world        # Create a binary project\n";
    std::cout << "    sail new --lib my_library   # Create a library project\n";
}

std::string NewCommand::sanitizeProjectName(const std::string& name) const {
    std::string sanitized = name;
    
    // Replace invalid characters with underscores
    std::regex invalidChars("[^a-zA-Z0-9_-]");
    sanitized = std::regex_replace(sanitized, invalidChars, "_");
    
    // Ensure it starts with a letter or underscore
    if (!sanitized.empty() && !std::isalpha(sanitized[0]) && sanitized[0] != '_') {
        sanitized = "_" + sanitized;
    }
    
    return sanitized;
}

} // namespace sail