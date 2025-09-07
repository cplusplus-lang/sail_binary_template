# Template Embedding Module for Sail Project
#
# This module converts a directory of template files into embedded C++ code that can be
# compiled directly into the Sail binary. This eliminates the need for external template
# files at runtime and ensures `sail new` works out-of-the-box.
#
# ARCHITECTURE:
#   Template Directory (sail_binary_template/) → CMake Processing → Generated C++ → Runtime
#
# USAGE:
#   include(cmake/TemplateEmbedding.cmake)
#   sail_setup_template_embedding()
#
# GENERATED FILES:
#   - build/generated_templates/embedded_templates.hpp (C++ header)
#   - build/generated_templates/embedded_templates.cpp (C++ source)
#   - embedded_templates (CMake library target)
#
# FEATURES:
#   - Automatic file discovery and filtering
#   - Smart rebuilding (only when templates change)
#   - C++ string escaping
#   - CMake dependency tracking
#
# See docs/TEMPLATE_EMBEDDING.md for detailed documentation.

# Function to escape C++ string literals for safe embedding
# Converts raw file content into valid C++ string literal content
# 
# PARAMETERS:
#   INPUT - Raw string content from file
#   OUTPUT_VAR - Variable name to store escaped result
#
# ESCAPING RULES:
#   \ → \\    (backslash must be doubled)
#   " → \"    (quotes must be escaped)
#   newline → \n, carriage return → \r, tab → \t
function(escape_cpp_string INPUT OUTPUT_VAR)
    string(REPLACE "\\" "\\\\" ESCAPED "${INPUT}")
    string(REPLACE "\"" "\\\"" ESCAPED "${ESCAPED}")
    string(REPLACE "\n" "\\n" ESCAPED "${ESCAPED}")
    string(REPLACE "\r" "\\r" ESCAPED "${ESCAPED}")
    string(REPLACE "\t" "\\t" ESCAPED "${ESCAPED}")
    set(${OUTPUT_VAR} "${ESCAPED}" PARENT_SCOPE)
endfunction()

