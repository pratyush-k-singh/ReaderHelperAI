#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "BookPreprocessor.hpp"
#include "Document.hpp"
#include "Book.hpp"

namespace book_recommender {

class BookDataLoader {
public:
    BookDataLoader(const std::string& data_file = "books.csv");

    // Load and preprocess data
    std::pair<std::vector<Book>, std::vector<Document>> loadAndPreprocess();

    // Configuration
    void setMinRatings(int min_ratings) { min_ratings_ = min_ratings; }
    void setLanguageFilter(const std::string& lang) { language_filter_ = lang; }
    void setYearRange(int min_year, int max_year) {
        min_year_ = min_year;
        max_year_ = max_year;
    }

private:
    std::filesystem::path data_path_;
    std::unique_ptr<BookPreprocessor> preprocessor_;
    
    // Filtering criteria
    int min_ratings_ = 100;
    std::string language_filter_ = "en";
    int min_year_ = 1900;
    int max_year_ = 2025;

    // Helper methods
    void validateDataFile() const;
    std::vector<std::vector<std::string>> readCsvFile() const;
    Book parseBookRow(const std::vector<std::string>& row) const;
    bool passesFilters(const Book& book) const;
    
    // CSV parsing helpers
    std::string cleanString(const std::string& str) const;
    std::vector<std::string> parseGenres(const std::string& genres_str) const;
    int parseYear(const std::string& date_str) const;
    double parseRating(const std::string& rating_str) const;
    int parseInteger(const std::string& int_str) const;
    std::string parseIsbn13(const std::string& isbn_str) const;
};

}