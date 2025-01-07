#include "BookQueryEngine.hpp"
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <regex>
#include <transformers/TextEmbedding.hpp>
#include <spdlog/spdlog.h>

namespace book_recommender {

// Initialize static embedding model
static std::unique_ptr<TextEmbedding> text_embedder = 
    std::make_unique<TextEmbedding>("BAAI/bge-small-en-v1.5");

BookQueryEngine::BookQueryEngine(std::shared_ptr<BookVectorStore> vector_store)
    : vector_store_(std::move(vector_store)) {}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::getRecommendations(
    const std::string& query,
    const QueryFilter& filter,
    int top_k
) {
    try {
        std::string enhanced_query = enhanceQuery(query);
        auto query_vector = vectorizeQuery(enhanced_query);
        
        auto search_results = vector_store_->search(query_vector, top_k * 2);
        auto recommendations = processSearchResults(search_results, query, filter);
        
        rankResults(recommendations);
        if (recommendations.size() > static_cast<size_t>(top_k)) {
            recommendations.resize(top_k);
        }
        
        return recommendations;
    } catch (const std::exception& e) {
        spdlog::error("Error getting recommendations: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::getSimilarBooks(
    const std::string& book_id,
    const QueryFilter& filter,
    int top_k
) {
    try {
        auto search_results = vector_store_->searchSimilar(book_id, top_k * 2);
        auto recommendations = processSearchResults(search_results, "", filter);
        
        // Remove the query book itself
        recommendations.erase(
            std::remove_if(
                recommendations.begin(),
                recommendations.end(),
                [&](const auto& rec) { return rec.book.getId() == book_id; }
            ),
            recommendations.end()
        );
        
        rankResults(recommendations);
        if (recommendations.size() > static_cast<size_t>(top_k)) {
            recommendations.resize(top_k);
        }
        
        return recommendations;
    } catch (const std::exception& e) {
        spdlog::error("Error getting similar books: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::getAuthorRecommendations(
    const std::string& author,
    const QueryFilter& filter,
    int top_k
) {
    try {
        // Create author-specific filter
        QueryFilter author_filter = filter;
        if (!author_filter.authors) {
            author_filter.authors = std::vector<std::string>();
        }
        author_filter.authors->push_back(author);
        
        // Get recommendations with enhanced query
        std::string query = "books by author " + author;
        return getRecommendations(query, author_filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting author recommendations: {}", e.what());
        return {};
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::getSeriesRecommendations(
    const std::string& series,
    const QueryFilter& filter,
    int top_k
) {
    try {
        std::string query = "books in series " + series;
        return getRecommendations(query, filter, top_k);
    } catch (const std::exception& e) {
        spdlog::error("Error getting series recommendations: {}", e.what());
        return {};
    }
}

std::string BookQueryEngine::enhanceQuery(const std::string& query) const {
    std::ostringstream enhanced;
    enhanced << query;

    // Add context based on query content
    if (query.find("like") != std::string::npos) {
        enhanced << " similar books recommendations";
    }
    
    if (query.find("author") != std::string::npos) {
        enhanced << " books written by";
    }
    
    if (query.find("series") != std::string::npos) {
        enhanced << " book series sequel prequel";
    }

    // Add genre-specific enhancements
    std::vector<std::pair<std::string, std::string>> genre_keywords = {
        {"fantasy", "magic dragons adventure quest"},
        {"science fiction", "space technology future sci-fi"},
        {"mystery", "detective crime investigation thriller"},
        {"romance", "love relationship emotional"},
        {"horror", "scary supernatural terror dark"}
    };

    for (const auto& [genre, keywords] : genre_keywords) {
        if (query.find(genre) != std::string::npos) {
            enhanced << " " << keywords;
        }
    }

    return enhanced.str();
}

std::vector<float> BookQueryEngine::vectorizeQuery(const std::string& query) const {
    try {
        // Preprocess and get embedding
        std::string processed_query = preprocessQuery(query);
        auto embedding = text_embedder->embed(processed_query);
        
        // Normalize embedding vector
        float norm = 0.0f;
        for (float val : embedding) {
            norm += val * val;
        }
        norm = std::sqrt(norm);
        
        if (norm > 0) {
            for (float& val : embedding) {
                val /= norm;
            }
        }
        
        return embedding;
    } catch (const std::exception& e) {
        spdlog::error("Error vectorizing query: {}", e.what());
        return std::vector<float>(384, 0.0f);
    }
}

std::string BookQueryEngine::preprocessQuery(const std::string& query) const {
    std::string processed = query;
    
    // Convert to lowercase
    std::transform(processed.begin(), processed.end(), processed.begin(), ::tolower);
    
    // Remove punctuation
    processed.erase(
        std::remove_if(processed.begin(), processed.end(), ::ispunct),
        processed.end()
    );
    
    // Remove excessive whitespace
    std::regex whitespace_regex("\\s+");
    processed = std::regex_replace(processed, whitespace_regex, " ");
    
    // Trim
    processed = std::regex_replace(processed, std::regex("^\\s+|\\s+$"), "");
    
    return processed;
}

bool BookQueryEngine::passesFilter(const Book& book, const QueryFilter& filter) const {
    // Check genres
    if (filter.genres && !filter.genres->empty()) {
        bool has_matching_genre = false;
        for (const auto& genre : book.getGenres()) {
            if (std::find(filter.genres->begin(), filter.genres->end(), genre) != filter.genres->end()) {
                has_matching_genre = true;
                break;
            }
        }
        if (!has_matching_genre) return false;
    }

    // Check rating range
    if (filter.min_rating && book.getAverageRating() < *filter.min_rating) return false;
    if (filter.max_rating && book.getAverageRating() > *filter.max_rating) return false;

    // Check ratings count
    if (filter.min_ratings_count && book.getRatingsCount() < *filter.min_ratings_count) return false;

    // Check publication year
    int pub_year = book.getPublicationYear();
    if (filter.publication_year_start && pub_year < *filter.publication_year_start) return false;
    if (filter.publication_year_end && pub_year > *filter.publication_year_end) return false;

    // Check language
    if (filter.language && book.getLanguage() != *filter.language) return false;

    // Check ebook availability
    if (filter.ebook_only && !book.isEbook()) return false;

    // Check authors
    if (filter.authors && !filter.authors->empty()) {
        if (std::find(filter.authors->begin(), filter.authors->end(), book.getAuthor()) 
            == filter.authors->end()) {
            return false;
        }
    }

    return true;
}

void BookQueryEngine::rankResults(std::vector<RecommendationResult>& results) const {
    // Calculate diversity score
    double diversity_score = calculateDiversityScore(results);

    // Sort based on weighted scores
    std::sort(results.begin(), results.end(),
        [this, diversity_score](const RecommendationResult& a, const RecommendationResult& b) {
            const double SIMILARITY_WEIGHT = 0.5;
            const double POPULARITY_WEIGHT = 0.3;
            const double DIVERSITY_WEIGHT = 0.2;

            double a_score = (SIMILARITY_WEIGHT * a.similarity_score) +
                           (POPULARITY_WEIGHT * a.book.getPopularityScore()) +
                           (DIVERSITY_WEIGHT * diversity_score);

            double b_score = (SIMILARITY_WEIGHT * b.similarity_score) +
                           (POPULARITY_WEIGHT * b.book.getPopularityScore()) +
                           (DIVERSITY_WEIGHT * diversity_score);

            return a_score > b_score;
        });
}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::processSearchResults(
    const std::vector<BookVectorStore::SearchResult>& results,
    const std::string& query,
    const QueryFilter& filter
) const {
    std::vector<RecommendationResult> recommendations;
    recommendations.reserve(results.size());

    for (const auto& result : results) {
        Book book(result.document.getId(),
                 result.document.getMetadata().at("title").get<std::string>(),
                 result.document.getMetadata().at("author").get<std::string>(),
                 result.document.getMetadata().at("genres").get<std::vector<std::string>>(),
                 result.document.getText(),
                 result.document.getMetadata().at("page_count").get<int>(),
                 result.document.getMetadata().at("average_rating").get<double>(),
                 result.document.getMetadata().at("ratings_count").get<int>(),
                 result.document.getMetadata().at("review_count").get<int>(),
                 result.document.getSeries(),
                 result.document.getMetadata().at("language").get<std::string>(),
                 result.document.getMetadata().at("publisher").get<std::string>(),
                 result.document.getMetadata().at("publication_date").get<std::string>(),
                 result.document.getMetadata().at("isbn13").get<std::string>(),
                 result.document.getMetadata().at("is_ebook").get<bool>());

        if (passesFilter(book, filter)) {
            recommendations.push_back({
                book,
                result.similarity,
                generateExplanation(book, query)
            });
        }
    }

    return recommendations;
}

std::string BookQueryEngine::generateExplanation(const Book& book, const std::string& query) const {
    std::ostringstream explanation;
    
    explanation << "Recommended because ";

    // Match based on query content
    if (!query.empty()) {
        explanation << "it matches your interest in "
                   << book.getGenres()[0];  // Use first genre
                   
        if (query.find(book.getAuthor()) != std::string::npos) {
            explanation << " and is written by " << book.getAuthor();
        }
    }

    // Add rating information
    explanation << ". It has a rating of " << book.getAverageRating()
               << "/5 from " << book.getRatingsCount() << " readers";

    // Add series information if applicable
    if (auto series = book.getSeries()) {
        explanation << " and is part of the " << *series << " series";
    }

    // Add genre information
    explanation << ". This book combines elements of ";
    const auto& genres = book.getGenres();
    for (size_t i = 0; i < genres.size() && i < 3; ++i) {
        if (i > 0) explanation << (i == genres.size() - 1 ? " and " : ", ");
        explanation << genres[i];
    }

    return explanation.str();
}

double BookQueryEngine::calculateDiversityScore(
    const std::vector<RecommendationResult>& results
) const {
    if (results.empty()) return 0.0;

    // Calculate genre diversity
    std::unordered_set<std::string> unique_genres;
    std::unordered_set<std::string> unique_authors;

    for (const auto& result : results) {
        unique_authors.insert(result.book.getAuthor());
        for (const auto& genre : result.book.getGenres()) {
            unique_genres.insert(genre);
        }
    }

    // Normalize diversity scores
    double genre_diversity = static_cast<double>(unique_genres.size()) / 
                           (results.size() * 3.0);  // Assuming avg 3 genres per book
    double author_diversity = static_cast<double>(unique_authors.size()) /
                            results.size();

    return (genre_diversity + author_diversity) / 2.0;
}

}