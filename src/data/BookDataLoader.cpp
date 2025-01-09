#include "book_recommender/BookDataLoader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <spdlog/spdlog.h>

namespace book_recommender {

BookDataLoader::BookDataLoader(const std::string& data_file)
    : preprocessor_(std::make_unique<BookPreprocessor>()) {
    auto& config = Config::getInstance();
    data_path_ = config.getRawDataDir() / data_file;
}

std::pair<std::vector<Book>, std::vector<Document>> BookDataLoader::loadAndPreprocess() {
    validateDataFile();
    
    std::vector<Book> books;
    std::vector<Document> documents;
    
    auto rows = readCsvFile();
    spdlog::info("Read {} rows from CSV file", rows.size());
    
    // Skip header row
    for (size_t i = 1; i < rows.size(); ++i) {
        try {
            auto book = parseBookRow(rows[i]);
            
            if (passesFilters(book)) {
                books.push_back(book);
                documents.push_back(preprocessor_->createDocument(book));
            }
        } catch (const std::exception& e) {
            spdlog::warn("Failed to parse row {}: {}", i, e.what());
            continue;
        }
    }
    
    spdlog::info("Successfully loaded {} books after filtering", books.size());
    return {books, documents};
}

void BookDataLoader::validateDataFile() const {
    if (!std::filesystem::exists(data_path_)) {
        throw std::runtime_error("Data file not found: " + data_path_.string());
    }
}

std::vector<std::vector<std::string>> BookDataLoader::readCsvFile() const {
    std::vector<std::vector<std::string>> rows;
    std::ifstream file(data_path_);
    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;

        bool in_quotes = false;
        std::string current_field;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                row.push_back(cleanString(current_field));
                current_field.clear();
            } else {
                current_field += c;
            }
        }
        row.push_back(cleanString(current_field));
        rows.push_back(row);
    }

    return rows;
}

Book BookDataLoader::parseBookRow(const std::vector<std::string>& row) const {
    if (row.size() < 12) {
        throw std::runtime_error("Invalid row format: insufficient columns");
    }

    return Book(
        row[0],                    // id
        cleanString(row[1]),       // title
        cleanString(row[2]),       // author
        parseGenres(row[3]),       // genres
        cleanString(row[4]),       // description
        parseInteger(row[5]),      // page_count
        parseRating(row[6]),       // average_rating
        parseInteger(row[7]),      // ratings_count
        parseInteger(row[8]),      // review_count
        row[9].empty() ? std::nullopt : 
            std::optional<std::string>(cleanString(row[9])), // series
        cleanString(row[10]),      // language
        cleanString(row[11]),      // publisher
        row[12],                   // publication_date
        parseIsbn13(row[13]),      // isbn13
        row[14] == "true"          // is_ebook
    );
}

bool BookDataLoader::passesFilters(const Book& book) const {
    return book.getRatingsCount() >= min_ratings_ &&
           book.getLanguage() == language_filter_ &&
           book.getPublicationYear() >= min_year_ &&
           book.getPublicationYear() <= max_year_;
}

std::string BookDataLoader::cleanString(const std::string& str) const {
    std::string cleaned = str;
    
    // Remove surrounding quotes
    if (cleaned.size() >= 2 && cleaned.front() == '"' && cleaned.back() == '"') {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }
    
    // Trim whitespace
    cleaned.erase(0, cleaned.find_first_not_of(" \t\n\r"));
    cleaned.erase(cleaned.find_last_not_of(" \t\n\r") + 1);
    
    return cleaned;
}

std::vector<std::string> BookDataLoader::parseGenres(const std::string& genres_str) const {
    std::vector<std::string> genres;
    std::string cleaned = cleanString(genres_str);
    
    if (cleaned.empty() || cleaned == "[]") {
        return genres;
    }
    
    // Remove brackets
    cleaned = cleaned.substr(1, cleaned.size() - 2);
    
    std::stringstream ss(cleaned);
    std::string genre;
    while (std::getline(ss, genre, ',')) {
        genres.push_back(cleanString(genre));
    }
    
    return genres;
}

int BookDataLoader::parseYear(const std::string& date_str) const {
    std::regex year_regex("\\d{4}");
    std::smatch match;
    if (std::regex_search(date_str, match, year_regex)) {
        return std::stoi(match[0]);
    }
    return 0;
}

double BookDataLoader::parseRating(const std::string& rating_str) const {
    try {
        return rating_str.empty() ? 0.0 : std::stod(rating_str);
    } catch (const std::exception&) {
        return 0.0;
    }
}

int BookDataLoader::parseInteger(const std::string& int_str) const {
    try {
        return int_str.empty() ? 0 : std::stoi(int_str);
    } catch (const std::exception&) {
        return 0;
    }
}

std::string BookDataLoader::parseIsbn13(const std::string& isbn_str) const {
    std::string cleaned = cleanString(isbn_str);
    std::regex isbn_regex("\\d{13}");
    std::smatch match;
    if (std::regex_search(cleaned, match, isbn_regex)) {
        return match[0];
    }
    return "";
}

}