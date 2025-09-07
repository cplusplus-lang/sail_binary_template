#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <fmt/base.h>
#include <spdlog/spdlog.h>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `sail`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
#include <sail/template_manager.hpp>

int handle_new_command(const std::string& project_name, const std::filesystem::path& target_directory, bool overwrite) {
  try {
    sail::TemplateManager template_manager;
    
    if (template_manager.create_project(project_name, target_directory, overwrite)) {
      return EXIT_SUCCESS;
    } else {
      return EXIT_FAILURE;
    }
  } catch (const std::exception& e) {
    spdlog::error("Failed to create project: {}", e.what());
    return EXIT_FAILURE;
  }
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format("{} v{}", sail::cmake::project_name, sail::cmake::project_version) };
    app.description("A modern C++ project generator and build tool");
    app.require_subcommand(0, 1); // Allow 0 or 1 subcommand

    // Global flags
    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");

    // 'new' subcommand
    auto* new_cmd = app.add_subcommand("new", "Create a new project from template");
    
    std::string project_name;
    new_cmd->add_option("name", project_name, "Name of the new project")
           ->required();

    std::filesystem::path target_directory = std::filesystem::current_path();
    new_cmd->add_option("--path", target_directory, "Directory where to create the project")
           ->check(CLI::ExistingDirectory);

    bool overwrite = false;
    new_cmd->add_flag("--overwrite", overwrite, "Overwrite existing files");

    // Parse command line
    CLI11_PARSE(app, argc, argv);

    // Handle global flags first
    if (show_version) {
      fmt::print("Sail Build Tool v{}\n", sail::cmake::project_version);
      fmt::print("A modern C++ project generator and build tool\n");
      return EXIT_SUCCESS;
    }

    // Handle subcommands
    if (*new_cmd) {
      return handle_new_command(project_name, target_directory, overwrite);
    }

    // If no subcommand was provided, show help
    if (app.get_subcommands().empty() || app.get_subcommands()[0]->get_name().empty()) {
      fmt::print("{}\n", app.help());
      return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
    return EXIT_FAILURE;
  }
}
