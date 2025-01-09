#include "GroqClient.hpp"
#include <stdexcept>
#include <cstdlib>

namespace book_recommender {

GroqClient::GroqClient() : client_(base_url_) {
    validateApiKey();
}

void GroqClient::validateApiKey() {
    api_key_ = std::getenv("GROQ_API_KEY");
    if (api_key_.empty()) {
        throw std::runtime_error("GROQ_API_KEY environment variable not set");
    }
}

std::vector<float> GroqClient::getEmbedding(const std::string& text) {
    nlohmann::json request_data = {
        {"model", model_},
        {"messages", {{
            {"role", "system"},
            {"content", "Generate embedding vectors for text representation."}
        }, {
            {"role", "user"},
            {"content", text}
        }}},
        {"stream", false}
    };

    try {
        auto response = makeRequest("embeddings", request_data);
        return parseEmbedding(response);
    } catch (const std::exception& e) {
        spdlog::error("Error getting embedding: {}", e.what());
        throw;
    }
}

std::string GroqClient::enhanceQuery(const std::string& query) {
    nlohmann::json request_data = {
        {"model", model_},
        {"messages", {{
            {"role", "system"},
            {"content", "Enhance the book search query to improve recommendation results. "
                       "Add relevant themes, genres, and literary elements."}
        }, {
            {"role", "user"},
            {"content", query}
        }}},
        {"temperature", 0.3},
        {"stream", false}
    };

    try {
        auto response = makeRequest("chat/completions", request_data);
        return response["choices"][0]["message"]["content"];
    } catch (const std::exception& e) {
        spdlog::error("Error enhancing query: {}", e.what());
        return query;  // Return original query on error
    }
}

std::string GroqClient::generateExplanation(
    const std::string& book_info,
    const std::string& query
) {
    nlohmann::json request_data = {
        {"model", model_},
        {"messages", {{
            {"role", "system"},
            {"content", "Generate a natural explanation for why this book matches the user's query. "
                       "Focus on specific elements that align with their interests."}
        }, {
            {"role", "user"},
            {"content", "Query: " + query + "\nBook: " + book_info}
        }}},
        {"temperature", 0.7},
        {"stream", false}
    };

    try {
        auto response = makeRequest("chat/completions", request_data);
        return response["choices"][0]["message"]["content"];
    } catch (const std::exception& e) {
        spdlog::error("Error generating explanation: {}", e.what());
        return "This book matches elements of your query.";  // Fallback explanation
    }
}

nlohmann::json GroqClient::makeRequest(
    const std::string& endpoint,
    const nlohmann::json& data
) {
    web::http::http_request request(web::http::methods::POST);
    request.headers().add("Authorization", "Bearer " + api_key_);
    request.headers().add("Content-Type", "application/json");
    request.set_body(data.dump());

    auto response = client_.request(request).get();
    
    if (response.status_code() != 200) {
        throw std::runtime_error("Groq API request failed with status code: " + 
                               std::to_string(response.status_code()));
    }

    return nlohmann::json::parse(response.extract_string().get());
}

std::vector<float> GroqClient::parseEmbedding(const nlohmann::json& response) {
    auto embeddings = response["data"][0]["embedding"];
    return embeddings.get<std::vector<float>>();
}

}
