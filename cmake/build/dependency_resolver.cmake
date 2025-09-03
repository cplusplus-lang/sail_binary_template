# Unified Dependency Resolver
# Handles both system libraries and downloaded packages
# Tries system first, falls back to CPM download if needed

# Function to resolve a single dependency
function(resolve_dependency DEP_NAME DEP_VALUE DEPENDENCY_TYPE TARGET_NAME)
    # Convert dependency name to lowercase for file matching
    string(TOLOWER "${DEP_NAME}" DEP_NAME_LOWER)
    
    message(STATUS "Resolving ${DEPENDENCY_TYPE} dependency: ${DEP_NAME} = ${DEP_VALUE}")
    
    # Check if we have a system library handler first
    set(SYSTEM_LIB_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../system/libraries/${DEP_NAME_LOWER}.cmake")
    if(EXISTS "${SYSTEM_LIB_HANDLER_FILE}")
        message(STATUS "Found system library handler for ${DEP_NAME}, attempting system resolution")
        
        # Try to resolve as system library
        set(SYSTEM_RESOLUTION_SUCCESS FALSE)
        try_system_dependency("${DEP_NAME}" "${DEP_VALUE}" "${TARGET_NAME}")
        if(SYSTEM_RESOLUTION_SUCCESS)
            message(STATUS "Successfully resolved ${DEP_NAME} as system library")
            return()
        else()
            message(STATUS "System resolution failed for ${DEP_NAME}, falling back to CPM download")
        endif()
    endif()
    
    # Check if we have a CPM dependency handler
    set(CPM_DEP_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../dependencies/${DEP_NAME_LOWER}.cmake")
    if(EXISTS "${CPM_DEP_HANDLER_FILE}")
        message(STATUS "Found CPM handler for ${DEP_NAME}, downloading via CPM")
        
        # Include the dependency handler
        include("${CPM_DEP_HANDLER_FILE}")
        
        # Call the dependency function
        cmake_language(CALL "add_${DEP_NAME_LOWER}_dependency" "${DEP_VALUE}" "${DEPENDENCY_TYPE}")
        
        message(STATUS "Successfully resolved ${DEP_NAME} via CPM download")
    else()
        # Fallback: try as direct CPM package
        message(STATUS "No specific handler found for ${DEP_NAME}, trying direct CPM")
        
        # Include CPM
        include(${CMAKE_CURRENT_LIST_DIR}/cpm.cmake)
        
        # Try to add as generic CPM package
        if("${DEP_VALUE}" STREQUAL "*" OR "${DEP_VALUE}" STREQUAL "latest")
            CPMAddPackage("gh:${DEP_NAME}/${DEP_NAME}@main")
        else()
            CPMAddPackage("gh:${DEP_NAME}/${DEP_NAME}@${DEP_VALUE}")
        endif()
    endif()
endfunction()

# Function to try resolving as system dependency
function(try_system_dependency DEP_NAME DEP_VALUE TARGET_NAME)
    string(TOLOWER "${DEP_NAME}" DEP_NAME_LOWER)
    set(SYSTEM_LIB_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../system/libraries/${DEP_NAME_LOWER}.cmake")
    
    if(EXISTS "${SYSTEM_LIB_HANDLER_FILE}")
        # Include the system library handler
        include("${SYSTEM_LIB_HANDLER_FILE}")
        
        # Try to call the system library function
        # Wrap in try-catch equivalent using CMake's error handling
        set(SYSTEM_RESOLUTION_SUCCESS TRUE)
        
        # Parse dependency value similar to system_libraries.cmake
        if("${DEP_VALUE}" STREQUAL "true" OR "${DEP_VALUE}" STREQUAL "*")
            cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}")
        else()
            # Parse structured dependency specification from TOML parser
            # Format: "version=6;components=Core,Widgets;" (semicolon-separated key=value pairs)
            string(FIND "${DEP_VALUE}" ";" SEMICOLON_POS)
            if(SEMICOLON_POS EQUAL -1)
                # Simple version or single parameter (no semicolon means simple string)
                cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${DEP_VALUE}")
            else()
                # Parse structured key=value pairs separated by semicolons
                string(REPLACE ";" " " DEP_PARTS_SPACE "${DEP_VALUE}")
                string(REPLACE " " ";" DEP_PARTS "${DEP_PARTS_SPACE}")
                set(VERSION "")
                set(COMPONENTS "")
                set(OTHER_PARAMS "")
                
                foreach(PART ${DEP_PARTS})
                    # Skip empty parts
                    if(NOT "${PART}" STREQUAL "")
                        string(FIND "${PART}" "=" EQUALS_POS)
                        if(NOT EQUALS_POS EQUAL -1)
                            string(SUBSTRING "${PART}" 0 ${EQUALS_POS} KEY)
                            math(EXPR VALUE_START "${EQUALS_POS} + 1")
                            string(SUBSTRING "${PART}" ${VALUE_START} -1 VALUE)
                            
                            if("${KEY}" STREQUAL "version")
                                set(VERSION "${VALUE}")
                            elseif("${KEY}" STREQUAL "components")
                                # Convert comma-separated components to space-separated for CMake
                                string(REPLACE "," " " COMPONENTS "${VALUE}")
                            else()
                                # Store other parameters for future use
                                set(OTHER_PARAMS "${OTHER_PARAMS}${KEY}=${VALUE} ")
                            endif()
                        endif()
                    endif()
                endforeach()
                
                # Call function with parsed parameters
                if(VERSION AND COMPONENTS)
                    cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${VERSION}" "${COMPONENTS}")
                elseif(VERSION)
                    cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "${VERSION}")
                elseif(COMPONENTS)
                    cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}" "" "${COMPONENTS}")
                else()
                    cmake_language(CALL "add_${DEP_NAME_LOWER}_support" "${TARGET_NAME}")
                endif()
            endif()
        endif()
        
        # Success - set flag for parent scope
        set(SYSTEM_RESOLUTION_SUCCESS TRUE PARENT_SCOPE)
        return()
    endif()
    
    # If we get here, system resolution failed
    set(SYSTEM_RESOLUTION_SUCCESS FALSE PARENT_SCOPE)
