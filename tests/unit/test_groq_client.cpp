#include <catch2/catch.hpp>
#include "../../src/utils/GroqClient.hpp"
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace book_recommender;

// Helper to check if GROQ_API_KEY is set
bool hasApiKey() {
    return std::getenv("GROQ_API_KEY") != nullptr;
}

TEST_CASE("GroqClient Initialization", "[groq]") {
    SECTION("Singleton Instance") {
        REQUIRE_NOTHROW(GroqClient::getInstance());
        auto& instance1 = GroqClient::getInstance();
        auto& instance2 = GroqClient::getInstance();
        REQUIRE(&instance1 == &instance2);
    }

    SECTION("API Key Validation") {
        if (!hasApiKey()) {
            WARN("GROQ_API_KEY not set, skipping API key validation test");
            return;
        }
        REQUIRE_NOTHROW(GroqClient::getInstance());
    }
}

TEST_CASE("GroqClient Embedding Generation", "[groq]") {
    if (!hasApiKey()) {
        WARN("GROQ_API_KEY not set, skipping embedding tests");
        return;
    }

    auto& client = GroqClient::getInstance();

    SECTION("Basic Embedding") {
        std::string text = "Test text for embedding";
        auto embedding = client.getEmbedding(text);
        
        REQUIRE(!embedding.empty());
        REQUIRE(embedding.size() == 384);  // Check expected dimension
    }

    SECTION("Empty Input") {
        REQUIRE_THROWS(client.getEmbedding(""));
    }

    SECTION("Long Text") {
        std::string long_text(10000, 'a');  // 10K character text
        REQUIRE_NOTHROW(client.getEmbedding(long_text));
    }

    SECTION("Rate Limiting") {
        // Test multiple rapid requests
        std::vector<std::vector<float>> embeddings;
        for (int i = 0; i < 3; ++i) {
            auto embedding = client.getEmbedding("Test " + std::to_string(i));
            embeddings.push_back(embedding);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        REQUIRE(embeddings.size() == 3);
    }
}

TEST_CASE("GroqClient Query Enhancement", "[groq]") {
    if (!hasApiKey()) {
        WARN("GROQ_API_KEY not set, skipping query enhancement tests");
        return;
    }

    auto& client = GroqClient::getInstance();

    SECTION("Basic Query Enhancement") {
        std::string query = "fantasy books with magic";
        std::string enhanced = client.enhanceQuery(query);
        
        REQUIRE(!enhanced.empty());
        REQUIRE(enhanced.length() > query.length());
        REQUIRE(enhanced.find("magic") != std::string::npos);
    }

    SECTION("Complex Query") {
        std::string query = "books like The Lord of the Rings but with a modern setting";
        std::string enhanced = client.enhanceQuery(query);
        
        REQUIRE(!enhanced.empty());
        REQUIRE(enhanced.find("fantasy") != std::string::npos);
    }
}

TEST_CASE("GroqClient Explanation Generation", "[groq]") {
    if (!hasApiKey()) {
        WARN("GROQ_API_KEY not set, skipping explanation tests");
        return;
    }

    auto& client = GroqClient::getInstance();

    SECTION("Basic Explanation") {
        std::string book_info = "Title: Test Book\n"
                               "Author: Test Author\n"
                               "Genres: fantasy, adventure\n"
                               "Rating: 4.5/5.0";
        std::string query = "fantasy books with magic";
        
        std::string explanation = client.generateExplanation(book_info, query);
        
        REQUIRE(!explanation.empty());
        REQUIRE(explanation.length() > 50);  // Reasonable explanation length
        REQUIRE(explanation.find("fantasy") != std::string::npos);
    }

    SECTION("Detailed Book Match") {
        std::string book_info = "Title: The Quantum Thief\n"
                               "Author: Hannu Rajaniemi\n"
                               "Genres: science fiction, cyberpunk\n"
                               "Rating: 4.2/5.0\n"
                               "Description: A post-human science fiction heist story.";
        std::string query = "complex sci-fi with deep worldbuilding";
        
        std::string explanation = client.generateExplanation(book_info, query);
        
        REQUIRE(!explanation.empty());
        REQUIRE(explanation.find("sci-fi") != std::string::npos || 
                explanation.find("science fiction") != std::string::npos);
    }
}

TEST_CASE("GroqClient Error Handling", "[groq]") {
    if (!hasApiKey()) {
        WARN("GROQ_API_KEY not set, skipping error handling tests");
        return;
    }

    auto& client = GroqClient::getInstance();

    SECTION("Invalid Request") {
        std::string very_long_text(100000, 'a');  // Exceeds token limit
        REQUIRE_THROWS(client.getEmbedding(very_long_text));
    }

    SECTION("Network Error Recovery") {
        // Simulate network issues by making rapid requests
        for (int i = 0; i < 5; ++i) {
            REQUIRE_NOTHROW(client.getEmbedding("Test " + std::to_string(i)));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
