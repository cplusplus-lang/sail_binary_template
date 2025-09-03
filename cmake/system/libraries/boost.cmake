# Boost Libraries Support Module
# Finds and links Boost libraries based on specified version and components

function(add_boost_support TARGET_NAME BOOST_VERSION BOOST_COMPONENTS)
    # Set Boost version (if not specified, use any available)
    if(BOOST_VERSION)
        set(Boost_FIND_VERSION "${BOOST_VERSION}")
        message(STATUS "Finding Boost ${BOOST_VERSION} for ${TARGET_NAME}")
    else()
        message(STATUS "Finding Boost (any version) for ${TARGET_NAME}")
    endif()
    
    # Parse components if specified
    if(BOOST_COMPONENTS)
        string(REPLACE " " ";" COMPONENT_LIST "${BOOST_COMPONENTS}")
        find_package(Boost ${Boost_FIND_VERSION} COMPONENTS ${COMPONENT_LIST})
    else()
        find_package(Boost ${Boost_FIND_VERSION})
    endif()
    
    if(Boost_FOUND)
        message(STATUS "Boost ${Boost_VERSION} found for ${TARGET_NAME}")
        
        # Include Boost headers
        target_include_directories(${TARGET_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
        
        # Link Boost libraries
        if(BOOST_COMPONENTS)
            target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
        endif()
        
        # Add Boost definitions
        target_compile_definitions(${TARGET_NAME} PRIVATE BOOST_VERSION=${Boost_VERSION})
        
    else()
        if(BOOST_VERSION)
            message(FATAL_ERROR "Boost ${BOOST_VERSION} not found for ${TARGET_NAME}")
        else()
            message(FATAL_ERROR "Boost not found for ${TARGET_NAME}")
        endif()
    endif()
endfunction()