# Main function to set up template embedding system
# 
# This function:
#   1. Discovers all template files in sail_binary_template/
#   2. Filters out unwanted files (build artifacts, binaries, etc.)
#   3. Generates C++ code with embedded template content
#   4. Creates embedded_templates library target
#   5. Sets up proper CMake dependencies for rebuilding
#
# CALL THIS FROM: Root CMakeLists.txt after including this module
# CREATES: embedded_templates library target that can be linked
function(sail_setup_template_embedding)
    # Setup file paths for template processing
    set(TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sail_binary_template")
    set(GENERATED_TEMPLATES_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_templates")
    set(EMBEDDED_TEMPLATES_CPP "${GENERATED_TEMPLATES_DIR}/embedded_templates.cpp")
    set(EMBEDDED_TEMPLATES_HPP "${GENERATED_TEMPLATES_DIR}/embedded_templates.hpp")

    # Validate template directory exists
    if(NOT EXISTS "${TEMPLATE_DIR}")
        message(FATAL_ERROR "Template directory not found: ${TEMPLATE_DIR}")
    endif()

    # Create output directory
    file(MAKE_DIRECTORY "${GENERATED_TEMPLATES_DIR}")

    # Get all files in template directory recursively
    file(GLOB_RECURSE ALL_TEMPLATE_FILES 
        RELATIVE "${TEMPLATE_DIR}"
        "${TEMPLATE_DIR}/*"
    )

    # Filter out unwanted files and build full paths for dependency tracking
    # We maintain two lists: relative paths for embedding, full paths for dependencies
    set(TEMPLATE_FILES "")       # Relative paths (for embedding in generated code)
    set(TEMPLATE_FILE_PATHS "")  # Full paths (for CMake dependency tracking)
    
    foreach(FILE ${ALL_TEMPLATE_FILES})
        # Skip version control, build artifacts, and system files
        if(NOT FILE MATCHES "^\\.git" AND 
           NOT FILE MATCHES "^build/" AND
           NOT FILE MATCHES "^out/" AND
           NOT FILE MATCHES "\\.DS_Store$" AND
           NOT FILE MATCHES "\\.gitkeep$")
            
            # Skip binary files - we only embed text files for now
            # Binary support could be added later with base64 encoding
            get_filename_component(FILE_EXT "${FILE}" LAST_EXT)
            if(NOT FILE_EXT MATCHES "\\.(png|jpg|jpeg|gif|bmp|ico|exe|bin|so|dylib|dll)$")
                list(APPEND TEMPLATE_FILES "${FILE}")
                list(APPEND TEMPLATE_FILE_PATHS "${TEMPLATE_DIR}/${FILE}")
            endif()
        endif()
    endforeach()

    # Smart regeneration: only rebuild if templates have changed
    # This avoids expensive string processing on every build
    set(NEED_REGENERATE FALSE)
    
    # Always regenerate if output files don't exist
    if(NOT EXISTS "${EMBEDDED_TEMPLATES_CPP}" OR NOT EXISTS "${EMBEDDED_TEMPLATES_HPP}")
        set(NEED_REGENERATE TRUE)
    else()
        # Check if any template file is newer than generated files
        file(TIMESTAMP "${EMBEDDED_TEMPLATES_CPP}" GENERATED_TIME)
        foreach(TEMPLATE_PATH ${TEMPLATE_FILE_PATHS})
            file(TIMESTAMP "${TEMPLATE_PATH}" TEMPLATE_TIME)
            if("${TEMPLATE_TIME}" IS_NEWER_THAN "${GENERATED_TIME}")
                set(NEED_REGENERATE TRUE)
                break()  # No need to check remaining files
            endif()
        endforeach()
    endif()

    # Only regenerate if needed
    if(NOT NEED_REGENERATE)
        message(STATUS "Template files are up to date, skipping regeneration")
    else()
        # Generate header content
        list(LENGTH TEMPLATE_FILES FILE_COUNT)
        set(HEADER_CONTENT "#pragma once

#include <array>
#include <string_view>

namespace sail::templates {
    struct TemplateFile {
        std::string_view path;
        std::string_view content;
        bool is_binary;
    };
    
    extern const std::array<TemplateFile, ${FILE_COUNT}> embedded_files;
    constexpr size_t file_count = ${FILE_COUNT};
}
")

        # Generate source content
        set(SOURCE_CONTENT "#include \"embedded_templates.hpp\"
#include <array>
#include <string_view>

namespace sail::templates {
")

        # Process each file
        set(FILE_INDEX 0)
        set(FILE_DECLARATIONS "")
        
        foreach(FILE ${TEMPLATE_FILES})
            set(FULL_PATH "${TEMPLATE_DIR}/${FILE}")
            
            # Read and escape file content
            file(READ "${FULL_PATH}" FILE_CONTENT)
            escape_cpp_string("${FILE_CONTENT}" ESCAPED_CONTENT)
            escape_cpp_string("${FILE}" ESCAPED_PATH)
            
            # Add to source content
            string(APPEND SOURCE_CONTENT "    static constexpr std::string_view file_${FILE_INDEX}_path = \"${ESCAPED_PATH}\";\n")
            string(APPEND SOURCE_CONTENT "    static constexpr std::string_view file_${FILE_INDEX}_content = \"${ESCAPED_CONTENT}\";\n\n")
            
            # Add to declarations
            if(FILE_INDEX GREATER 0)
                string(APPEND FILE_DECLARATIONS ",\n")
            endif()
            string(APPEND FILE_DECLARATIONS "        {.path=file_${FILE_INDEX}_path, .content=file_${FILE_INDEX}_content, .is_binary=false}")
            
            math(EXPR FILE_INDEX "${FILE_INDEX} + 1")
        endforeach()

        # Complete the source file
        string(APPEND SOURCE_CONTENT "    const std::array<TemplateFile, ${FILE_COUNT}> embedded_files = {{\n${FILE_DECLARATIONS}\n    }};\n}")

        # Write the files
        file(WRITE "${EMBEDDED_TEMPLATES_HPP}" "${HEADER_CONTENT}")
        file(WRITE "${EMBEDDED_TEMPLATES_CPP}" "${SOURCE_CONTENT}")

        message(STATUS "Generated embedded templates: ${FILE_COUNT} files")
    endif()

    # Create embedded templates library
    add_library(embedded_templates STATIC ${EMBEDDED_TEMPLATES_CPP})
    target_include_directories(embedded_templates PUBLIC ${GENERATED_TEMPLATES_DIR})
    target_link_libraries(embedded_templates PRIVATE sail_options)

    # Set up proper dependencies so CMake knows to reconfigure when templates change
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TEMPLATE_FILE_PATHS})
endfunction()