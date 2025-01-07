#include <iostream>
#include <book_recommender/BookRecommender.hpp>
#include <spdlog/spdlog.h>

using namespace book_recommender;

void printRecommendations(const std::vector<BookQueryEngine::RecommendationResult>& recommendations) {
    std::cout << "\nRecommendations:\n";
    std::cout << "================\n\n";
    
    for (size_t i = 0; i < recommendations.size(); ++i) {
        const auto& rec = recommendations[i];
        std::cout << i + 1 << ". " << rec.book.getTitle() << "\n";
        std::cout << "   Author: " << rec.book.getAuthor() << "\n";
        std::cout << "   Genres: " << join(rec.book.getGenres(), ", ") << "\n";
        std::cout << "   Rating: " << rec.book.getAverageRating() 
                  << "/5.0 (" << rec.book.getRatingsCount() << " ratings)\n";
        std::cout << "   Similarity Score: " << rec.similarity_score << "\n";
        std::cout << "   Why this book: " << rec.explanation << "\n\n";
    }
}

int main() {
    try {
        // Initialize the recommender
        BookRecommender::RecommenderConfig config{
            .data_file = "books.csv",
            .embedding_dimension = 384,
            .cache_size = 1000,
            .language_filter = "en",
            .min_ratings = 100
        };
        
        BookRecommender recommender(config);
        
        // Example 1: Get recommendations based on a query
        std::cout << "Getting recommendations for 'fantasy books with magic schools'...\n";
        auto recommendations = recommender.getRecommendations(
            "fantasy books with magic schools",
            {
                .genres = {{"fantasy"}},
                .min_rating = 4.0,
                .min_ratings_count = 1000
            }
        );
        printRecommendations(recommendations);

        // Example 2: Get similar books
        std::cout << "\nGetting books similar to 'The Name of the Wind'...\n";
        auto similar_books = recommender.getSimilarBooks(
            "name_of_the_wind_id",  // You would need the actual book ID
            {
                .min_rating = 4.0
            }
        );
        printRecommendations(similar_books);

        // Example 3: Get author recommendations
        std::cout << "\nGetting recommendations for books by Brandon Sanderson...\n";
        auto author_recs = recommender.getAuthorRecommendations(
            "Brandon Sanderson",
            {
                .min_ratings_count = 5000
            }
        );
        printRecommendations(author_recs);

        // Example 4: Get popular genres
        std::cout << "\nPopular genres:\n";
        auto popular_genres = recommender.getPopularGenres(5);
        for (const auto& genre : popular_genres) {
            std::cout << "- " << genre << "\n";
        }

        // Example 5: Get top rated books
        std::cout << "\nTop rated books:\n";
        auto top_books = recommender.getTopRatedBooks(5);
        for (const auto& book : top_books) {
            std::cout << "- " << book.getTitle() 
                      << " (" << book.getAverageRating() << "/5.0)\n";
        }

    } catch (const BookRecommenderError& e) {
        spdlog::error("Recommender error: {}", e.what());
        return 1;
    } catch (const std::exception& e) {
        spdlog::error("Unknown error: {}", e.what());
        return 1;
    }

    return 0;
}

// Helper function to join strings with delimiter
std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::string result;
    for (size_t i = 0; i < strings.size(); ++i) {
        result += strings[i];
        if (i < strings.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}