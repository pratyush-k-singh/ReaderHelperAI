#include "book_recommender/BookPreprocessor.hpp"
#include <algorithm>
#include <regex>
#include <set>
#include <sstream>
#include <cctype>
#include <fstream>
#include <Levenshtein.hpp>
#include <spdlog/spdlog.h>

namespace book_recommender {

BookPreprocessor::BookPreprocessor() {
    initializeGenreMappings();
    loadCustomGenreMappings();
}

void BookPreprocessor::initializeGenreMappings() {
    standard_genres_ = {
        "fiction",
        "non-fiction",
        "mystery",
        "thriller",
        "romance",
        "science-fiction",
        "fantasy",
        "horror",
        "historical-fiction",
        "literary-fiction",
        "young-adult",
        "children",
        "biography",
        "history",
        "science",
        "technology",
        "business",
        "self-help",
        "poetry",
        "drama",
        "comedy",
        "adventure",
        "crime",
        "contemporary",
        "classics"
    };

    // Initialize basic mappings
    genre_mapping_ = {
        {"sci-fi", "science-fiction"},
        {"sf", "science-fiction"},
        {"scifi", "science-fiction"},
        {"ya", "young-adult"},
        {"biographical", "biography"},
        {"biographies", "biography"},
        {"historic", "history"},
        {"historical", "history"},
        {"tech", "technology"},
        {"computers", "technology"},
        {"programming", "technology"},
        {"romance", "romance"},
        {"romantic", "romance"},
        {"love", "romance"},
        {"mystery", "mystery"},
        {"mysteries", "mystery"},
        {"detective", "mystery"}
    };
}

void BookPreprocessor::loadCustomGenreMappings() {
    try {
        std::ifstream file("config/genre_mappings.txt");
        std::string line;
        
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string raw, mapped;
            
            if (iss >> raw >> mapped) {
                if (std::find(standard_genres_.begin(), standard_genres_.end(), mapped) 
                    != standard_genres_.end()) {
                    genre_mapping_[toLowerCase(raw)] = mapped;
                }
            }
        }
    } catch (const std::exception& e) {
        spdlog::warn("Could not load custom genre mappings: {}", e.what());
    }
}

void BookPreprocessor::updateGenreMapping(const std::string& raw_genre, const std::string& normalized_genre) {
    if (std::find(standard_genres_.begin(), standard_genres_.end(), normalized_genre) 
        != standard_genres_.end()) {
        genre_mapping_[raw_genre] = normalized_genre;
        
        // Save to custom mappings file
        try {
            std::ofstream file("config/genre_mappings.txt", std::ios::app);
            file << raw_genre << " " << normalized_genre << "\n";
        } catch (const std::exception& e) {
            spdlog::warn("Could not save genre mapping: {}", e.what());
        }
    }
}

std::string BookPreprocessor::findClosestGenre(const std::string& raw_genre) {
    std::string closest = standard_genres_[0];
    double min_distance = std::numeric_limits<double>::max();
    
    for (const auto& standard : standard_genres_) {
        double distance = calculateGenreSimilarity(raw_genre, standard);
        
        if (distance < min_distance) {
            min_distance = distance;
            closest = standard;
        }
    }
    
    return closest;
}

double BookPreprocessor::calculateGenreSimilarity(const std::string& genre1, const std::string& genre2) {
    // Normalize strings
    std::string g1 = toLowerCase(removePunctuation(genre1));
    std::string g2 = toLowerCase(removePunctuation(genre2));
    
    // Calculate Levenshtein distance
    int distance = Levenshtein::distance(g1, g2);
    
    // Normalize by length of longer string
    int max_length = std::max(g1.length(), g2.length());
    if (max_length == 0) return 1.0;
    
    return static_cast<double>(distance) / max_length;
}

}