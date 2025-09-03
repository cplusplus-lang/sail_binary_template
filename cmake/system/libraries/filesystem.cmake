# C++17 Filesystem Support Module
# Detects and links C++17 filesystem library with fallback for older compilers

function(add_filesystem_support TARGET_NAME)
    # Include module for compile checks
    include(CheckCXXSourceCompiles)
    
    # Try to detect standard filesystem library
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
            message(STATUS "Using experimental::filesystem for ${TARGET_NAME}")
            # Link experimental filesystem library if needed
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
                target_link_libraries(${TARGET_NAME} stdc++fs)
            elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                find_library(CPP_FS_LIB c++experimental)
                if(CPP_FS_LIB)
                    target_link_libraries(${TARGET_NAME} ${CPP_FS_LIB})
                endif()
            endif()
        else()
            message(WARNING "No filesystem support found for ${TARGET_NAME}")
        endif()
    else()
        message(STATUS "Using std::filesystem for ${TARGET_NAME}")
    endif()
endfunction()