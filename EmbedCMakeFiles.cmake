# Embedded CMake Files Generation Module
# Generates C++ header and source files from CMake files for embedding in the binary

# Convert a file path to a valid C++ identifier
function(cmake_path_to_identifier INPUT_PATH OUTPUT_VAR)
    string(REGEX REPLACE "\\.[^.]*$" "" PATH_NO_EXT "${INPUT_PATH}")
    string(REGEX REPLACE "[/\\\\.-]" "_" IDENTIFIER "${PATH_NO_EXT}")
    string(TOUPPER "${IDENTIFIER}" IDENTIFIER)
    set(${OUTPUT_VAR} "${IDENTIFIER}" PARENT_SCOPE)
endfunction()

# Generate C++ files from CMake files
function(generate_embedded_cmake_file CMAKE_FILE_PATH OUTPUT_DIR)
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_FILE_PATH}")
    get_filename_component(FILE_NAME "${REL_PATH}" NAME_WE)
    get_filename_component(FILE_DIR "${REL_PATH}" DIRECTORY)
    
    if(FILE_DIR)
        set(OUTPUT_SUBDIR "${OUTPUT_DIR}/${FILE_DIR}")
    else()
        set(OUTPUT_SUBDIR "${OUTPUT_DIR}")
    endif()
    
    set(OUTPUT_FILE "${OUTPUT_SUBDIR}/${FILE_NAME}.h")
    set(CPP_SOURCE_FILE "${OUTPUT_SUBDIR}/${FILE_NAME}.cpp")
    file(MAKE_DIRECTORY "${OUTPUT_SUBDIR}")
    
    cmake_path_to_identifier("${REL_PATH}" IDENTIFIER)
    file(READ "${CMAKE_FILE_PATH}" FILE_CONTENT)
    set(HEADER_GUARD "EMBEDDED_CMAKE_${IDENTIFIER}_H")
    
    # Generate header file
    set(CPP_CONTENT "#ifndef ${HEADER_GUARD}
#define ${HEADER_GUARD}

#include <string_view>

namespace embedded_cmake {

extern const std::string_view ${IDENTIFIER};

} // namespace embedded_cmake

#endif // ${HEADER_GUARD}
")
    
    # Generate source file
    set(CPP_SOURCE_CONTENT "#include \"${FILE_NAME}.h\"

namespace embedded_cmake {

const std::string_view ${IDENTIFIER} = R\"EMBED_CMAKE(${FILE_CONTENT})EMBED_CMAKE\";

} // namespace embedded_cmake
")
    
    # Write files only if content changed
    set(SHOULD_WRITE TRUE)
    if(EXISTS "${OUTPUT_FILE}")
        file(READ "${OUTPUT_FILE}" EXISTING_CONTENT)
        if("${EXISTING_CONTENT}" STREQUAL "${CPP_CONTENT}")
            set(SHOULD_WRITE FALSE)
        endif()
    endif()
    
    if(SHOULD_WRITE)
        file(WRITE "${OUTPUT_FILE}" "${CPP_CONTENT}")
        message(STATUS "Generated embedded CMake header: ${OUTPUT_FILE}")
    endif()
    
    set(SHOULD_WRITE_CPP TRUE)
    if(EXISTS "${CPP_SOURCE_FILE}")
        file(READ "${CPP_SOURCE_FILE}" EXISTING_CPP_CONTENT)
        if("${EXISTING_CPP_CONTENT}" STREQUAL "${CPP_SOURCE_CONTENT}")
            set(SHOULD_WRITE_CPP FALSE)
        endif()
    endif()
    
    if(SHOULD_WRITE_CPP)
        file(WRITE "${CPP_SOURCE_FILE}" "${CPP_SOURCE_CONTENT}")
        message(STATUS "Generated embedded CMake source: ${CPP_SOURCE_FILE}")
    endif()
endfunction()

# Main function to generate all embedded CMake files
function(setup_embedded_cmake_files)
    # Generate all embedded CMake files
    set(CMAKE_EMBED_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/.sail")
    file(GLOB_RECURSE CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
    list(FILTER CMAKE_FILES EXCLUDE REGEX "CMakeLists\\.txt$")

    foreach(CMAKE_FILE ${CMAKE_FILES})
        generate_embedded_cmake_file("${CMAKE_FILE}" "${CMAKE_EMBED_OUTPUT_DIR}")
    endforeach()

    # Create master header
    set(MASTER_HEADER "${CMAKE_EMBED_OUTPUT_DIR}/embedded_cmake_files.h")
    set(MASTER_CONTENT "#ifndef EMBEDDED_CMAKE_FILES_H
#define EMBEDDED_CMAKE_FILES_H

// Auto-generated header that includes all embedded CMake files

")

    foreach(CMAKE_FILE ${CMAKE_FILES})
        file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_FILE}")
        get_filename_component(FILE_NAME "${REL_PATH}" NAME_WE)
        get_filename_component(FILE_DIR "${REL_PATH}" DIRECTORY)
        
        if(FILE_DIR)
            set(INCLUDE_PATH "${FILE_DIR}/${FILE_NAME}.h")
        else()
            set(INCLUDE_PATH "${FILE_NAME}.h")
        endif()
        
        string(APPEND MASTER_CONTENT "#include \"${INCLUDE_PATH}\"\n")
    endforeach()

    string(APPEND MASTER_CONTENT "
#endif // EMBEDDED_CMAKE_FILES_H
")

    set(SHOULD_WRITE_MASTER TRUE)
    if(EXISTS "${MASTER_HEADER}")
        file(READ "${MASTER_HEADER}" EXISTING_MASTER_CONTENT)
        if("${EXISTING_MASTER_CONTENT}" STREQUAL "${MASTER_CONTENT}")
            set(SHOULD_WRITE_MASTER FALSE)
        endif()
    endif()

    if(SHOULD_WRITE_MASTER)
        file(WRITE "${MASTER_HEADER}" "${MASTER_CONTENT}")
        message(STATUS "Generated master embedded CMake header: ${MASTER_HEADER}")
    endif()

    # Create custom target for dependency tracking
    file(GLOB_RECURSE CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
    list(FILTER CMAKE_FILES EXCLUDE REGEX "CMakeLists\\.txt$")
    
    add_custom_target(generate_embedded_cmake
        DEPENDS ${CMAKE_FILES}
        COMMENT "Embedded CMake files are up to date"
    )
endfunction()