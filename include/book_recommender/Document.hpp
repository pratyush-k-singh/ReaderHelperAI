#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>

namespace movie_recommender {

class Document {
public:
    using Metadata = std::map<std::string, nlohmann::json>;
    using Embedding = std::vector<float>;
    using TimePoint = std::chrono::system_clock::time_point;

    Document(
        std::string id,
        std::string text,
        Metadata metadata,
        std::optional<Embedding> embedding = std::nullopt
    );

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getText() const { return text_; }
    const Metadata& getMetadata() const { return metadata_; }
    const std::optional<Embedding>& getEmbedding() const { return embedding_; }
    const TimePoint& getTimestamp() const { return timestamp_; }

    // Setters
    void setEmbedding(Embedding embedding);
    void updateMetadata(const Metadata& new_metadata);

    // Utility functions
    std::string getGenreString() const;
    std::optional<std::string> getCollection() const;
    std::map<std::string, double> getMetrics() const;
    double calculateEngagementScore() const;
    double calculateRoi() const;
    bool isSuccessful() const;

    // Serialization
    nlohmann::json toJson() const;
    static Document fromJson(const nlohmann::json& json);

private:
    std::string id_;
    std::string text_;
    Metadata metadata_;
    std::optional<Embedding> embedding_;
    TimePoint timestamp_;

    static constexpr double ENGAGEMENT_THRESHOLD = 5.0;
    static constexpr double ROI_THRESHOLD = 0.5;
};

}