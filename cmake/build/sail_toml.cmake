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

    # Parse dependencies (look under [dependencies] section)
    string(REGEX MATCH "\\[dependencies\\]([^\\[]*)" _ "${SAIL_TOML_CONTENT}")
    if(CMAKE_MATCH_1)
        set(DEPENDENCIES_SECTION "${CMAKE_MATCH_1}")
        # Find all dependency lines: name = "value"
        string(REGEX MATCHALL "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" DEPENDENCY_MATCHES "${DEPENDENCIES_SECTION}")
        
        # Create lists for dependency names and values
        set(SAIL_DEPENDENCY_NAMES "")
        set(SAIL_DEPENDENCY_VALUES "")
        
        foreach(MATCH ${DEPENDENCY_MATCHES})
            string(REGEX MATCH "([a-zA-Z0-9_-]+) *= *\"([^\"]+)\"" _ "${MATCH}")
            if(CMAKE_MATCH_1 AND CMAKE_MATCH_2)
                list(APPEND SAIL_DEPENDENCY_NAMES "${CMAKE_MATCH_1}")
                list(APPEND SAIL_DEPENDENCY_VALUES "${CMAKE_MATCH_2}")
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
