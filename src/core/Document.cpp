#include "book_recommender/Document.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <regex>
#include <sstream>

namespace book_recommender {

Document::Document(
    std::string id,
    std::string text,
    Metadata metadata,
    std::optional<Embedding> embedding
) : id_(std::move(id)),
    text_(std::move(text)),
    metadata_(std::move(metadata)),
    embedding_(std::move(embedding)),
    timestamp_(std::chrono::system_clock::now()) {}

void Document::setEmbedding(Embedding embedding) {
    embedding_ = std::move(embedding);
}

void Document::updateMetadata(const Metadata& new_metadata) {
    metadata_.insert(new_metadata.begin(), new_metadata.end());
}

std::string Document::getGenreString() const {
    auto genres_it = metadata_.find("genres");
    if (genres_it == metadata_.end() || !genres_it->second.is_array()) {
        return "";
    }

    std::vector<std::string> genres = genres_it->second.get<std::vector<std::string>>();
    std::ostringstream oss;
    for (size_t i = 0; i < genres.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << genres[i];
    }
    return oss.str();
}

std::optional<std::string> Document::getSeries() const {
    auto series_it = metadata_.find("series");
    if (series_it == metadata_.end() || series_it->second.is_null()) {
        return std::nullopt;
    }
    return series_it->second.get<std::string>();
}

std::string Document::getAuthor() const {
    auto author_it = metadata_.find("author");
    if (author_it == metadata_.end()) {
        return "";
    }
    return author_it->second.get<std::string>();
}

std::map<std::string, double> Document::getMetrics() const {
    std::map<std::string, double> metrics;
    const std::vector<std::string> metric_keys = {
        "page_count", "average_rating", "ratings_count",
        "review_count", "publication_year"
    };

    for (const auto& key : metric_keys) {
        auto it = metadata_.find(key);
        if (it != metadata_.end() && it->second.is_number()) {
            metrics[key] = it->second.get<double>();
        } else {
            metrics[key] = 0.0;
        }
    }
    return metrics;
}

double Document::calculateEngagementScore() const {
    auto metrics = getMetrics();
    double rating_weight = std::min(metrics["ratings_count"] / static_cast<double>(MIN_RATINGS), 1.0);
    return metrics["average_rating"] * rating_weight;
}

bool Document::isRecommended() const {
    auto metrics = getMetrics();
    return metrics["average_rating"] >= ENGAGEMENT_THRESHOLD && 
           metrics["ratings_count"] >= MIN_RATINGS;
}

int Document::getPublicationYear() const {
    auto metrics = getMetrics();
    return static_cast<int>(metrics["publication_year"]);
}

std::string Document::getReadingLevel() const {
    auto metrics = getMetrics();
    int page_count = static_cast<int>(metrics["page_count"]);
    
    if (page_count < 100) return "Easy";
    if (page_count < 300) return "Intermediate";
    if (page_count < 500) return "Advanced";
    return "Expert";
}

double Document::cosineSimilarity(const Embedding& a, const Embedding& b) const {
    if (a.size() != b.size() || a.empty()) {
        return 0.0;
    }

    double dot_product = std::inner_product(a.begin(), a.end(), b.begin(), 0.0);
    
    double norm_a = std::sqrt(std::inner_product(a.begin(), a.end(), a.begin(), 0.0));
    double norm_b = std::sqrt(std::inner_product(b.begin(), b.end(), b.begin(), 0.0));
    
    if (norm_a == 0.0 || norm_b == 0.0) {
        return 0.0;
    }
    
    return dot_product / (norm_a * norm_b);
}

double Document::getTextSimilarity(const Document& other) const {
    if (!embedding_ || !other.embedding_) {
        return 0.0;
    }
    
    return cosineSimilarity(*embedding_, *other.embedding_);
}

nlohmann::json Document::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["text"] = text_;
    j["metadata"] = metadata_;
    if (embedding_) {
        j["embedding"] = *embedding_;
    }
    j["timestamp"] = std::chrono::system_clock::to_time_t(timestamp_);
    
    // Add computed fields
    j["genres"] = getGenreString();
    j["series"] = getSeries();
    j["author"] = getAuthor();
    j["metrics"] = getMetrics();
    j["engagement_score"] = calculateEngagementScore();
    j["is_recommended"] = isRecommended();
    j["publication_year"] = getPublicationYear();
    j["reading_level"] = getReadingLevel();
    
    return j;
}

Document Document::fromJson(const nlohmann::json& j) {
    std::optional<Embedding> embedding;
    if (j.contains("embedding") && !j["embedding"].is_null()) {
        embedding = j["embedding"].get<Embedding>();
    }

    return Document(
        j["id"].get<std::string>(),
        j["text"].get<std::string>(),
        j["metadata"].get<Metadata>(),
        embedding
    );
}

}