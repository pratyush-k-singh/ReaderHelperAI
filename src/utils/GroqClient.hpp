#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cpprest/http_client.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace book_recommender {

class GroqClient {
public:
    static GroqClient& getInstance() {
        static GroqClient instance;
        return instance;
    }

    std::vector<float> getEmbedding(const std::string& text);
    std::string enhanceQuery(const std::string& query);
    std::string generateExplanation(const std::string& book_info, const std::string& query);

private:
    GroqClient();
    
    web::http::client::http_client client_;
    std::string api_key_;
    const std::string base_url_ = "https://api.groq.com/v1/";
    const std::string model_ = "mixtral-8x7b-32768";

    nlohmann::json makeRequest(const std::string& endpoint, const nlohmann::json& data);
    void validateApiKey();
    std::vector<float> parseEmbedding(const nlohmann::json& response);
};

}