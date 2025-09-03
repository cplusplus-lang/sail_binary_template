# Binary Building Module
# Handles building the main binary executable

message(STATUS "=== Binary.cmake is being executed ===")

# Collect all source files from src directory
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

add_executable(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES})

# Add include directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_include_directories(${SAIL_PROJECT_NAME} PRIVATE include)
endif()

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_MACOS)
else()
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_LINUX)
endif()

# Include unified dependency resolver
include(${CMAKE_CURRENT_LIST_DIR}/dependency_resolver.cmake)

# Resolve all dependencies (system and downloaded)
resolve_all_dependencies(${SAIL_PROJECT_NAME})

# Install binary target
install(TARGETS ${SAIL_PROJECT_NAME} DESTINATION bin)