#pragma once

#include <string>
#include <vector>
#include <map>
#include "../models/Book.hpp"
#include "../models/Document.hpp"

namespace book_recommender {

class BookPreprocessor {
public:
    BookPreprocessor();

    // Main preprocessing function
    Document createDocument(const Book& book);

    // Text preprocessing
    std::string preprocessText(const std::string& text);
    std::string combineBookText(const Book& book);

    // Genre handling
    std::vector<std::string> normalizeGenres(const std::vector<std::string>& genres);
    void updateGenreMapping(const std::string& raw_genre, const std::string& normalized_genre);

    // Metadata handling
    Document::Metadata createMetadata(const Book& book);

private:
    // Genre normalization maps
    std::map<std::string, std::string> genre_mapping_;
    std::vector<std::string> standard_genres_;

    // Text preprocessing helpers
    std::string removePunctuation(const std::string& text);
    std::string toLowerCase(const std::string& text);
    std::string removeStopWords(const std::string& text);
    std::string stemWords(const std::string& text);
    
    // Genre helpers
    std::string findClosestGenre(const std::string& raw_genre);
    double calculateGenreSimilarity(const std::string& genre1, const std::string& genre2);
    
    // Initialize standard genres and mappings
    void initializeGenreMappings();
    void loadCustomGenreMappings();
};

}