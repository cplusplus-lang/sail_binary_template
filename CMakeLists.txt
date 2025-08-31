cmake_minimum_required(VERSION 3.20)
project(sail VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Git QUIET)
if(NOT Git_FOUND)
    message(FATAL_ERROR "Git is required for sail to function")
endif()

add_executable(sail
    src/main.cpp
    src/cli.cpp
    src/install_command.cpp
    src/new_command.cpp
    src/build_command.cpp
    src/run_command.cpp
    src/utils.cpp
    src/git_utils.cpp
    src/cmake_builder.cpp
)

target_include_directories(sail PRIVATE include)

if(WIN32)
    target_compile_definitions(sail PRIVATE SAIL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(sail PRIVATE SAIL_PLATFORM_MACOS)
else()
    target_compile_definitions(sail PRIVATE SAIL_PLATFORM_LINUX)
endif()

# Testing
include(FetchContent)

# Fetch Catch2
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.10.0
)

FetchContent_MakeAvailable(Catch2)

# Create a library for the main functionality (excluding main.cpp)
add_library(saillib
    src/cli.cpp
    src/install_command.cpp
    src/new_command.cpp
    src/build_command.cpp
    src/run_command.cpp
    src/utils.cpp
    src/git_utils.cpp
    src/cmake_builder.cpp
    src/package_registry.cpp
)

target_include_directories(saillib PUBLIC include)

if(WIN32)
    target_compile_definitions(saillib PUBLIC SAIL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(saillib PUBLIC SAIL_PLATFORM_MACOS)
else()
    target_compile_definitions(saillib PUBLIC SAIL_PLATFORM_LINUX)
endif()

# Link the library to the main executable
target_link_libraries(sail saillib)

# Test executable
enable_testing()

add_executable(sail_tests
    tests/test_utils.cpp
    tests/test_git_utils.cpp
    tests/test_install_command.cpp
    tests/test_new_command.cpp
    tests/test_build_command.cpp
    tests/test_run_command.cpp
    tests/test_main.cpp
    tests/test_package_registry.cpp
)

target_link_libraries(sail_tests
    saillib
    Catch2::Catch2WithMain
)

add_test(NAME sail_tests COMMAND sail_tests)

install(TARGETS sail DESTINATION bin)