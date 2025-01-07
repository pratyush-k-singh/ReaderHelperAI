#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <memory>
#include <book_recommender/BookRecommender.hpp>
#include <spdlog/spdlog.h>

using namespace book_recommender;

class BookRecommenderCLI {
public:
    BookRecommenderCLI() {
        initializeRecommender();
    }

    void run() {
        printWelcome();
        
        while (true) {
            printMenu();
            int choice = getMenuChoice();
            
            if (choice == 0) {
                break;
            }
            
            handleMenuChoice(choice);
            
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        std::cout << "\nThank you for using the Book Recommender!\n";
    }

private:
    std::unique_ptr<BookRecommender> recommender_;
    static constexpr int MAX_DISPLAY_LENGTH = 50;

    void initializeRecommender() {
        try {
            BookRecommender::RecommenderConfig config{
                .data_file = "books.csv",
                .embedding_dimension = 384,
                .cache_size = 1000,
                .language_filter = "en",
                .min_ratings = 100,
                .load_existing_index = true
            };
            
            recommender_ = std::make_unique<BookRecommender>(config);
            spdlog::info("Recommender system initialized successfully");
        } catch (const std::exception& e) {
            spdlog::error("Failed to initialize recommender: {}", e.what());
            throw;
        }
    }

    void printWelcome() {
        std::cout << "\n┌────────────────────────────────────┐\n";
        std::cout << "│     Welcome to Book Recommender     │\n";
        std::cout << "└────────────────────────────────────┘\n";
    }

    void printMenu() {
        std::cout << "\nWhat would you like to do?\n\n";
        std::cout << "1. Get personalized book recommendations\n";
        std::cout << "2. Find similar books\n";
        std::cout << "3. Explore author recommendations\n";
        std::cout << "4. Browse popular genres\n";
        std::cout << "5. See top rated books\n";
        std::cout << "6. Advanced search\n";
        std::cout << "7. Browse book series\n";
        std::cout << "8. View reading statistics\n";
        std::cout << "0. Exit\n\n";
        std::cout << "Enter your choice: ";
    }

    int getMenuChoice() {
        int choice;
        while (!(std::cin >> choice) || choice < 0 || choice > 8) {
            std::cout << "Invalid choice. Please enter a number between 0 and 8: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return choice;
    }

    void handleMenuChoice(int choice) {
        try {
            switch (choice) {
                case 1: handleGetRecommendations(); break;
                case 2: handleSimilarBooks(); break;
                case 3: handleAuthorRecommendations(); break;
                case 4: handlePopularGenres(); break;
                case 5: handleTopRatedBooks(); break;
                case 6: handleAdvancedSearch(); break;
                case 7: handleSeriesRecommendations(); break;
                case 8: handleReadingStatistics(); break;
                default: std::cout << "Invalid choice. Please try again.\n";
            }
        } catch (const std::exception& e) {
            spdlog::error("Error: {}", e.what());
            std::cout << "An error occurred. Please try again.\n";
        }
    }

    void handleGetRecommendations() {
        std::cout << "\n📚 What kind of books are you interested in? \n";
        std::cout << "(Describe your interests, preferred genres, themes, etc.)\n> ";
        
        std::string query;
        std::getline(std::cin, query);

        auto filter = getBasicFilter();
        auto recommendations = recommender_->getRecommendations(query, filter);
        printRecommendations(recommendations);
    }

    void handleSimilarBooks() {
        std::cout << "\n📖 Enter the title of a book you enjoyed: \n> ";
        std::string title;
        std::getline(std::cin, title);

        try {
            auto books = recommender_->searchBooks(title);
            if (books.empty()) {
                std::cout << "\nSorry, I couldn't find that book in the database.\n";
                return;
            }
            
            if (books.size() > 1) {
                std::cout << "\nI found multiple matches. Which book did you mean?\n\n";
                for (size_t i = 0; i < books.size(); ++i) {
                    std::cout << i + 1 << ". " << books[i].getTitle() 
                              << " by " << books[i].getAuthor() << " (" 
                              << books[i].getPublicationYear() << ")\n";
                }
                
                std::cout << "\nEnter the number of your choice (1-" << books.size() << "): ";
                size_t choice;
                std::cin >> choice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                if (choice < 1 || choice > books.size()) {
                    std::cout << "Invalid choice.\n";
                    return;
                }
                
                auto similar = recommender_->getSimilarBooks(books[choice - 1].getId());
                printRecommendations(similar);
            } else {
                auto similar = recommender_->getSimilarBooks(books[0].getId());
                printRecommendations(similar);
            }
        } catch (const std::exception& e) {
            spdlog::error("Error finding similar books: {}", e.what());
            std::cout << "An error occurred while searching for similar books.\n";
        }
    }

    void handleAuthorRecommendations() {
        std::cout << "\n✍️ Enter the name of an author: \n> ";
        std::string author;
        std::getline(std::cin, author);

        auto filter = getBasicFilter();
        try {
            auto recommendations = recommender_->getAuthorRecommendations(author, filter);
            
            if (recommendations.empty()) {
                std::cout << "\nSorry, I couldn't find any books by that author.\n";
                return;
            }
            
            printRecommendations(recommendations);
        } catch (const std::exception& e) {
            spdlog::error("Error getting author recommendations: {}", e.what());
            std::cout << "An error occurred while getting author recommendations.\n";
        }
    }

    void handlePopularGenres() {
        std::cout << "\n📊 Top Genres:\n\n";
        auto genres = recommender_->getPopularGenres(10);
        
        for (size_t i = 0; i < genres.size(); ++i) {
            std::cout << std::setw(2) << i + 1 << ". " << genres[i] << "\n";
        }

        std::cout << "\nWould you like to see recommendations for any of these genres? (y/n): ";
        std::string response;
        std::getline(std::cin, response);

        if (response == "y" || response == "Y") {
            std::cout << "Enter the number of the genre (1-" << genres.size() << "): ";
            size_t choice;
            while (!(std::cin >> choice) || choice < 1 || choice > genres.size()) {
                std::cout << "Invalid choice. Please enter a number between 1 and " 
                          << genres.size() << ": ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            BookQueryEngine::QueryFilter filter;
            filter.genres = std::vector<std::string>{genres[choice - 1]};
            auto recommendations = recommender_->getRecommendations(
                "best " + genres[choice - 1] + " books",
                filter
            );
            printRecommendations(recommendations);
        }
    }

    void handleTopRatedBooks() {
        std::cout << "\n⭐ How many top rated books would you like to see? (1-50): ";
        int count;
        while (!(std::cin >> count) || count < 1 || count > 50) {
            std::cout << "Please enter a number between 1 and 50: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto books = recommender_->getTopRatedBooks(count);
        printBooks(books);
    }

    void handleAdvancedSearch() {
        auto filter = getAdvancedFilter();
        
        std::cout << "\n🔍 Enter your search query: \n> ";
        std::string query;
        std::getline(std::cin, query);

        auto books = recommender_->searchBooks(query, filter);
        printBooks(books);
    }

    void handleSeriesRecommendations() {
        std::cout << "\n📚 Enter the name of a book series: \n> ";
        std::string series;
        std::getline(std::cin, series);

        try {
            auto recommendations = recommender_->getSeriesRecommendations(series);
            
            if (recommendations.empty()) {
                std::cout << "\nSorry, I couldn't find that series.\n";
                return;
            }
            
            printRecommendations(recommendations);
        } catch (const std::exception& e) {
            spdlog::error("Error getting series recommendations: {}", e.what());
            std::cout << "An error occurred while getting series recommendations.\n";
        }
    }

    void handleReadingStatistics() {
        std::cout << "\n📊 Reading Statistics:\n";
        std::cout << "====================\n\n";

        auto popular_genres = recommender_->getPopularGenres(5);
        auto top_authors = recommender_->getPopularAuthors(5);
        auto top_rated = recommender_->getTopRatedBooks(5);

        std::cout << "Top Genres:\n";
        for (const auto& genre : popular_genres) {
            std::cout << "  • " << genre << "\n";
        }

        std::cout << "\nTop Authors:\n";
        for (const auto& author : top_authors) {
            std::cout << "  • " << author << "\n";
        }

        std::cout << "\nHighest Rated Books:\n";
        for (const auto& book : top_rated) {
            std::cout << "  • " << book.getTitle() 
                      << " (" << book.getAverageRating() << "/5.0)\n";
        }
    }

    BookQueryEngine::QueryFilter getBasicFilter() {
        BookQueryEngine::QueryFilter filter;
        
        std::cout << "\nMinimum rating (0-5, press Enter for no minimum): ";
        std::string input;
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                double rating = std::stod(input);
                if (rating >= 0 && rating <= 5) {
                    filter.min_rating = rating;
                }
            } catch (...) {}
        }

        return filter;
    }

    BookQueryEngine::QueryFilter getAdvancedFilter() {
        BookQueryEngine::QueryFilter filter;
        std::string input;

        // Get genres
        std::cout << "\nEnter genres (comma-separated, press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            filter.genres = splitString(input, ',');
        }

        // Get rating range
        std::cout << "Minimum rating (0-5, press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                filter.min_rating = std::stod(input);
            } catch (...) {}
        }

