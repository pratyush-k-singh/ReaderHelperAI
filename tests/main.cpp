#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    try {
        // Configure logging
        spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
        spdlog::set_level(spdlog::level::info);
        
        // Check for environment variables needed for tests
        if (!std::getenv("GROQ_API_KEY")) {
            spdlog::warn("GROQ_API_KEY environment variable not set. Some tests will be skipped.");
        }

        // Initialize test environment
        spdlog::info("Initializing test environment...");
        
        // Set up temporary test directories if needed
        auto temp_dir = std::filesystem::temp_directory_path() / "book_recommender_test";
        std::filesystem::create_directories(temp_dir);

        // Run the tests
        spdlog::info("Running tests...");
        int result = Catch::Session().run(argc, argv);

        // Cleanup
        spdlog::info("Cleaning up test environment...");
        std::filesystem::remove_all(temp_dir);

        return result;
    } catch (const std::exception& e) {
        spdlog::error("Error in test runner: {}", e.what());
        return 1;
    }
}
