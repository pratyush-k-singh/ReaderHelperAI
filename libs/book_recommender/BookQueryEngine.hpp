#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "../models/Book.hpp"
#include "BookVectorStore.hpp"

namespace book_recommender {

class BookQueryEngine {
public:
    struct RecommendationResult {
        Book book;
        float similarity_score;
        std::string explanation;
    };

    struct QueryFilter {
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

    BookQueryEngine(std::shared_ptr<BookVectorStore> vector_store);

    // Main recommendation methods
    std::vector<RecommendationResult> getRecommendations(
        const std::string& query,
        const QueryFilter& filter = {},
        int top_k = 5
    );

    std::vector<RecommendationResult> getSimilarBooks(
        const std::string& book_id,
        const QueryFilter& filter = {},
        int top_k = 5
    );

    std::vector<RecommendationResult> getAuthorRecommendations(
        const std::string& author,
        const QueryFilter& filter = {},
        int top_k = 5
    );

    std::vector<RecommendationResult> getSeriesRecommendations(
        const std::string& series,
        const QueryFilter& filter = {},
        int top_k = 5
    );

private:
    std::shared_ptr<BookVectorStore> vector_store_;

    // Query processing
    std::string enhanceQuery(const std::string& query) const;
    std::vector<float> vectorizeQuery(const std::string& query) const;
    bool passesFilter(const Book& book, const QueryFilter& filter) const;
    std::string generateExplanation(const Book& book, const std::string& query) const;
    
    // Sorting and ranking
    void rankResults(std::vector<RecommendationResult>& results) const;
    double calculateDiversityScore(const std::vector<RecommendationResult>& results) const;
    double calculateRelevanceScore(const Book& book, const std::string& query) const;
    
    // Helper methods
    std::vector<RecommendationResult> processSearchResults(
        const std::vector<BookVectorStore::SearchResult>& results,
        const std::string& query,
        const QueryFilter& filter
    ) const;
};

}