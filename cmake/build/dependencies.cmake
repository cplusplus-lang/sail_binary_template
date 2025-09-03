# Dependency Management Module
# Handles dependencies and system commands from Sail.toml

# Dependencies via CPM
include(${CMAKE_CURRENT_LIST_DIR}/cpm.cmake)

# Initialize test link libraries list
set(SAIL_TEST_LINK_LIBRARIES "")

# Add dependencies from Sail.toml using modular system
if(SAIL_DEPENDENCY_NAMES AND SAIL_DEPENDENCY_VALUES)
    foreach(DEP_NAME DEP_VALUE IN ZIP_LISTS SAIL_DEPENDENCY_NAMES SAIL_DEPENDENCY_VALUES)
        # Convert dependency name to lowercase for file matching
        string(TOLOWER "${DEP_NAME}" DEP_NAME_LOWER)
        
        # Check if we have a specific handler for this dependency
        set(DEP_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../dependencies/${DEP_NAME_LOWER}.cmake")
        if(EXISTS "${DEP_HANDLER_FILE}")
            # Include the dependency handler
            include("${DEP_HANDLER_FILE}")
            
            # Call the dependency function with regular dependency type
            cmake_language(CALL "add_${DEP_NAME_LOWER}_dependency" "${DEP_VALUE}" "regular")
        else()
            # Fallback for unknown dependencies - try as CPM package directly
            message(WARNING "No specific handler found for dependency '${DEP_NAME}', trying as direct CPM package")
            CPMAddPackage("${DEP_VALUE}")
        endif()
    endforeach()
endif()

# Add system command dependencies from Sail.toml
if(SAIL_SYSTEM_CMD_NAMES AND SAIL_SYSTEM_CMD_VALUES)
    foreach(CMD_NAME CMD_VALUE IN ZIP_LISTS SAIL_SYSTEM_CMD_NAMES SAIL_SYSTEM_CMD_VALUES)
        # Convert command name to lowercase for file matching
        string(TOLOWER "${CMD_NAME}" CMD_NAME_LOWER)
        
        # Check if we have a specific handler for this system command
        set(CMD_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../system/commands/${CMD_NAME_LOWER}.cmake")
        if(EXISTS "${CMD_HANDLER_FILE}")
            # Include the system command handler
            include("${CMD_HANDLER_FILE}")
            
            # Call the system dependency function
            cmake_language(CALL "add_${CMD_NAME_LOWER}_dependency" "${CMD_VALUE}")
        else()
            # Fallback for unknown system commands - just try to find the program
            string(TOUPPER "${CMD_NAME}" CMD_NAME_UPPER)
            message(STATUS "No specific handler for system command '${CMD_NAME}', trying generic find_program")
            find_program(${CMD_NAME_UPPER}_EXECUTABLE ${CMD_NAME})
            if(NOT ${CMD_NAME_UPPER}_EXECUTABLE)
                message(WARNING "System command '${CMD_NAME}' not found")
            else()
                message(STATUS "Found system command: ${CMD_NAME} at ${${CMD_NAME_UPPER}_EXECUTABLE}")
            endif()
        endif()
    endforeach()
endif()