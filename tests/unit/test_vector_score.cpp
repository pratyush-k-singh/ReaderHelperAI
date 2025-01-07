#include <catch2/catch.hpp>
#include <book_recommender/BookVectorStore.hpp>

using namespace book_recommender;

TEST_CASE("VectorStore Basic Operations", "[vector_store]") {
    BookVectorStore store(384);  // 384-dimensional vectors

    SECTION("Index Creation and Search") {
        // Create test documents
        std::vector<Document> documents;
        std::vector<float> embedding1(384, 0.1f);
        std::vector<float> embedding2(384, 0.2f);
        
        Document doc1("1", "test1", {{"title", "Book 1"}}, embedding1);
        Document doc2("2", "test2", {{"title", "Book 2"}}, embedding2);
        documents.push_back(doc1);
        documents.push_back(doc2);

        // Initialize index
        REQUIRE_NOTHROW(store.initializeIndex(documents));

        // Search
        std::vector<float> query_vector(384, 0.15f);
        auto results = store.search(query_vector, 2);
        
        REQUIRE(results.size() == 2);
        REQUIRE(results[0].similarity > 0.0f);
    }

    SECTION("Document Management") {
        std::vector<float> embedding(384, 0.1f);
        Document doc("test_id", "test", {{"title", "Test Book"}}, embedding);

        // Add document
        REQUIRE_NOTHROW(store.addDocuments({doc}));

        // Remove document
        REQUIRE_NOTHROW(store.removeDocument("test_id"));
    }

    SECTION("Index Persistence") {
        std::vector<float> embedding(384, 0.1f);
        Document doc("test_id", "test", {{"title", "Test Book"}}, embedding);
        store.addDocuments({doc});

        // Save index
        REQUIRE_NOTHROW(store.saveIndex("test_index"));

        // Load index
        BookVectorStore new_store(384);
        REQUIRE_NOTHROW(new_store.loadIndex("test_index"));

        // Verify loaded index
        auto results = new_store.search(embedding, 1);
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].doc_id == "test_id");
    }
}