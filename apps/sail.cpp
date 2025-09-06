#include <cstdlib>
#include <exception>
#include <fmt/base.h>
#include <fmt/format.h>
#include <optional>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `sail`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
#include <sail/sample_library.hpp>
#include <string>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  constexpr int max_factorial_demo = 5;
  
  try {
    CLI::App app{ fmt::format("{} version {}", sail::cmake::project_name, sail::cmake::project_version) };

    std::optional<std::string> message;
    app.add_option("-m,--message", message, "A message to print back out");
    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");

    bool is_turn_based = false;
    auto *turn_based = app.add_flag("--turn_based", is_turn_based);

    bool is_loop_based = false;
    auto *loop_based = app.add_flag("--loop_based", is_loop_based);

    turn_based->excludes(loop_based);
    loop_based->excludes(turn_based);


    CLI11_PARSE(app, argc, argv);

    if (show_version) {
      fmt::print("{}\n", sail::cmake::project_version);
      return EXIT_SUCCESS;
    }

    if (message.has_value()) {
      fmt::print("Message: {}\n", message.value());
    }

    // Demonstrate sample library integration
    fmt::print("Sample Library Demo:\n");
    for (int i = 1; i <= max_factorial_demo; ++i) {
      fmt::print("  factorial({}) = {} (runtime)\n", i, factorial(i));
      fmt::print("  factorial({}) = {} (compile-time)\n", i, factorial_constexpr(i));
    }

  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }
}
