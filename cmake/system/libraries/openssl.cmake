# OpenSSL Support Module
# Finds and links OpenSSL libraries

function(add_openssl_support TARGET_NAME OPENSSL_VERSION)
    if(OPENSSL_VERSION AND NOT "${OPENSSL_VERSION}" STREQUAL "*")
        message(STATUS "Finding OpenSSL ${OPENSSL_VERSION} for ${TARGET_NAME}")
        find_package(OpenSSL ${OPENSSL_VERSION} REQUIRED)
    else()
        message(STATUS "Finding OpenSSL (any version) for ${TARGET_NAME}")
        find_package(OpenSSL REQUIRED)
    endif()
    
    if(OpenSSL_FOUND)
        message(STATUS "OpenSSL ${OPENSSL_VERSION} found for ${TARGET_NAME}")
        
        # Link OpenSSL libraries
        target_link_libraries(${TARGET_NAME} OpenSSL::SSL OpenSSL::Crypto)
        
        # Add OpenSSL definitions
        target_compile_definitions(${TARGET_NAME} PRIVATE OPENSSL_VERSION_TEXT="${OPENSSL_VERSION}")
        
    else()
        message(FATAL_ERROR "OpenSSL not found for ${TARGET_NAME}")
    endif()
endfunction()