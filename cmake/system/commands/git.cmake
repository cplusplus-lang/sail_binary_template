# Git System Dependency Handler
# Handles git system command dependency verification

function(add_git_dependency VERSION)
    message(STATUS "Checking git system dependency: version requirement ${VERSION}")
    
    # Find git executable
    find_program(GIT_EXECUTABLE git)
    
    if(NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "Git not found. Please install git to use this project.")
    endif()
    
    # Get git version if specific version is required (not "*")
    if(NOT VERSION STREQUAL "*")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} --version
            OUTPUT_VARIABLE GIT_VERSION_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_VERSION_RESULT
        )
        
        if(GIT_VERSION_RESULT EQUAL 0)
            # Extract version from output like "git version 2.39.0"
            string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" GIT_VERSION "${GIT_VERSION_OUTPUT}")
            message(STATUS "Found git version: ${GIT_VERSION}")
            
            # Version comparison could be added here if needed
            # For now, just report the found version
        else()
            message(WARNING "Could not determine git version")
        endif()
    else()
        message(STATUS "Git found: ${GIT_EXECUTABLE}")
    endif()
    
    # Make git available to the project
    set(GIT_FOUND TRUE PARENT_SCOPE)
    set(GIT_EXECUTABLE "${GIT_EXECUTABLE}" PARENT_SCOPE)
endfunction()