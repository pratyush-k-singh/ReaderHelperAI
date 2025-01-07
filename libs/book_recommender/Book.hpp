#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace book_recommender {

class Book {
public:
    Book(
        std::string id,
        std::string title,
        std::string author,
        std::vector<std::string> genres,
        std::string description,
        int page_count,
        double average_rating,
        int ratings_count,
        int review_count,
        std::optional<std::string> series = std::nullopt,
        std::string language = "en",
        std::string publisher = "",
        std::string publication_date = "",
        std::string isbn13 = "",
        bool is_ebook = false
    );

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getAuthor() const { return author_; }
    const std::vector<std::string>& getGenres() const { return genres_; }
    const std::string& getDescription() const { return description_; }
    int getPageCount() const { return page_count_; }
    double getAverageRating() const { return average_rating_; }
    int getRatingsCount() const { return ratings_count_; }
    int getReviewCount() const { return review_count_; }
    const std::optional<std::string>& getSeries() const { return series_; }
    const std::string& getLanguage() const { return language_; }
    const std::string& getPublisher() const { return publisher_; }
    const std::string& getPublicationDate() const { return publication_date_; }
    const std::string& getIsbn13() const { return isbn13_; }
    bool isEbook() const { return is_ebook_; }

    // Computed properties
    double getEngagementScore() const;
    double getPopularityScore() const;
    bool isHighlyRated() const;
    int getPublicationYear() const;
    
    // Serialization
    nlohmann::json toJson() const;
    static Book fromJson(const nlohmann::json& json);

private:
    std::string id_;
    std::string title_;
    std::string author_;
    std::vector<std::string> genres_;
    std::string description_;
    int page_count_;
    double average_rating_;
    int ratings_count_;
    int review_count_;
    std::optional<std::string> series_;
    std::string language_;
    std::string publisher_;
    std::string publication_date_;
    std::string isbn13_;
    bool is_ebook_;

    static constexpr double HIGH_RATING_THRESHOLD = 4.0;
    static constexpr int MIN_RATINGS_FOR_RELIABLE = 100;
};

}