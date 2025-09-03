# Sail TOML Parser Module
# Parses Sail.toml file for project name and version

function(sail_parse_toml)
    # Check if Sail.toml exists
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Sail.toml")
        message(FATAL_ERROR "Sail.toml not found in project root")
    endif()

    # Read the TOML file
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Sail.toml" SAIL_TOML_CONTENT)

    # Parse project name (look under [package] section)
    string(REGEX MATCH "\\[package\\][^\\[]*name = \"([^\"]+)\"" _ "${SAIL_TOML_CONTENT}")
    if(NOT CMAKE_MATCH_1)
        message(FATAL_ERROR "Could not parse project name from Sail.toml")
    endif()
    set(SAIL_PROJECT_NAME "${CMAKE_MATCH_1}" PARENT_SCOPE)

    # Parse project version (look under [package] section)
    string(REGEX MATCH "\\[package\\][^\\[]*version = \"([^\"]+)\"" _ "${SAIL_TOML_CONTENT}")
    if(NOT CMAKE_MATCH_1)
        message(WARNING "Could not parse project version from Sail.toml, using 1.0.0")
        set(SAIL_PROJECT_VERSION "1.0.0" PARENT_SCOPE)
    else()
        set(SAIL_PROJECT_VERSION "${CMAKE_MATCH_1}" PARENT_SCOPE)
    endif()

    # Parse C++ standard (look under [package] section)
    string(REGEX MATCH "\\[package\\][^\\[]*standard = \"([^\"]+)\"" _ "${SAIL_TOML_CONTENT}")
    if(NOT CMAKE_MATCH_1)
        message(WARNING "Could not parse C++ standard from Sail.toml, using 17")
        set(SAIL_CPP_STANDARD "17" PARENT_SCOPE)
    else()
        set(SAIL_CPP_STANDARD "${CMAKE_MATCH_1}" PARENT_SCOPE)
        # Validate the C++ standard
        set(VALID_STANDARDS 98 03 11 14 17 20 23 26)
        if(NOT "${CMAKE_MATCH_1}" IN_LIST VALID_STANDARDS)
            message(FATAL_ERROR "Invalid C++ standard '${CMAKE_MATCH_1}'. Valid options are: ${VALID_STANDARDS}")
        endif()
    endif()

    # Parse dependencies using string operations to handle arrays properly
    string(FIND "${SAIL_TOML_CONTENT}" "[dependencies]" DEPS_START)
    if(NOT DEPS_START EQUAL -1)
        # Calculate start position after section name
        math(EXPR DEPS_CONTENT_START "${DEPS_START} + 14")  # length of "[dependencies]"
        string(SUBSTRING "${SAIL_TOML_CONTENT}" ${DEPS_CONTENT_START} -1 REMAINING_CONTENT)
        
        # Find the next section by looking for [dev-dependencies] directly
        string(FIND "${REMAINING_CONTENT}" "[dev-dependencies]" DEV_DEPS_POS)
        if(NOT DEV_DEPS_POS EQUAL -1)
            string(SUBSTRING "${REMAINING_CONTENT}" 0 ${DEV_DEPS_POS} DEPENDENCIES_SECTION)
        else()
            # Look for [system.commands] as fallback
            string(FIND "${REMAINING_CONTENT}" "[system.commands]" SYS_CMD_POS) 
            if(NOT SYS_CMD_POS EQUAL -1)
                string(SUBSTRING "${REMAINING_CONTENT}" 0 ${SYS_CMD_POS} DEPENDENCIES_SECTION)
            else()
                set(DEPENDENCIES_SECTION "${REMAINING_CONTENT}")
            endif()
        endif()
        
        # Create lists for dependency names and values
        set(SAIL_DEPENDENCY_NAMES "")
        set(SAIL_DEPENDENCY_VALUES "")
        
        # Process dependencies line by line to avoid regex issues
        string(REPLACE "\n" ";" DEPS_LINES "${DEPENDENCIES_SECTION}")
        
        foreach(LINE ${DEPS_LINES})
            # Skip empty lines, comments, and section headers
            string(STRIP "${LINE}" LINE_TRIMMED)
            if(NOT LINE_TRIMMED STREQUAL "" AND NOT LINE_TRIMMED MATCHES "^#" AND NOT LINE_TRIMMED MATCHES "^\\[.*\\]$")
                
                # Check for simple string dependency: name = "value"  
                string(REGEX MATCH "^([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" SIMPLE_MATCH "${LINE_TRIMMED}")
                if(SIMPLE_MATCH)
                    list(APPEND SAIL_DEPENDENCY_NAMES "${CMAKE_MATCH_1}")
                    list(APPEND SAIL_DEPENDENCY_VALUES "${CMAKE_MATCH_2}")
                
                # Check for inline table dependency: name = { ... }
                elseif(LINE_TRIMMED MATCHES "^([a-zA-Z0-9_-]+) *= *\\{([^}]+)\\}")
                    string(REGEX MATCH "^([a-zA-Z0-9_-]+) *= *\\{([^}]+)\\}" _ "${LINE_TRIMMED}")
                    set(DEP_NAME "${CMAKE_MATCH_1}")
                    set(TABLE_CONTENT "${CMAKE_MATCH_2}")
                    
                    # Parse the inline table content
                    set(PARSED_TABLE "")
                    
                    # Extract version if present
                    string(REGEX MATCH "version *= *\"([^\"]+)\"" _ "${TABLE_CONTENT}")
                    if(CMAKE_MATCH_1)
                        set(PARSED_TABLE "${PARSED_TABLE}version=${CMAKE_MATCH_1};")
                    endif()
                    
                    # Extract components array if present
                    string(REGEX MATCH "components *= *\\[([^\\]]+)\\]" _ "${TABLE_CONTENT}")
                    if(CMAKE_MATCH_1)
                        set(COMPONENTS_RAW "${CMAKE_MATCH_1}")
                        # Clean up the components string - remove quotes and spaces
                        string(REGEX REPLACE "\"" "" COMPONENTS_CLEAN "${COMPONENTS_RAW}")
                        string(REGEX REPLACE " +" "" COMPONENTS_CLEAN "${COMPONENTS_CLEAN}")
                        set(PARSED_TABLE "${PARSED_TABLE}components=${COMPONENTS_CLEAN};")
                    endif()
                    
                    # Extract other key-value pairs
                    string(REGEX MATCHALL "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" OTHER_MATCHES "${TABLE_CONTENT}")
                    foreach(OTHER_MATCH ${OTHER_MATCHES})
                        string(REGEX MATCH "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" _ "${OTHER_MATCH}")
                        if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
                            # Skip version as we already handled it
                            if(NOT "${CMAKE_MATCH_1}" STREQUAL "version")
                                set(PARSED_TABLE "${PARSED_TABLE}${CMAKE_MATCH_1}=${CMAKE_MATCH_2};")
                            endif()
                        endif()
                    endforeach()
                    
                    list(APPEND SAIL_DEPENDENCY_NAMES "${DEP_NAME}")
                    list(APPEND SAIL_DEPENDENCY_VALUES "${PARSED_TABLE}")
                endif()
            endif()
        endforeach()
        
        set(SAIL_DEPENDENCY_NAMES "${SAIL_DEPENDENCY_NAMES}" PARENT_SCOPE)
        set(SAIL_DEPENDENCY_VALUES "${SAIL_DEPENDENCY_VALUES}" PARENT_SCOPE)
    else()
        set(SAIL_DEPENDENCY_NAMES "" PARENT_SCOPE)
        set(SAIL_DEPENDENCY_VALUES "" PARENT_SCOPE)
    endif()

    # Parse system commands (look under [system.commands] section)
    string(REGEX MATCH "\\[system\\.commands\\]([^\\[]*)" _ "${SAIL_TOML_CONTENT}")
    if(CMAKE_MATCH_1)
        set(SYSTEM_COMMANDS_SECTION "${CMAKE_MATCH_1}")
        # Find all system command lines: name = "value"
        string(REGEX MATCHALL "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" SYSTEM_CMD_MATCHES "${SYSTEM_COMMANDS_SECTION}")
        
        # Create lists for system command names and values
        set(SAIL_SYSTEM_CMD_NAMES "")
        set(SAIL_SYSTEM_CMD_VALUES "")
        
        foreach(MATCH ${SYSTEM_CMD_MATCHES})
            string(REGEX MATCH "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" _ "${MATCH}")
            if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
                list(APPEND SAIL_SYSTEM_CMD_NAMES "${CMAKE_MATCH_1}")
                list(APPEND SAIL_SYSTEM_CMD_VALUES "${CMAKE_MATCH_2}")
            endif()
        endforeach()
        
        set(SAIL_SYSTEM_CMD_NAMES "${SAIL_SYSTEM_CMD_NAMES}" PARENT_SCOPE)
        set(SAIL_SYSTEM_CMD_VALUES "${SAIL_SYSTEM_CMD_VALUES}" PARENT_SCOPE)
    else()
        set(SAIL_SYSTEM_CMD_NAMES "" PARENT_SCOPE)
        set(SAIL_SYSTEM_CMD_VALUES "" PARENT_SCOPE)
    endif()

    # Parse dev-dependencies (look under [dev-dependencies] section)
    string(REGEX MATCH "\\[dev-dependencies\\]([^\\[]*)" _ "${SAIL_TOML_CONTENT}")
    if(CMAKE_MATCH_1)
        set(DEV_DEPENDENCIES_SECTION "${CMAKE_MATCH_1}")
        # Find all dev-dependency lines: name = "value"
        string(REGEX MATCHALL "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" DEV_DEPENDENCY_MATCHES "${DEV_DEPENDENCIES_SECTION}")
        
        # Create lists for dev-dependency names and values
        set(SAIL_DEV_DEPENDENCY_NAMES "")
        set(SAIL_DEV_DEPENDENCY_VALUES "")
        
        foreach(MATCH ${DEV_DEPENDENCY_MATCHES})
            string(REGEX MATCH "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" _ "${MATCH}")
            if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
                list(APPEND SAIL_DEV_DEPENDENCY_NAMES "${CMAKE_MATCH_1}")
                list(APPEND SAIL_DEV_DEPENDENCY_VALUES "${CMAKE_MATCH_2}")
            endif()
        endforeach()
        
        set(SAIL_DEV_DEPENDENCY_NAMES "${SAIL_DEV_DEPENDENCY_NAMES}" PARENT_SCOPE)
        set(SAIL_DEV_DEPENDENCY_VALUES "${SAIL_DEV_DEPENDENCY_VALUES}" PARENT_SCOPE)
    else()
        set(SAIL_DEV_DEPENDENCY_NAMES "" PARENT_SCOPE)
        set(SAIL_DEV_DEPENDENCY_VALUES "" PARENT_SCOPE)
    endif()

    # Debug output (optional)
    message(STATUS "Sail project: ${SAIL_PROJECT_NAME} v${SAIL_PROJECT_VERSION}")
endfunction()
