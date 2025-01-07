#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace book_recommender {

// Common type definitions
using Embedding = std::vector<float>;
using TimePoint = std::chrono::system_clock::time_point;

// Constants
constexpr int DEFAULT_EMBEDDING_DIMENSION = 384;
constexpr int DEFAULT_CACHE_SIZE = 1000;
constexpr int DEFAULT_MIN_RATINGS = 100;
constexpr double MIN_SIMILARITY_SCORE = 0.5;
constexpr int DEFAULT_TOP_K = 5;

// Common structs
struct BookMetadata {
    std::string title;
    std::string author;
    std::vector<std::string> genres;
    std::string description;
    int page_count;
    double average_rating;
    int ratings_count;
    int review_count;
    std::optional<std::string> series;
    std::string language;
    std::string publisher;
    std::string publication_date;
    std::string isbn13;
    bool is_ebook;
};

struct SearchResult {
    std::string id;
    float similarity;
    std::string title;
    std::string author;
    std::vector<std::string> genres;
    double rating;
    std::string explanation;
};

struct SearchFilter {
    std::optional<std::vector<std::string>> genres;
    std::optional<double> min_rating;
    std::optional<double> max_rating;
    std::optional<int> min_ratings_count;
    std::optional<int> publication_year_start;
    std::optional<int> publication_year_end;
    std::optional<std::string> language;
    std::optional<bool> ebook_only;
    std::optional<std::vector<std::string>> authors;
};

// Error types
class BookRecommenderError : public std::runtime_error {
public:
    explicit BookRecommenderError(const std::string& message) 
        : std::runtime_error(message) {}
};

class DataLoadError : public BookRecommenderError {
public:
    explicit DataLoadError(const std::string& message) 
        : BookRecommenderError(message) {}
};

class IndexError : public BookRecommenderError {
public:
    explicit IndexError(const std::string& message) 
        : BookRecommenderError(message) {}
};

class QueryError : public BookRecommenderError {
public:
    explicit QueryError(const std::string& message) 
        : BookRecommenderError(message) {}
};

}