cmake_minimum_required(VERSION 3.15)
project(book_recommender VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find required packages
find_package(FAISS REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Catch2 REQUIRED)

# Optional SSL support for REST client
find_package(OpenSSL)
if(OPENSSL_FOUND)
    add_definitions(-DHAS_OPENSSL)
endif()

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${FAISS_INCLUDE_DIRS}
)

# Library source files
set(LIB_SOURCES
    src/core/Book.cpp
    src/core/Document.cpp
    src/core/BookRecommender.cpp
    src/data/BookDataLoader.cpp
    src/data/BookPreprocessor.cpp
    src/indexing/BookVectorStore.cpp
    src/query/BookQueryEngine.cpp
    src/utils/GroqClient.cpp
)

# Create static library
add_library(book_recommender_lib STATIC ${LIB_SOURCES})
target_link_libraries(book_recommender_lib
    PRIVATE
    ${FAISS_LIBRARIES}
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    cpprestsdk::cpprest
    OpenMP::OpenMP_CXX
)

if(OPENSSL_FOUND)
    target_link_libraries(book_recommender_lib
        PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
    )
endif()

# Examples
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE book_recommender_lib)

add_executable(interactive_cli examples/interactive_cli.cpp)
target_link_libraries(interactive_cli PRIVATE book_recommender_lib)

# Tests
enable_testing()

# Test configuration
file(GLOB TEST_SOURCES tests/unit/*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests
    PRIVATE
    book_recommender_lib
    Catch2::Catch2
)

add_test(NAME unit_tests COMMAND unit_tests)

# Installation
install(TARGETS book_recommender_lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Copy data and configuration files
file(COPY ${CMAKE_SOURCE_DIR}/data/raw/books.csv
     DESTINATION ${CMAKE_BINARY_DIR}/data/raw)

# Generate and install config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/book_recommender-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config.cmake
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config-version.cmake"
    DESTINATION lib/cmake/book_recommender
)

# Add dependencies examples folder
add_subdirectory(examples)

# Documentation (optional)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/include)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else()
    message(STATUS "Doxygen not found, documentation will not be built")
endif()

# Don't treat Doxygen or examples as required
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
else()
    message(STATUS "Examples directory not found, skipping examples build")
endif()
