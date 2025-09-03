# Test Building Module
# Handles building the test executable

enable_testing()

# Include CPM for dev-dependencies
include(${CMAKE_CURRENT_LIST_DIR}/cpm.cmake)

# Add dev-dependencies from Sail.toml using modular system (only for tests)
if(SAIL_DEV_DEPENDENCY_NAMES AND SAIL_DEV_DEPENDENCY_VALUES)
    foreach(DEP_NAME DEP_VALUE IN ZIP_LISTS SAIL_DEV_DEPENDENCY_NAMES SAIL_DEV_DEPENDENCY_VALUES)
        # Convert dependency name to lowercase for file matching
        string(TOLOWER "${DEP_NAME}" DEP_NAME_LOWER)
        
        # Check if we have a specific handler for this dependency
        set(DEV_DEP_HANDLER_FILE "${CMAKE_CURRENT_LIST_DIR}/../dependencies/${DEP_NAME_LOWER}.cmake")
        if(EXISTS "${DEV_DEP_HANDLER_FILE}")
            # Include the dependency handler
            include("${DEV_DEP_HANDLER_FILE}")
            
            # Call the dependency function with dev dependency type
            cmake_language(CALL "add_${DEP_NAME_LOWER}_dependency" "${DEP_VALUE}" "dev")
        else()
            # Fallback for unknown dev-dependencies - try as CPM package directly
            message(WARNING "No specific handler found for dev-dependency '${DEP_NAME}', trying as direct CPM package")
            CPMAddPackage("${DEP_VALUE}")
        endif()
    endforeach()
endif()

# Collect test source files and exclude main.cpp from project sources for tests
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
file(GLOB_RECURSE TEST_PROJECT_SOURCES "src/*.cpp")
list(REMOVE_ITEM TEST_PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(${SAIL_PROJECT_NAME}_tests
    ${TEST_SOURCES}
    ${TEST_PROJECT_SOURCES}
)

target_include_directories(${SAIL_PROJECT_NAME}_tests PRIVATE include)

# Set output directory for test executable to tests subdirectory
set_target_properties(${SAIL_PROJECT_NAME}_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
)

# Link test dependencies (populated by dependency handlers)
if(SAIL_TEST_LINK_LIBRARIES)
    target_link_libraries(${SAIL_PROJECT_NAME}_tests ${SAIL_TEST_LINK_LIBRARIES})
endif()

# Update test command to use the new path
add_test(NAME ${SAIL_PROJECT_NAME}_tests COMMAND "${CMAKE_BINARY_DIR}/tests/${SAIL_PROJECT_NAME}_tests")