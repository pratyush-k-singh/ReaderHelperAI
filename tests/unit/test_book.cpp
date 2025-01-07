#include <catch2/catch.hpp>
#include <book_recommender/Book.hpp>

using namespace book_recommender;

TEST_CASE("Book Creation and Basic Properties", "[book]") {
    Book book(
        "1",                    // id
        "Test Book",           // title
        "Test Author",         // author
        {"fiction", "drama"},  // genres
        "Test description",    // description
        200,                   // page_count
        4.5,                   // average_rating
        1000,                  // ratings_count
        500,                   // review_count
        "Test Series",         // series
        "en",                  // language
        "Test Publisher",      // publisher
        "2023-01-01",         // publication_date
        "9781234567890",      // isbn13
        true                   // is_ebook
    );

    SECTION("Basic Properties") {
        REQUIRE(book.getId() == "1");
        REQUIRE(book.getTitle() == "Test Book");
        REQUIRE(book.getAuthor() == "Test Author");
        REQUIRE(book.getDescription() == "Test description");
        REQUIRE(book.getPageCount() == 200);
        REQUIRE(book.getAverageRating() == Approx(4.5));
        REQUIRE(book.getRatingsCount() == 1000);
        REQUIRE(book.getReviewCount() == 500);
        REQUIRE(book.getSeries().value() == "Test Series");
        REQUIRE(book.getLanguage() == "en");
        REQUIRE(book.getPublisher() == "Test Publisher");
        REQUIRE(book.getPublicationDate() == "2023-01-01");
        REQUIRE(book.getIsbn13() == "9781234567890");
        REQUIRE(book.isEbook() == true);
    }

    SECTION("Genres") {
        auto genres = book.getGenres();
        REQUIRE(genres.size() == 2);
        REQUIRE(genres[0] == "fiction");
        REQUIRE(genres[1] == "drama");
    }

    SECTION("Computed Properties") {
        REQUIRE(book.getPublicationYear() == 2023);
        REQUIRE(book.getPopularityScore() > 0);
        
        // Popularity score should be affected by ratings
        Book less_popular_book = book;
        less_popular_book = Book(
            "2", "Less Popular", "Author", {"fiction"}, "desc",
            200, 4.5, 100, 50, "Series", "en", "Pub", "2023-01-01",
            "9781234567890", true
        );
        REQUIRE(book.getPopularityScore() > less_popular_book.getPopularityScore());
    }
}

TEST_CASE("Book JSON Serialization", "[book]") {
    Book original(
        "1", "Test Book", "Test Author", {"fiction"}, "description",
        200, 4.5, 1000, 500, "Series", "en", "Publisher",
        "2023-01-01", "9781234567890", true
    );

    SECTION("Serialization and Deserialization") {
        auto json = original.toJson();
        auto deserialized = Book::fromJson(json);

        REQUIRE(deserialized.getId() == original.getId());
        REQUIRE(deserialized.getTitle() == original.getTitle());
        REQUIRE(deserialized.getAuthor() == original.getAuthor());
        REQUIRE(deserialized.getGenres() == original.getGenres());
        REQUIRE(deserialized.getAverageRating() == original.getAverageRating());
    }
}s