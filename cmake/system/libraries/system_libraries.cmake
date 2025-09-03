# System Libraries Module
# Manages system library dependencies defined in Sail.toml

function(add_system_libraries TARGET_NAME)
    # Check if system dependencies are defined in Sail.toml
    if(SAIL_SYSTEM_DEPENDENCY_NAMES AND SAIL_SYSTEM_DEPENDENCY_VALUES)
        message(STATUS "Adding system libraries to ${TARGET_NAME}")
        
        # Process each system dependency
        foreach(DEP_NAME DEP_VALUE IN ZIP_LISTS SAIL_SYSTEM_DEPENDENCY_NAMES SAIL_SYSTEM_DEPENDENCY_VALUES)
            # Convert dependency name to lowercase for file matching
            string(TOLOWER "${DEP_NAME}" DEP_NAME_LOWER)
            
            # Check if we have a specific handler for this system library
            set(SYSTEM_LIB_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/system/libraries/${DEP_NAME_LOWER}.cmake")
            if(EXISTS "${SYSTEM_LIB_HANDLER_FILE}")
                # Include the system library handler
                include("${SYSTEM_LIB_HANDLER_FILE}")
                
                # Parse dependency value (could be version, components, etc.)
                if("${DEP_VALUE}" STREQUAL "true" OR "${DEP_VALUE}" STREQUAL "*")
                    # Simple dependency with no specific version/components
                    cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}")
                else()
                    # Parse complex dependency specification
                    # Format: "version=6 components=Core Widgets" or just "6" for version
                    string(FIND "${DEP_VALUE}" " " SPACE_POS)
                    if(SPACE_POS EQUAL -1)
                        # Simple version or single parameter
                        cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${DEP_VALUE}")
                    else()
                        # Parse key=value pairs
                        string(REPLACE " " ";" DEP_PARTS "${DEP_VALUE}")
                        set(VERSION "")
                        set(COMPONENTS "")
                        
                        foreach(PART ${DEP_PARTS})
                            string(FIND "${PART}" "=" EQUALS_POS)
                            if(NOT EQUALS_POS EQUAL -1)
                                string(SUBSTRING "${PART}" 0 ${EQUALS_POS} KEY)
                                math(EXPR VALUE_START "${EQUALS_POS} + 1")
                                string(SUBSTRING "${PART}" ${VALUE_START} -1 VALUE)
                                
                                if("${KEY}" STREQUAL "version")
                                    set(VERSION "${VALUE}")
                                elseif("${KEY}" STREQUAL "components")
                                    set(COMPONENTS "${VALUE}")
                                endif()
                            endif()
                        endforeach()
                        
                        # Call function with parsed parameters
                        if(VERSION AND COMPONENTS)
                            cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${VERSION}" "${COMPONENTS}")
                        elseif(VERSION)
                            cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${VERSION}")
                        else()
                            cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}")
                        endif()
                    endif()
                endif()
            else()
                message(WARNING "No specific handler found for system library '${DEP_NAME}', skipping")
            endif()
        endforeach()
    endif()
endfunction()