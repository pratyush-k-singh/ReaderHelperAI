#include <catch2/catch.hpp>
#include <book_recommender/BookDataLoader.hpp>
#include <filesystem>
#include <fstream>

using namespace book_recommender;

// Helper function to create test CSV file
void createTestCsv(const std::string& filename) {
    std::ofstream file(filename);
    file << "id,title,author,genres,description,page_count,average_rating,ratings_count,review_count,series,language,publisher,publication_date,isbn13,is_ebook\n"
         << "1,Test Book,Test Author,[\"fantasy\",\"fiction\"],Test description,300,4.5,1000,500,Test Series,en,Test Publisher,2023-01-01,9781234567890,true\n"
         << "2,Another Book,Author Two,[\"sci-fi\"],Another description,250,4.0,800,400,,en,Publisher Two,2023-02-01,9789876543210,false\n";
    file.close();
}

TEST_CASE("BookDataLoader Basic Functionality", "[data_loader]") {
    // Setup test environment
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "book_recommender_test";
    std::filesystem::create_directories(test_dir);
    std::string test_file = (test_dir / "test_books.csv").string();
    createTestCsv(test_file);

    SECTION("Load and Process Data") {
        BookDataLoader loader(test_file);
        auto [books, documents] = loader.loadAndPreprocess();

        REQUIRE(books.size() == 2);
        REQUIRE(documents.size() == 2);

        // Check first book
        const auto& book1 = books[0];
        REQUIRE(book1.getTitle() == "Test Book");
        REQUIRE(book1.getAuthor() == "Test Author");
        REQUIRE(book1.getGenres().size() == 2);
        REQUIRE(book1.getGenres()[0] == "fantasy");
        REQUIRE(book1.getAverageRating() == Approx(4.5));
        REQUIRE(book1.getSeries().value() == "Test Series");

        // Check second book
        const auto& book2 = books[1];
        REQUIRE(book2.getTitle() == "Another Book");
        REQUIRE(book2.getGenres().size() == 1);
        REQUIRE(book2.getGenres()[0] == "sci-fi");
        REQUIRE(!book2.getSeries().has_value());
    }

    SECTION("Filter Application") {
        BookDataLoader loader(test_file);
        loader.setMinRatings(900);  // Should only get the first book
        auto [books, documents] = loader.loadAndPreprocess();

        REQUIRE(books.size() == 1);
        REQUIRE(books[0].getTitle() == "Test Book");
    }

    SECTION("Language Filter") {
        BookDataLoader loader(test_file);
        loader.setLanguageFilter("fr");  // Should get no books
        auto [books, documents] = loader.loadAndPreprocess();

        REQUIRE(books.empty());
    }

    SECTION("Year Range Filter") {
        BookDataLoader loader(test_file);
        loader.setYearRange(2023, 2023);  // Should get both books
        auto [books, documents] = loader.loadAndPreprocess();

        REQUIRE(books.size() == 2);
    }

    SECTION("Invalid File Handling") {
        BookDataLoader loader("nonexistent.csv");
        REQUIRE_THROWS(loader.loadAndPreprocess());
    }

    // Cleanup
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("BookDataLoader Document Creation", "[data_loader]") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "book_recommender_test";
    std::filesystem::create_directories(test_dir);
    std::string test_file = (test_dir / "test_books.csv").string();
    createTestCsv(test_file);

    BookDataLoader loader(test_file);
    auto [books, documents] = loader.loadAndPreprocess();

    SECTION("Document Metadata") {
        REQUIRE(documents.size() == 2);
        const auto& doc = documents[0];

        // Check metadata conversion
        auto metadata = doc.getMetadata();
        REQUIRE(metadata.contains("title"));
        REQUIRE(metadata.contains("author"));
        REQUIRE(metadata.contains("genres"));
        REQUIRE(metadata.contains("average_rating"));

        // Check specific values
        REQUIRE(metadata["title"].get<std::string>() == "Test Book");
        REQUIRE(metadata["average_rating"].get<double>() == Approx(4.5));
    }

    SECTION("Document Text Generation") {
        const auto& doc = documents[0];
        std::string text = doc.getText();
        
        // Text should contain important book information
        REQUIRE(text.find("Test Book") != std::string::npos);
        REQUIRE(text.find("Test Author") != std::string::npos);
        REQUIRE(text.find("fantasy") != std::string::npos);
    }

    // Cleanup
    std::filesystem::remove_all(test_dir);
}
