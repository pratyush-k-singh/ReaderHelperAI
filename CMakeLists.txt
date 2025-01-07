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
find_package(Torch REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Catch2 REQUIRED)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${FAISS_INCLUDE_DIRS}
    ${TORCH_INCLUDE_DIRS}
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
)

# Create library
add_library(book_recommender_lib ${LIB_SOURCES})
target_link_libraries(book_recommender_lib
    PRIVATE
    ${FAISS_LIBRARIES}
    ${TORCH_LIBRARIES}
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    OpenMP::OpenMP_CXX
)

# Examples
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE book_recommender_lib)

add_executable(interactive_cli examples/interactive_cli.cpp)
target_link_libraries(interactive_cli PRIVATE book_recommender_lib)

# Tests
enable_testing()
add_subdirectory(tests)

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

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config-version.cmake"
    DESTINATION lib/cmake/book_recommender
)

# Add test subdirectory
add_subdirectory(tests)

# Test configuration
file(GLOB TEST_SOURCES tests/unit/*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests
    PRIVATE
    book_recommender_lib
    Catch2::Catch2
)

add_test(NAME unit_tests COMMAND unit_tests)

# Copy data files during build
file(COPY ${CMAKE_SOURCE_DIR}/data/raw/books.csv
     DESTINATION ${CMAKE_BINARY_DIR}/data/raw)

# Create config for find_package
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/book_recommender-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/book_recommender-config.cmake
    @ONLY
)