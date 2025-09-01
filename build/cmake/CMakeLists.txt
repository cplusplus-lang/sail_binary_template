set(CMAKE_CXX_STANDARD ${SAIL_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files from src directory
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

add_executable(${SAIL_PROJECT_NAME} ${PROJECT_SOURCES})

# Add include directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_include_directories(${SAIL_PROJECT_NAME} PRIVATE include)
endif()

if(WIN32)
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_MACOS)
else()
    target_compile_definitions(${SAIL_PROJECT_NAME} PRIVATE SAIL_PLATFORM_LINUX)
endif()

# Testing
include(${CMAKE_CURRENT_LIST_DIR}/CPM.cmake)

# Add Catch2 via CPM
CPMAddPackage("gh:catchorg/Catch2@3.10.0")

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

# Test executable
enable_testing()

# Collect test source files and exclude main.cpp from project sources for tests
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
file(GLOB_RECURSE TEST_PROJECT_SOURCES "src/*.cpp")
list(REMOVE_ITEM TEST_PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(${SAIL_PROJECT_NAME}_tests
    ${TEST_SOURCES}
    ${TEST_PROJECT_SOURCES}
)

target_include_directories(${SAIL_PROJECT_NAME}_tests PRIVATE include)

target_link_libraries(${SAIL_PROJECT_NAME}_tests
    Catch2::Catch2WithMain
)

add_test(NAME ${SAIL_PROJECT_NAME}_tests COMMAND ${SAIL_PROJECT_NAME}_tests)

install(TARGETS ${SAIL_PROJECT_NAME} DESTINATION bin)
