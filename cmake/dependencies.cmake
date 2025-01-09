# This file handles all external dependencies

# FAISS
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(FAISS REQUIRED)

# cpprestsdk
find_package(cpprestsdk REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    add_definitions(-DUSE_OPENMP)
endif()

# OpenSSL
find_package(OpenSSL)
if(OPENSSL_FOUND)
    add_definitions(-DHAS_OPENSSL)
endif()

# Catch2 for testing
if(BUILD_TESTING)
    find_package(Catch2 REQUIRED)
endif()

# Set variables for dependencies
set(EXTERNAL_INCLUDE_DIRS
    ${FAISS_INCLUDE_DIRS}
)

set(EXTERNAL_LIBRARIES
    ${FAISS_LIBRARIES}
    cpprestsdk::cpprest
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

if(OpenMP_CXX_FOUND)
    list(APPEND EXTERNAL_LIBRARIES OpenMP::OpenMP_CXX)
endif()

if(OPENSSL_FOUND)
    list(APPEND EXTERNAL_LIBRARIES 
        OpenSSL::SSL 
        OpenSSL::Crypto
    )
endif()

# Function to check all required dependencies
function(check_dependencies)
    set(MISSING_DEPS "")
    
    if(NOT FAISS_FOUND)
        list(APPEND MISSING_DEPS "FAISS")
    endif()
    
    if(NOT cpprestsdk_FOUND)
        list(APPEND MISSING_DEPS "cpprestsdk")
    endif()
    
    if(NOT spdlog_FOUND)
        list(APPEND MISSING_DEPS "spdlog")
    endif()
    
    if(NOT nlohmann_json_FOUND)
        list(APPEND MISSING_DEPS "nlohmann_json")
    endif()
    
    if(BUILD_TESTING AND NOT Catch2_FOUND)
        list(APPEND MISSING_DEPS "Catch2")
    endif()
    
    if(MISSING_DEPS)
        message(FATAL_ERROR "Missing required dependencies: ${MISSING_DEPS}")
    endif()
endfunction()

# Call check_dependencies
check_dependencies()
