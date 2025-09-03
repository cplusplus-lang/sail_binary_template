# Binary Building Module
# Handles building the main binary executable

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

# Handle C++17 filesystem support with fallback
# Try to detect and link filesystem library if needed
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
    #include <filesystem>
    int main() { std::filesystem::current_path(); return 0; }
" HAVE_STD_FILESYSTEM)

if(NOT HAVE_STD_FILESYSTEM)
    # Try with experimental filesystem
    check_cxx_source_compiles("
        #include <experimental/filesystem>
        int main() { std::experimental::filesystem::current_path(); return 0; }
    " HAVE_EXPERIMENTAL_FILESYSTEM)
    
    if(HAVE_EXPERIMENTAL_FILESYSTEM)
        # Link experimental filesystem library if needed
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
            target_link_libraries(${SAIL_PROJECT_NAME} stdc++fs)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            find_library(CPP_FS_LIB c++experimental)
            if(CPP_FS_LIB)
                target_link_libraries(${SAIL_PROJECT_NAME} ${CPP_FS_LIB})
            endif()
        endif()
    endif()
endif()

# Install binary target
install(TARGETS ${SAIL_PROJECT_NAME} DESTINATION bin)