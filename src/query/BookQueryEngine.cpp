#include "book_recommender/BookQueryEngine.hpp"
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <regex>
#include <spdlog/spdlog.h>
#include "../utils/GroqClient.hpp"

namespace book_recommender {

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
        QueryFilter author_filter = filter;
        if (!author_filter.authors) {
            author_filter.authors = std::vector<std::string>();
        }
        author_filter.authors->push_back(author);
        
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

std::vector<float> BookQueryEngine::vectorizeQuery(const std::string& query) const {
    try {
        // Use Groq for embedding generation
        auto& groq = GroqClient::getInstance();
        return groq.getEmbedding(preprocessQuery(query));
    } catch (const std::exception& e) {
        spdlog::error("Error vectorizing query with Groq: {}", e.what());
        return std::vector<float>(384, 0.0f);  // Fallback to zero vector
    }
}

std::string BookQueryEngine::enhanceQuery(const std::string& query) const {
    try {
        auto& groq = GroqClient::getInstance();
        return groq.enhanceQuery(query);
    } catch (const std::exception& e) {
        spdlog::error("Error enhancing query with Groq: {}", e.what());
        return query;  // Return original query on error
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

    if (filter.min_rating && book.getAverageRating() < *filter.min_rating) return false;
    if (filter.max_rating && book.getAverageRating() > *filter.max_rating) return false;
    if (filter.min_ratings_count && book.getRatingsCount() < *filter.min_ratings_count) return false;

    int pub_year = book.getPublicationYear();
    if (filter.publication_year_start && pub_year < *filter.publication_year_start) return false;
    if (filter.publication_year_end && pub_year > *filter.publication_year_end) return false;

    if (filter.language && book.getLanguage() != *filter.language) return false;
    if (filter.ebook_only && !book.isEbook()) return false;

    if (filter.authors && !filter.authors->empty()) {
        if (std::find(filter.authors->begin(), filter.authors->end(), book.getAuthor()) 
            == filter.authors->end()) {
            return false;
        }
    }

    return true;
}

void BookQueryEngine::rankResults(std::vector<RecommendationResult>& results) const {
    double diversity_score = calculateDiversityScore(results);

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

std::string BookQueryEngine::generateExplanation(
    const Book& book,
    const std::string& query
) const {
    try {
        auto& groq = GroqClient::getInstance();
        
        // Create detailed book info for better context
        std::ostringstream book_info;
        book_info << "Title: " << book.getTitle() << "\n"
                 << "Author: " << book.getAuthor() << "\n"
                 << "Genres: " << joinStrings(book.getGenres(), ", ") << "\n"
                 << "Rating: " << book.getAverageRating() << "/5.0 from "
                 << book.getRatingsCount() << " readers\n"
                 << "Publication Year: " << book.getPublicationYear() << "\n";
        
        if (auto series = book.getSeries()) {
            book_info << "Series: " << *series << "\n";
        }
        
        book_info << "Description: " << book.getDescription();
        
        return groq.generateExplanation(book_info.str(), query);
    } catch (const std::exception& e) {
        spdlog::error("Error generating explanation with Groq: {}", e.what());
        
        // Fallback to template-based explanation
        std::ostringstream explanation;
        explanation << "Recommended because it matches your interest in "
                   << book.getGenres()[0];
        
        if (book.getAverageRating() >= 4.0) {
            explanation << " and is highly rated with " 
                       << book.getAverageRating() << "/5.0 from "
                       << book.getRatingsCount() << " readers";
        }
        
        if (auto series = book.getSeries()) {
            explanation << ". Part of the " << *series << " series";
        }
        
        return explanation.str();
    }
}

std::vector<BookQueryEngine::RecommendationResult> BookQueryEngine::processSearchResults(
    const std::vector<BookVectorStore::SearchResult>& results,
    const std::string& query,
    const QueryFilter& filter
) const {
    std::vector<RecommendationResult> recommendations;
    recommendations.reserve(results.size());

    for (const auto& result : results) {
        Book book(
            result.document.getId(),
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
            result.document.getMetadata().at("is_ebook").get<bool>()
        );

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

double BookQueryEngine::calculateDiversityScore(
    const std::vector<RecommendationResult>& results
) const {
    if (results.empty()) return 0.0;

    // Calculate genre and author diversity
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

std::string BookQueryEngine::joinStrings(
    const std::vector<std::string>& strings,
    const std::string& delimiter
) const {
    std::string result;
    for (size_t i = 0; i < strings.size(); ++i) {
        result += strings[i];
        if (i < strings.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

}