        std::cout << "Maximum rating (0-5, press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                filter.max_rating = std::stod(input);
            } catch (...) {}
        }

        // Get year range
        std::cout << "Start year (press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                filter.publication_year_start = std::stoi(input);
            } catch (...) {}
        }

        std::cout << "End year (press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                filter.publication_year_end = std::stoi(input);
            } catch (...) {}
        }

        // Get language
        std::cout << "Language (press Enter for any): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            filter.language = input;
        }

        // Get ebook preference
        std::cout << "Ebook only? (y/n, press Enter to skip): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            filter.ebook_only = (std::tolower(input[0]) == 'y');
        }

        return filter;
    }

    void printRecommendations(const std::vector<BookQueryEngine::RecommendationResult>& recommendations) {
        if (recommendations.empty()) {
            std::cout << "\nNo recommendations found matching your criteria.\n";
            return;
        }

        std::cout << "\n📚 Recommended Books:\n";
        std::cout << "===================\n\n";

        for (size_t i = 0; i < recommendations.size(); ++i) {
            const auto& rec = recommendations[i];
            
            // Print title and author
            std::cout << i + 1 << ". " << rec.book.getTitle() << "\n";
            std::cout << "   Author: " << rec.book.getAuthor() << "\n";
            
            // Print genres
            std::cout << "   Genres: " << joinStrings(rec.book.getGenres(), ", ") << "\n";
            
            // Print rating information
            std::cout << "   Rating: " << std::fixed << std::setprecision(2) 
                      << rec.book.getAverageRating() << "/5.0 (" 
                      << rec.book.getRatingsCount() << " ratings)\n";
            
            // Print series information if available
            if (auto series = rec.book.getSeries()) {
                std::cout << "   Series: " << *series << "\n";
            }
            
            // Print publication year
            std::cout << "   Published: " << rec.book.getPublicationYear() << "\n";
            
            // Print explanation
            std::cout << "   Why recommended: " << truncateText(rec.explanation, MAX_DISPLAY_LENGTH) << "\n";
            
            // Print similarity score
            std::cout << "   Match Score: " << std::fixed << std::setprecision(2) 
                      << (rec.similarity_score * 100) << "%\n\n";
        }
    }

    void printBooks(const std::vector<Book>& books) {
        if (books.empty()) {
            std::cout << "\nNo books found matching your criteria.\n";
            return;
        }

        std::cout << "\n📚 Books:\n";
        std::cout << "========\n\n";

        for (size_t i = 0; i < books.size(); ++i) {
            const auto& book = books[i];
            std::cout << i + 1 << ". " << book.getTitle() << "\n";
            std::cout << "   Author: " << book.getAuthor() << "\n";
            std::cout << "   Genres: " << joinStrings(book.getGenres(), ", ") << "\n";
            std::cout << "   Rating: " << std::fixed << std::setprecision(2) 
                      << book.getAverageRating() << "/5.0 (" 
                      << book.getRatingsCount() << " ratings)\n";
            
            if (auto series = book.getSeries()) {
                std::cout << "   Series: " << *series << "\n";
            }
            
            std::cout << "   Published: " << book.getPublicationYear() << "\n\n";
        }
    }

    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            // Trim whitespace
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }

    std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter) {
        std::string result;
        for (size_t i = 0; i < strings.size(); ++i) {
            result += strings[i];
            if (i < strings.size() - 1) {
                result += delimiter;
            }
        }
        return result;
    }

    std::string truncateText(const std::string& text, size_t max_length) {
        if (text.length() <= max_length) {
            return text;
        }
        return text.substr(0, max_length - 3) + "...";
    }
};

int main(int argc, char* argv[]) {
    try {
        // Configure logging
        spdlog::set_pattern("[%H:%M:%S] [%l] %v");
        spdlog::set_level(spdlog::level::info);

        // Create and run the CLI
        BookRecommenderCLI cli;
        cli.run();
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        std::cerr << "An error occurred. Please check the logs for details.\n";
        return 1;
    }
    return 0;
}