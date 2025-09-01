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

    # Debug output (optional)
    message(STATUS "Sail project: ${SAIL_PROJECT_NAME} v${SAIL_PROJECT_VERSION}")
endfunction()
