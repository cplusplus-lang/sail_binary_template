#include "cli.h"
#include "install_command.h"
#include "new_command.h"
#include "build_command.h"
#include "run_command.h"
#include "package_registry.h"
#include <iostream>
#include <iomanip>

namespace sail {

int CLI::run(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "--help" || command == "-h") {
        printHelp();
        return 0;
    }
    
    if (command == "--version" || command == "-V") {
        printVersion();
        return 0;
    }
    
    if (command == "install") {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return executeInstall(args);
    }
    
    if (command == "list") {
        return executeList();
    }
    
    if (command == "new") {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return executeNew(args);
    }
    
    if (command == "build") {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return executeBuild(args);
    }
    
    if (command == "run") {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return executeRun(args);
    }
    
    std::cerr << "Unknown command: " << command << std::endl;
    printHelp();
    return 1;
}

void CLI::printHelp() const {
    std::cout << "Sail - C++ Package Manager\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    sail [OPTIONS] <SUBCOMMAND>\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help       Print help information\n";
    std::cout << "    -V, --version    Print version information\n\n";
    std::cout << "SUBCOMMANDS:\n";
    std::cout << "    build            Build the current project\n";
    std::cout << "    run              Build and run the current project\n";
    std::cout << "    install          Install a package from a Git repository or package name\n";
    std::cout << "    list             List available packages in the registry\n";
    std::cout << "    new              Create a new Sail project\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    sail build                                          # Build in debug mode\n";
    std::cout << "    sail build --release                                # Build in release mode\n";
    std::cout << "    sail run                                            # Build and run the project\n";
    std::cout << "    sail run -- --help                                  # Run with arguments\n";
    std::cout << "    sail install names                                  # Install from registry\n";
    std::cout << "    sail install https://github.com/user/repo.git      # Install from URL\n";
    std::cout << "    sail list                                           # Show available packages\n";
    std::cout << "    sail new hello_world                                # Create a new binary project\n";
    std::cout << "    sail new --lib my_library                           # Create a new library project\n";
}

void CLI::printVersion() const {
    std::cout << "sail 1.0.0\n";
}

int CLI::executeInstall(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: install command requires a package name or URL\n";
        std::cerr << "Usage: sail install <package-name|git-url>\n";
        std::cerr << "       sail list                    # Show available packages\n";
        return 1;
    }
    
    InstallCommand installCmd;
    return installCmd.execute(args[0]);
}

int CLI::executeList() const {
    PackageRegistry registry;
    auto packages = registry.getAllPackages();
    
    if (packages.empty()) {
        std::cout << "No packages available in registry.\n";
        return 0;
    }
    
    std::cout << "Available packages:\n\n";
    
    // Calculate max width for alignment
    size_t maxNameWidth = 0;
    for (const auto& pkg : packages) {
        maxNameWidth = std::max(maxNameWidth, pkg.name.length());
    }
    
    for (const auto& pkg : packages) {
        std::cout << "  " << std::left << std::setw(maxNameWidth + 2) << pkg.name;
        std::cout << pkg.description;
        
        if (!pkg.binaries.empty()) {
            std::cout << "\n    " << std::string(maxNameWidth + 2, ' ') << "Binaries: ";
            for (size_t i = 0; i < pkg.binaries.size(); ++i) {
                std::cout << pkg.binaries[i];
                if (i < pkg.binaries.size() - 1) std::cout << ", ";
            }
        }
        std::cout << "\n\n";
    }
    
    std::cout << "Usage: sail install <package-name>\n";
    std::cout << "   or: sail install <git-url>\n";
    
    return 0;
}

int CLI::executeNew(const std::vector<std::string>& args) {
    NewCommand newCmd;
    return newCmd.execute(args);
}

int CLI::executeBuild(const std::vector<std::string>& args) {
    BuildCommand buildCmd;
    return buildCmd.execute(args);
}

int CLI::executeRun(const std::vector<std::string>& args) {
    RunCommand runCmd;
    return runCmd.execute(args);
}

} // namespace sail