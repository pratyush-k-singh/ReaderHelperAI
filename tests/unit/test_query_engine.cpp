#include <catch2/catch.hpp>
#include <book_recommender/BookQueryEngine.hpp>
#include <book_recommender/BookVectorStore.hpp>

using namespace book_recommender;

TEST_CASE("QueryEngine Recommendation Logic", "[query_engine]") {
    auto vector_store = std::make_shared<BookVectorStore>(384);
    BookQueryEngine engine(vector_store);

    SECTION("Basic Query Processing") {
        auto enhanced = engine.enhanceQuery("fantasy magic books");
        REQUIRE_FALSE(enhanced.empty());
        REQUIRE(enhanced.find("magic") != std::string::npos);
    }

    SECTION("Filter Application") {
        BookQueryEngine::QueryFilter filter;
        filter.min_rating = 4.0;
        filter.genres = std::vector<std::string>{"fantasy"};

        Book test_book(
            "1", "Test Book", "Author", {"fantasy"}, "desc",
            200, 4.5, 1000, 500, "Series", "en", "Pub",
            "2023-01-01", "9781234567890", true
        );

        REQUIRE(engine.passesFilter(test_book, filter));

        // Test failing filter
        filter.min_rating = 4.8;
        REQUIRE_FALSE(engine.passesFilter(test_book, filter));
    }

    SECTION("Diversity Scoring") {
        std::vector<BookQueryEngine::RecommendationResult> results;
        
        // Add books from different genres and authors
        results.push_back({
            Book("1", "Book1", "Author1", {"fantasy"}, "desc", 200, 4.0, 1000, 500),
            0.9f, "explanation1"
        });
        results.push_back({
            Book("2", "Book2", "Author2", {"sci-fi"}, "desc", 200, 4.0, 1000, 500),
            0.8f, "explanation2"
        });

        auto diversity_score = engine.calculateDiversityScore(results);
        REQUIRE(diversity_score > 0.0);
    }
}

TEST_CASE("QueryEngine Integration", "[query_engine]") {
    auto vector_store = std::make_shared<BookVectorStore>(384);
    BookQueryEngine engine(vector_store);

    // Setup test data
    std::vector<Document> documents;
    std::vector<float> embedding(384, 0.1f);
    
    Document::Metadata metadata{
        {"title", "Test Fantasy Book"},
        {"author", "Test Author"},
        {"genres", std::vector<std::string>{"fantasy"}},
        {"average_rating", 4.5},
        {"ratings_count", 1000}
    };
    
    documents.push_back(Document("1", "test fantasy book", metadata, embedding));
    vector_store->initializeIndex(documents);

    SECTION("End-to-End Recommendation") {
        auto recommendations = engine.getRecommendations("fantasy books", {}, 5);
        REQUIRE_FALSE(recommendations.empty());
        
        const auto& first_rec = recommendations[0];
        REQUIRE_FALSE(first_rec.explanation.empty());
        REQUIRE(first_rec.similarity_score > 0.0f);
    }
}