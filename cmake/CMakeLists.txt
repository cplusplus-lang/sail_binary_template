# Build options - must be explicitly set
option(BUILD_BINARY "Build the main binary executable" OFF)
option(BUILD_TESTS "Build the test executable" OFF)

# Ensure at least one build type is selected
if(NOT BUILD_BINARY AND NOT BUILD_TESTS)
    message(FATAL_ERROR "You must specify either -DBUILD_BINARY=ON or -DBUILD_TESTS=ON (or both)")
endif()

set(CMAKE_CXX_STANDARD ${SAIL_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include dependency management
include(${CMAKE_CURRENT_LIST_DIR}/build/dependencies.cmake)

# Include embedded CMake files generation module
include(${CMAKE_CURRENT_SOURCE_DIR}/EmbedCMakeFiles.cmake)

# Setup embedded CMake files generation
setup_embedded_cmake_files()

# Include binary building logic only if requested
if(BUILD_BINARY)
    include(${CMAKE_CURRENT_LIST_DIR}/build/binary.cmake)
    # Ensure embedded CMake files are generated before building binary
    add_dependencies(${SAIL_PROJECT_NAME} generate_embedded_cmake)
endif()

# Include test building logic only if requested
if(BUILD_TESTS)
    include(${CMAKE_CURRENT_LIST_DIR}/build/tests.cmake)
    # Ensure embedded CMake files are generated before building tests
    add_dependencies(${SAIL_PROJECT_NAME}_tests generate_embedded_cmake)
endif()
