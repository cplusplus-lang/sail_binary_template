#include "clean_command.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace sail {

int CleanCommand::execute(const std::vector<std::string>& args) {
    if (!parseArguments(args)) {
        return 1;
    }
    
    if (m_help) {
        printUsage();
        return 0;
    }
    
    // Find project root
    std::string projectRoot = Utils::findProjectRoot();
    std::filesystem::path originalDir = std::filesystem::current_path();
    
    // Change to project root if needed
    if (originalDir != std::filesystem::absolute(projectRoot)) {
        try {
            std::filesystem::current_path(projectRoot);
            if (!m_quiet) {
                std::cout << "Changed to project root: " << projectRoot << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not change to project root: " << e.what() << std::endl;
            return 1;
        }
    }
    
    // Handle doc cleaning
    if (m_doc) {
        std::string docDir = "build/doc";
        if (std::filesystem::exists(docDir)) {
            printRemovalMessage(docDir, m_dryRun, m_verbose);
            if (!m_dryRun) {
                try {
                    std::filesystem::remove_all(docDir);
                    if (!m_quiet) {
                        std::cout << "Removed documentation directory" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: Could not remove documentation directory: " << e.what() << std::endl;
                    return 1;
                }
            }
        } else if (m_verbose) {
            std::cout << "Documentation directory does not exist" << std::endl;
        }
        return 0;
    }
    
    // Clean specific build configurations or all
    bool success = true;
    if (m_release && m_debug) {
        // Clean both
        success = cleanAllBuildDirectories(m_dryRun, m_verbose);
    } else if (m_release) {
        // Clean only release
        success = cleanBuildDirectory("build/release", m_dryRun, m_verbose);
    } else if (m_debug) {
        // Clean only debug
        success = cleanBuildDirectory("build/debug", m_dryRun, m_verbose);
    } else {
        // Clean entire build directory (default behavior)
        success = cleanBuildDirectory("build", m_dryRun, m_verbose);
        
        
        // Also clean generated CMakeLists.txt if it exists
        if (std::filesystem::exists("CMakeLists.txt")) {
            printRemovalMessage("CMakeLists.txt", m_dryRun, m_verbose);
            if (!m_dryRun) {
                try {
                    std::filesystem::remove("CMakeLists.txt");
                    if (m_verbose) {
                        std::cout << "Removed generated CMakeLists.txt" << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Could not remove CMakeLists.txt: " << e.what() << std::endl;
                }
            }
        }
    }
    
    if (!success) {
        return 1;
    }
    
    if (!m_quiet && !m_dryRun) {
        std::cout << "Clean completed successfully" << std::endl;
    }
    
    return 0;
}

bool CleanCommand::parseArguments(const std::vector<std::string>& args) {
    for (const auto& arg : args) {
        if (arg == "--help" || arg == "-h") {
            m_help = true;
        } else if (arg == "--release") {
            m_release = true;
        } else if (arg == "--debug") {
            m_debug = true;
        } else if (arg == "--dry-run") {
            m_dryRun = true;
        } else if (arg == "--verbose" || arg == "-v") {
            m_verbose = true;
        } else if (arg == "--quiet" || arg == "-q") {
            m_quiet = true;
        } else if (arg == "--doc") {
            m_doc = true;
        } else if (arg.substr(0, 2) == "--") {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            printUsage();
            return false;
        } else {
            std::cerr << "Error: Unexpected argument '" << arg << "'" << std::endl;
            printUsage();
            return false;
        }
    }
    
    // Validate option combinations
    if (m_quiet && m_verbose) {
        std::cerr << "Error: Cannot specify both --quiet and --verbose" << std::endl;
        return false;
    }
    
    return true;
}

bool CleanCommand::cleanBuildDirectory(const std::string& buildDir, bool dryRun, bool verbose) {
    if (!std::filesystem::exists(buildDir)) {
        if (verbose) {
            std::cout << "Build directory '" << buildDir << "' does not exist" << std::endl;
        }
        return true;
    }
    
    printRemovalMessage(buildDir, dryRun, verbose);
    
    if (!dryRun) {
        try {
            std::filesystem::remove_all(buildDir);
            if (!m_quiet) {
                std::cout << "Removed '" << buildDir << "' directory" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not remove '" << buildDir << "' directory: " << e.what() << std::endl;
            return false;
        }
    }
    
    return true;
}

bool CleanCommand::cleanAllBuildDirectories(bool dryRun, bool verbose) {
    bool success = true;
    
    // Clean debug build
    if (!cleanBuildDirectory("build/debug", dryRun, verbose)) {
        success = false;
    }
    
    // Clean release build
    if (!cleanBuildDirectory("build/release", dryRun, verbose)) {
        success = false;
    }
    
    // Clean any other build artifacts in build directory
    if (std::filesystem::exists("build")) {
        try {
            bool hasOtherFiles = false;
            for (const auto& entry : std::filesystem::directory_iterator("build")) {
                if (entry.is_regular_file() || (entry.is_directory() && 
                    entry.path().filename() != "debug" && 
                    entry.path().filename() != "release")) {
                    hasOtherFiles = true;
                    break;
                }
            }
            
            if (hasOtherFiles && verbose) {
                std::cout << "Other build artifacts exist in build directory" << std::endl;
            }
        } catch (const std::exception& e) {
            if (verbose) {
                std::cout << "Could not inspect build directory: " << e.what() << std::endl;
            }
        }
    }
    
    return success;
}

void CleanCommand::printRemovalMessage(const std::string& path, bool dryRun, bool verbose) {
    if (m_quiet) {
        return;
    }
    
    if (dryRun) {
        std::cout << "Would remove: " << path;
        if (verbose && std::filesystem::exists(path)) {
            std::uintmax_t size = getDirectorySize(path);
            if (size > 0) {
                std::cout << " (" << formatFileSize(size) << ")";
            }
        }
        std::cout << std::endl;
    } else if (verbose) {
        std::cout << "Removing: " << path;
        if (std::filesystem::exists(path)) {
            std::uintmax_t size = getDirectorySize(path);
            if (size > 0) {
                std::cout << " (" << formatFileSize(size) << ")";
            }
        }
        std::cout << std::endl;
    }
}

std::string CleanCommand::formatFileSize(std::uintmax_t size) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double displaySize = static_cast<double>(size);
    
    while (displaySize >= 1024.0 && unitIndex < 4) {
        displaySize /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    if (unitIndex == 0) {
        oss << static_cast<std::uintmax_t>(displaySize) << " " << units[unitIndex];
    } else {
        oss << std::fixed << std::setprecision(1) << displaySize << " " << units[unitIndex];
    }
    
    return oss.str();
}

std::uintmax_t CleanCommand::getDirectorySize(const std::string& path) const {
    std::uintmax_t totalSize = 0;
    
    try {
        if (std::filesystem::is_regular_file(path)) {
            return std::filesystem::file_size(path);
        }
        
        if (std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    try {
                        totalSize += entry.file_size();
                    } catch (const std::exception&) {
                        // Skip files we can't read
                    }
                }
            }
        }
    } catch (const std::exception&) {
        // Return 0 if we can't calculate size
        return 0;
    }
    
    return totalSize;
}

void CleanCommand::printUsage() const {
    std::cout << "Remove generated build artifacts\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail clean [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "        --release        Remove only release build artifacts\n";
    std::cout << "        --debug          Remove only debug build artifacts\n";
    std::cout << "        --doc            Remove only documentation artifacts\n";
    std::cout << "        --dry-run        Display what would be deleted without deleting\n";
    std::cout << "    -v, --verbose        Use verbose output\n";
    std::cout << "    -q, --quiet          Do not print clean output\n";
    std::cout << "    -h, --help           Print help information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail clean                    # Remove all build artifacts\n";
    std::cout << "    sail clean --release          # Remove only release artifacts\n";
    std::cout << "    sail clean --debug            # Remove only debug artifacts\n";
    std::cout << "    sail clean --dry-run          # Show what would be removed\n";
    std::cout << "    sail clean --doc              # Remove documentation artifacts\n";
    std::cout << "    sail clean -v                 # Verbose output\n\n";
    std::cout << "By default, removes the entire 'build' directory and generated CMakeLists.txt.\n";
}

} // namespace sail