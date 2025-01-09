#include "book_recommender/Book.hpp"
#include <algorithm>
#include <regex>
#include <sstream>

namespace book_recommender {

Book::Book(
    std::string id,
    std::string title,
    std::string author,
    std::vector<std::string> genres,
    std::string description,
    int page_count,
    double average_rating,
    int ratings_count,
    int review_count,
    std::optional<std::string> series,
    std::string language,
    std::string publisher,
    std::string publication_date,
    std::string isbn13,
    bool is_ebook
) : id_(std::move(id)),
    title_(std::move(title)),
    author_(std::move(author)),
    genres_(std::move(genres)),
    description_(std::move(description)),
    page_count_(page_count),
    average_rating_(average_rating),
    ratings_count_(ratings_count),
    review_count_(review_count),
    series_(std::move(series)),
    language_(std::move(language)),
    publisher_(std::move(publisher)),
    publication_date_(std::move(publication_date)),
    isbn13_(std::move(isbn13)),
    is_ebook_(is_ebook) {}

double Book::getEngagementScore() const {
    // Calculate engagement based on ratings and reviews
    double rating_weight = std::min(ratings_count_ / static_cast<double>(MIN_RATINGS_FOR_RELIABLE), 1.0);
    double review_ratio = review_count_ > 0 ? static_cast<double>(review_count_) / ratings_count_ : 0.0;
    
    return (average_rating_ * rating_weight + review_ratio * 5.0) / 2.0;
}

double Book::getPopularityScore() const {
    // Normalize ratings count to a 0-1 scale (assuming 10000 ratings is very popular)
    double normalized_ratings = std::min(ratings_count_ / 10000.0, 1.0);
    
    // Combine rating and popularity metrics
    return (normalized_ratings * 0.7 + (average_rating_ / 5.0) * 0.3) * 100.0;
}

bool Book::isHighlyRated() const {
    return average_rating_ >= HIGH_RATING_THRESHOLD && 
           ratings_count_ >= MIN_RATINGS_FOR_RELIABLE;
}

int Book::getPublicationYear() const {
    std::regex year_regex("\\d{4}");
    std::smatch match;
    if (std::regex_search(publication_date_, match, year_regex)) {
        return std::stoi(match[0]);
    }
    return 0;
}

nlohmann::json Book::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["title"] = title_;
    j["author"] = author_;
    j["genres"] = genres_;
    j["description"] = description_;
    j["page_count"] = page_count_;
    j["average_rating"] = average_rating_;
    j["ratings_count"] = ratings_count_;
    j["review_count"] = review_count_;
    j["series"] = series_.has_value() ? series_.value() : nullptr;
    j["language"] = language_;
    j["publisher"] = publisher_;
    j["publication_date"] = publication_date_;
    j["isbn13"] = isbn13_;
    j["is_ebook"] = is_ebook_;
    
    // Add computed fields
    j["engagement_score"] = getEngagementScore();
    j["popularity_score"] = getPopularityScore();
    j["highly_rated"] = isHighlyRated();
    j["publication_year"] = getPublicationYear();
    
    return j;
}

Book Book::fromJson(const nlohmann::json& j) {
    std::optional<std::string> series;
    if (!j["series"].is_null()) {
        series = j["series"].get<std::string>();
    }

    return Book(
        j["id"].get<std::string>(),
        j["title"].get<std::string>(),
        j["author"].get<std::string>(),
        j["genres"].get<std::vector<std::string>>(),
        j["description"].get<std::string>(),
        j["page_count"].get<int>(),
        j["average_rating"].get<double>(),
        j["ratings_count"].get<int>(),
        j["review_count"].get<int>(),
        series,
        j["language"].get<std::string>(),
        j["publisher"].get<std::string>(),
        j["publication_date"].get<std::string>(),
        j["isbn13"].get<std::string>(),
        j["is_ebook"].get<bool>()
    );
}

}