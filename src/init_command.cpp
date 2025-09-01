#include "init_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

namespace sail {

int InitCommand::execute(const std::vector<std::string>& args) {
    // Check for help first
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }
    
    std::string projectName;
    bool isBin = true;
    bool nameSpecified = false;
    
    // Parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--lib") {
            isBin = false;
        } else if (args[i] == "--bin") {
            isBin = true;
        } else if (args[i] == "--name") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: --name requires a value\n";
                printUsage();
                return 1;
            }
            projectName = args[++i];
            nameSpecified = true;
        } else if (args[i].substr(0, 2) == "--") {
            std::cerr << "Error: Unknown option '" << args[i] << "'\n";
            printUsage();
            return 1;
        } else {
            std::cerr << "Error: Unexpected argument '" << args[i] << "'\n";
            printUsage();
            return 1;
        }
    }
    
    // Use current directory name as project name if not specified
    if (!nameSpecified) {
        projectName = getCurrentDirectoryName();
    }
    
    // Sanitize the project name
    std::string sanitizedName = sanitizeProjectName(projectName);
    if (sanitizedName != projectName) {
        std::cout << "Note: Project name sanitized to '" << sanitizedName << "'\n";
        projectName = sanitizedName;
    }
    
    std::string projectPath = std::filesystem::current_path().string();
    
    // Check if Sail.toml already exists
    if (std::filesystem::exists(projectPath + "/Sail.toml")) {
        std::cerr << "Error: `sail init` cannot be run on existing Sail packages\n";
        return 1;
    }
    
    // Check if directory is not empty and has relevant files
    bool hasExistingSources = hasExistingSourceFiles(projectPath);
    
    std::cout << "Creating " << (isBin ? "binary" : "library") << " package in current directory\n";
    
    if (!initProject(projectName, projectPath, isBin)) {
        std::cerr << "Error: Failed to initialize project\n";
        return 1;
    }
    
    if (hasExistingSources) {
        std::cout << "Note: Existing source files found and preserved\n";
    }
    
    return 0;
}

bool InitCommand::initProject(const std::string& projectName, const std::string& projectPath, bool isBin) {
    // Create src directory if it doesn't exist
    std::string srcDir = projectPath + "/src";
    if (!std::filesystem::exists(srcDir)) {
        if (!Utils::createDirectoryRecursive(srcDir)) {
            std::cerr << "Error: Could not create src directory\n";
            return false;
        }
    }
    
    // For library projects, always create include directory
    if (!isBin) {
        std::string includeDir = projectPath + "/include";
        if (!std::filesystem::exists(includeDir)) {
            if (!Utils::createDirectoryRecursive(includeDir)) {
                std::cerr << "Error: Could not create include directory\n";
                return false;
            }
        }
    }
    
    // Check if source files already exist
    bool hasExistingSources = hasExistingSourceFiles(projectPath);
    
    // Create source file only if no existing source files
    if (!hasExistingSources) {
        if (!createSourceFile(projectPath, projectName, isBin)) {
            return false;
        }
    }
    
    // Create Sail.toml
    if (!createSailTomlFile(projectPath, projectName)) {
        return false;
    }
    
    // Create .gitignore if it doesn't exist
    if (!std::filesystem::exists(projectPath + "/.gitignore")) {
        if (!createGitIgnoreFile(projectPath)) {
            return false;
        }
    }
    
    return true;
}

bool InitCommand::createSourceFile(const std::string& projectPath, const std::string& projectName, bool isBin) {
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
        // Create include directory for library if it doesn't exist
        std::string includeDir = projectPath + "/include";
        if (!std::filesystem::exists(includeDir)) {
            if (!Utils::createDirectoryRecursive(includeDir)) {
                std::cerr << "Error: Could not create include directory\n";
                return false;
            }
        }
        
        // Create header file if it doesn't exist
        std::string headerFile = projectPath + "/include/" + projectName + ".h";
        if (!std::filesystem::exists(headerFile)) {
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
        }
        
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

bool InitCommand::createSailTomlFile(const std::string& projectPath, const std::string& projectName) {
    std::string tomlFile = projectPath + "/Sail.toml";
    std::ofstream file(tomlFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create Sail.toml\n";
        return false;
    }
    
    file << "[package]\n";
    file << "name = \"" << projectName << "\"\n";
    file << "version = \"1.0.0\"\n";
    file << "standard = \"17\"\n\n";
    file << "[dependencies]\n";
    
    file.close();
    return true;
}

bool InitCommand::createGitIgnoreFile(const std::string& projectPath) {
    std::string gitignoreFile = projectPath + "/.gitignore";
    std::ofstream file(gitignoreFile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not create .gitignore\n";
        return false;
    }
    
    file << "build/\n";
    file << "CMakeLists.txt\n";

    file.close();
    return true;
}

void InitCommand::printUsage() const {
    std::cout << "Create a new Sail package in the current directory\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail init [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "        --bin          Create a binary target (default)\n";
    std::cout << "        --lib          Create a library target\n";
    std::cout << "        --name <NAME>  Set the package name (defaults to directory name)\n";
    std::cout << "    -h, --help         Print help information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail init                    # Initialize as binary project\n";
    std::cout << "    sail init --lib              # Initialize as library project\n";
    std::cout << "    sail init --name my_project  # Initialize with specific name\n";
}

std::string InitCommand::sanitizeProjectName(const std::string& name) const {
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

std::string InitCommand::getCurrentDirectoryName() const {
    std::filesystem::path currentPath = std::filesystem::current_path();
    return currentPath.filename().string();
}

bool InitCommand::hasExistingSourceFiles(const std::string& projectPath) const {
    // Check for common source file patterns
    std::vector<std::string> sourcePatterns = {
        "/src/*.cpp", "/src/*.c", "/src/*.cc", "/src/*.cxx",
        "/include/*.h", "/include/*.hpp", "/include/*.hxx"
    };
    
    for (const auto& pattern : sourcePatterns) {
        std::string fullPattern = projectPath + pattern;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(projectPath + "/src")) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".cpp" || ext == ".c" || ext == ".cc" || ext == ".cxx") {
                        return true;
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Directory doesn't exist, continue
        }
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator(projectPath + "/include")) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".h" || ext == ".hpp" || ext == ".hxx") {
                        return true;
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Directory doesn't exist, continue
        }
    }
    
    return false;
}

} // namespace sail