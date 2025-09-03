# Qt Framework Support Module
# Finds and links Qt libraries based on specified components

function(add_qt_support TARGET_NAME QT_VERSION QT_COMPONENTS)
    # Set Qt version (default to Qt6 if not specified)
    if(NOT QT_VERSION)
        set(QT_VERSION "6")
    endif()
    
    # Parse components (space-separated string to list)
    string(REPLACE " " ";" COMPONENT_LIST "${QT_COMPONENTS}")
    
    message(STATUS "Finding Qt${QT_VERSION} with components: ${COMPONENT_LIST}")
    
    # Find Qt
    find_package(Qt${QT_VERSION} COMPONENTS ${COMPONENT_LIST})
    
    if(Qt${QT_VERSION}_FOUND)
        message(STATUS "Qt${QT_VERSION} found for ${TARGET_NAME}")
        
        # Link Qt libraries
        foreach(COMPONENT ${COMPONENT_LIST})
            target_link_libraries(${TARGET_NAME} Qt${QT_VERSION}::${COMPONENT})
        endforeach()
        
        # Enable Qt features
        set_target_properties(${TARGET_NAME} PROPERTIES
            AUTOMOC ON
            AUTORCC ON
            AUTOUIC ON
        )
        
        # Add Qt definitions
        target_compile_definitions(${TARGET_NAME} PRIVATE QT_VERSION_MAJOR=${QT_VERSION})
        
    else()
        message(FATAL_ERROR "Qt${QT_VERSION} not found for ${TARGET_NAME}")
    endif()
endfunction()