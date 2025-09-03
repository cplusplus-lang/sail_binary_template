# System Commands and Global Setup Module
# Handles system commands from Sail.toml
# Note: Dependencies are now resolved per-target using dependency_resolver.cmake

# Initialize test link libraries list
set(SAIL_TEST_LINK_LIBRARIES "")

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