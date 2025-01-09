add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE book_recommender_lib)

add_executable(interactive_cli interactive_cli.cpp)
target_link_libraries(interactive_cli PRIVATE book_recommender_lib)

file(COPY ${CMAKE_SOURCE_DIR}/data/raw/books.csv
     DESTINATION ${CMAKE_BINARY_DIR}/data/raw)