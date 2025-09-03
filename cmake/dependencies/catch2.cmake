# Catch2 Dependency Handler
# Handles Catch2 dependency addition and configuration

function(add_catch2_dependency VERSION DEPENDENCY_TYPE)
    # Add Catch2 via CPM
    message(STATUS "Adding Catch2 dependency: version ${VERSION} (${DEPENDENCY_TYPE})")
    CPMAddPackage("gh:catchorg/Catch2@${VERSION}")
    
    # Link based on dependency type
    if(DEPENDENCY_TYPE STREQUAL "regular")
        # Regular dependency: link to both binary and tests
        # For Catch2, this is unusual but we'll support it
        message(WARNING "Catch2 as regular dependency is unusual - typically should be dev-dependency")
        
        # Add to both binary and test link libraries
        if(TARGET ${SAIL_PROJECT_NAME})
            target_link_libraries(${SAIL_PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)
        endif()
        
        # Also add to test libraries
        if(NOT DEFINED SAIL_TEST_LINK_LIBRARIES)
            set(SAIL_TEST_LINK_LIBRARIES "" PARENT_SCOPE)
        endif()
        list(APPEND SAIL_TEST_LINK_LIBRARIES "Catch2::Catch2WithMain")
        set(SAIL_TEST_LINK_LIBRARIES "${SAIL_TEST_LINK_LIBRARIES}" PARENT_SCOPE)
        
    elseif(DEPENDENCY_TYPE STREQUAL "dev")
        # Dev dependency: only link to tests
        if(NOT DEFINED SAIL_TEST_LINK_LIBRARIES)
            set(SAIL_TEST_LINK_LIBRARIES "" PARENT_SCOPE)
        endif()
        list(APPEND SAIL_TEST_LINK_LIBRARIES "Catch2::Catch2WithMain")
        set(SAIL_TEST_LINK_LIBRARIES "${SAIL_TEST_LINK_LIBRARIES}" PARENT_SCOPE)
        
    else()
        message(FATAL_ERROR "Unknown dependency type: ${DEPENDENCY_TYPE}. Must be 'regular' or 'dev'")
    endif()
endfunction()