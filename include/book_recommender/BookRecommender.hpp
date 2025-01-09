#pragma once

#include <string>
#include <memory>
#include "BookDataLoader.hpp"
#include "BookQueryEngine.hpp"
#include "BookVectorStore.hpp"

namespace book_recommender {

class BookRecommender {
public:
    struct RecommenderConfig {
        std::string data_file = "books.csv";
        int embedding_dimension = 384;
        int cache_size = 1000;
        std::string language_filter = "en";
        int min_ratings = 100;
        bool load_existing_index = true;
    };

    explicit BookRecommender(const RecommenderConfig& config = RecommenderConfig{});
    ~BookRecommender();

    // Core recommendation methods
    std::vector<BookQueryEngine::RecommendationResult> getRecommendations(
        const std::string& query,
        const BookQueryEngine::QueryFilter& filter = {},
        int top_k = 5
    );

    std::vector<BookQueryEngine::RecommendationResult> getSimilarBooks(
        const std::string& book_id,
        const BookQueryEngine::QueryFilter& filter = {},
        int top_k = 5
    );

    // Specialized recommendation methods
    std::vector<BookQueryEngine::RecommendationResult> getAuthorRecommendations(
        const std::string& author,
        const BookQueryEngine::QueryFilter& filter = {},
        int top_k = 5
    );

    std::vector<BookQueryEngine::RecommendationResult> getSeriesRecommendations(
        const std::string& series,
        const BookQueryEngine::QueryFilter& filter = {},
        int top_k = 5
    );

    // Book search and filtering
    std::vector<Book> searchBooks(
        const std::string& query,
        const BookQueryEngine::QueryFilter& filter = {}
    );

    std::vector<std::string> getPopularGenres(int top_k = 10) const;
    std::vector<std::string> getPopularAuthors(int top_k = 10) const;
    std::vector<Book> getTopRatedBooks(int limit = 10) const;

    // Index management
    void saveIndex(const std::string& path);
    void loadIndex(const std::string& path);
    void rebuildIndex();
    void updateBook(const Book& book);
    void removeBook(const std::string& book_id);

private:
    RecommenderConfig config_;
    std::unique_ptr<BookDataLoader> data_loader_;
    std::shared_ptr<BookVectorStore> vector_store_;
    std::unique_ptr<BookQueryEngine> query_engine_;
    std::vector<Book> books_;

    // Initialization
    void initialize();
    void loadData();
    bool tryLoadExistingIndex();
    void createNewIndex();

    // Helper methods
    void validateConfig() const;
    std::string getDefaultIndexPath() const;
    void processBooks(const std::vector<Book>& books);
    void updatePopularityMetrics();
};

}