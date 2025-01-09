#include "book_recommender/BookRecommender.hpp"
#include "book_recommender/Document.hpp"
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <spdlog/spdlog.h>

namespace book_recommender {

BookRecommender::BookRecommender(const RecommenderConfig& config)
    : config_(config) {
    validateConfig();
    initialize();
}

BookRecommender::~BookRecommender() = default;

void BookRecommender::initialize() {
    try {
        data_loader_ = std::make_unique<BookDataLoader>(config_.data_file);
        data_loader_->setMinRatings(config_.min_ratings);
        data_loader_->setLanguageFilter(config_.language_filter);

        vector_store_ = std::make_shared<BookVectorStore>(
            config_.embedding_dimension,
            config_.cache_size
        );

        query_engine_ = std::make_unique<BookQueryEngine>(vector_store_);

        if (config_.load_existing_index && tryLoadExistingIndex()) {
            spdlog::info("Successfully loaded existing index");
        } else {
            createNewIndex();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize BookRecommender: {}", e.what());
        throw;
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookRecommender::getRecommendations(
    const std::string& query,
    const BookQueryEngine::QueryFilter& filter,
    int top_k
) {
    try {
        return query_engine_->getRecommendations(query, filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting recommendations: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookRecommender::getSimilarBooks(
    const std::string& book_id,
    const BookQueryEngine::QueryFilter& filter,
    int top_k
) {
    try {
        return query_engine_->getSimilarBooks(book_id, filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting similar books: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookRecommender::getAuthorRecommendations(
    const std::string& author,
    const BookQueryEngine::QueryFilter& filter,
    int top_k
) {
    try {
        return query_engine_->getAuthorRecommendations(author, filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting author recommendations: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookRecommender::getSeriesRecommendations(
    const std::string& series,
    const BookQueryEngine::QueryFilter& filter,
    int top_k
) {
    try {
        return query_engine_->getSeriesRecommendations(series, filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting series recommendations: {}", e.what());
        return {};
    }
}

std::vector<Book> BookRecommender::searchBooks(
    const std::string& query,
    const BookQueryEngine::QueryFilter& filter
) {
    try {
        auto results = query_engine_->getRecommendations(query, filter, 100);
        std::vector<Book> books;
        books.reserve(results.size());
        std::transform(results.begin(), results.end(), std::back_inserter(books),
                      [](const auto& result) { return result.book; });
        return books;
    } catch (const std::exception& e) {
        spdlog::error("Error searching books: {}", e.what());
        return {};
    }
}

std::vector<std::string> BookRecommender::getPopularGenres(int top_k) const {
    std::unordered_map<std::string, int> genre_counts;
    
    for (const auto& book : books_) {
        for (const auto& genre : book.getGenres()) {
            genre_counts[genre]++;
        }
    }

    std::vector<std::pair<std::string, int>> genre_pairs(
        genre_counts.begin(), genre_counts.end()
    );
    
    std::partial_sort(
        genre_pairs.begin(),
        genre_pairs.begin() + std::min(top_k, static_cast<int>(genre_pairs.size())),
        genre_pairs.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; }
    );

    std::vector<std::string> popular_genres;
    popular_genres.reserve(top_k);
    for (int i = 0; i < top_k && i < static_cast<int>(genre_pairs.size()); ++i) {
        popular_genres.push_back(genre_pairs[i].first);
    }
    
    return popular_genres;
}

std::vector<std::string> BookRecommender::getPopularAuthors(int top_k) const {
    std::unordered_map<std::string, int> author_counts;
    
    for (const auto& book : books_) {
        author_counts[book.getAuthor()]++;
    }

    std::vector<std::pair<std::string, int>> author_pairs(
        author_counts.begin(), author_counts.end()
    );
    
    std::partial_sort(
        author_pairs.begin(),
        author_pairs.begin() + std::min(top_k, static_cast<int>(author_pairs.size())),
        author_pairs.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; }
    );

    std::vector<std::string> popular_authors;
    popular_authors.reserve(top_k);
    for (int i = 0; i < top_k && i < static_cast<int>(author_pairs.size()); ++i) {
        popular_authors.push_back(author_pairs[i].first);
    }
    
    return popular_authors;
}

std::vector<Book> BookRecommender::getTopRatedBooks(int limit) const {
    std::vector<Book> top_books = books_;
    
    std::partial_sort(
        top_books.begin(),
        top_books.begin() + std::min(limit, static_cast<int>(top_books.size())),
        top_books.end(),
        [](const Book& a, const Book& b) {
            if (a.getAverageRating() == b.getAverageRating()) {
                return a.getRatingsCount() > b.getRatingsCount();
            }
            return a.getAverageRating() > b.getAverageRating();
        }
    );

    top_books.resize(std::min(limit, static_cast<int>(top_books.size())));
    return top_books;
}

void BookRecommender::saveIndex(const std::string& path) {
    vector_store_->saveIndex(path);
}

void BookRecommender::loadIndex(const std::string& path) {
    vector_store_->loadIndex(path);
}

void BookRecommender::rebuildIndex() {
    createNewIndex();
}

void BookRecommender::updateBook(const Book& book) {
    // Find and update book in books_ vector
    auto it = std::find_if(books_.begin(), books_.end(),
                          [&](const Book& b) { return b.getId() == book.getId(); });
    
    if (it != books_.end()) {
        *it = book;
    } else {
        books_.push_back(book);
    }

    // Update vector store
    std::vector<Document> doc = data_loader_->getPreprocessor().createDocument(book);
    vector_store_->addDocuments({doc});
}

void BookRecommender::removeBook(const std::string& book_id) {
    books_.erase(
        std::remove_if(
            books_.begin(),
            books_.end(),
            [&](const Book& b) { return b.getId() == book_id; }
        ),
        books_.end()
    );
    
    vector_store_->removeDocument(book_id);
}

void BookRecommender::validateConfig() const {
    if (config_.embedding_dimension <= 0) {
        throw std::invalid_argument("Invalid embedding dimension");
    }
    if (config_.cache_size <= 0) {
        throw std::invalid_argument("Invalid cache size");
    }
    if (config_.min_ratings < 0) {
        throw std::invalid_argument("Invalid minimum ratings");
    }
}

std::string BookRecommender::getDefaultIndexPath() const {
    return (std::filesystem::current_path() / "data" / "index" / "book_index").string();
}

void BookRecommender::processBooks(const std::vector<Book>& books) {
    std::vector<Document> documents;
    documents.reserve(books.size());

    for (const auto& book : books) {
        documents.push_back(data_loader_->getPreprocessor().createDocument(book));
    }

    vector_store_->batchAddDocuments(documents);
    updatePopularityMetrics();
}

void BookRecommender::updatePopularityMetrics() {
    // Update any cached popularity metrics or scores
    vector_store_->optimizeIndex();
}

}