endfunction()

# Main function to resolve all dependencies
function(resolve_all_dependencies TARGET_NAME)
    message(STATUS "=== Dependency Resolution for ${TARGET_NAME} ===")
    message(STATUS "SAIL_DEPENDENCY_NAMES: '${SAIL_DEPENDENCY_NAMES}'")
    message(STATUS "SAIL_DEPENDENCY_VALUES: '${SAIL_DEPENDENCY_VALUES}'")
    
    # Resolve regular dependencies
    if(SAIL_DEPENDENCY_NAMES AND SAIL_DEPENDENCY_VALUES)
        message(STATUS "Resolving regular dependencies for ${TARGET_NAME}")
        foreach(DEP_NAME DEP_VALUE IN ZIP_LISTS SAIL_DEPENDENCY_NAMES SAIL_DEPENDENCY_VALUES)
            resolve_dependency("${DEP_NAME}" "${DEP_VALUE}" "regular" "${TARGET_NAME}")
        endforeach()
    else()
        message(STATUS "No regular dependencies found for ${TARGET_NAME}")
    endif()
    
    # Resolve dev-dependencies (only for test targets)
    if(SAIL_DEV_DEPENDENCY_NAMES AND SAIL_DEV_DEPENDENCY_VALUES)
        string(FIND "${TARGET_NAME}" "_tests" TEST_POS)
        if(NOT TEST_POS EQUAL -1)
            message(STATUS "Resolving dev-dependencies for ${TARGET_NAME}")
            foreach(DEP_NAME DEP_VALUE IN ZIP_LISTS SAIL_DEV_DEPENDENCY_NAMES SAIL_DEV_DEPENDENCY_VALUES)
                resolve_dependency("${DEP_NAME}" "${DEP_VALUE}" "dev" "${TARGET_NAME}")
            endforeach()
        endif()
    endif()
endfunction()