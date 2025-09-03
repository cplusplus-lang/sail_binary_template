# fmt Dependency Handler
# Handles fmt dependency addition and configuration

function(add_fmt_dependency VERSION DEPENDENCY_TYPE)
    # Add fmt via CPM
    message(STATUS "Adding fmt dependency: version ${VERSION} (${DEPENDENCY_TYPE})")
    CPMAddPackage("gh:fmtlib/fmt#${VERSION}")
    
    # Link based on dependency type
    if(DEPENDENCY_TYPE STREQUAL "regular")
        # Regular dependency: link to both binary and tests
        
        # Add to binary target if it exists
        if(TARGET ${SAIL_PROJECT_NAME})
            target_link_libraries(${SAIL_PROJECT_NAME} PRIVATE fmt::fmt)
        endif()
        
        # Also add to test libraries
        if(NOT DEFINED SAIL_TEST_LINK_LIBRARIES)
            set(SAIL_TEST_LINK_LIBRARIES "" PARENT_SCOPE)
        endif()
        list(APPEND SAIL_TEST_LINK_LIBRARIES "fmt::fmt")
        set(SAIL_TEST_LINK_LIBRARIES "${SAIL_TEST_LINK_LIBRARIES}" PARENT_SCOPE)
        
    elseif(DEPENDENCY_TYPE STREQUAL "dev")
        # Dev dependency: only link to tests
        if(NOT DEFINED SAIL_TEST_LINK_LIBRARIES)
            set(SAIL_TEST_LINK_LIBRARIES "" PARENT_SCOPE)
        endif()
        list(APPEND SAIL_TEST_LINK_LIBRARIES "fmt::fmt")
        set(SAIL_TEST_LINK_LIBRARIES "${SAIL_TEST_LINK_LIBRARIES}" PARENT_SCOPE)
        
    else()
        message(FATAL_ERROR "Unknown dependency type: ${DEPENDENCY_TYPE}. Must be 'regular' or 'dev'")
    endif()
endfunction()