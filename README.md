# Book Recommender

A modern C++ book recommendation system leveraging Groq's LLMs for intelligent semantic search and natural language understanding.

## 🌟 Features

- **Advanced LLM Integration**: Uses Groq's Mixtral model for text understanding and embeddings
- **Smart Recommendations**: Semantic search with natural language queries
- **Intelligent Explanations**: LLM-generated explanations for recommendations
- **Fast Similarity Search**: FAISS-powered vector search
- **Rich Filtering**: Filter by genre, rating, publication year, language, and more
- **Modern C++**: Built with C++17, emphasizing performance and clean design

## 🚀 Quick Start

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- FAISS library
- cpprestsdk
- spdlog
- nlohmann_json
- OpenMP (optional, for performance)
- Groq API key

### Environment Setup

```bash
# Set your Groq API key
export GROQ_API_KEY=your_api_key_here

# Install dependencies (Ubuntu/Debian)
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    libfaiss-dev \
    libcpprest-dev \
    libspdlog-dev \
    nlohmann-json3-dev \
    libssl-dev \
    libomp-dev
```

### Installation

```bash
# Clone the repository
git clone https://github.com/username/book-recommender.git
cd book-recommender

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j8

# Run tests
ctest

# Install
sudo make install
```

### Quick Example

```cpp
#include <book_recommender/BookRecommender.hpp>

int main() {
    // Initialize recommender
    book_recommender::BookRecommender::RecommenderConfig config{
        .data_file = "books.csv",
        .cache_size = 1000
    };
    
    book_recommender::BookRecommender recommender(config);

    // Get recommendations
    auto recommendations = recommender.getRecommendations(
        "Looking for science fiction books that explore artificial intelligence and consciousness",
        {
            .genres = {"science fiction"},
            .min_rating = 4.0
        }
    );

    // Process results
    for (const auto& rec : recommendations) {
        std::cout << "📚 " << rec.book.getTitle() << "\n";
        std::cout << "✍️  " << rec.book.getAuthor() << "\n";
        std::cout << "⭐ " << rec.book.getAverageRating() << "/5.0\n";
        std::cout << "💡 " << rec.explanation << "\n\n";
    }

    return 0;
}
```

## 🔧 Advanced Configuration

### Groq API Configuration

The system uses Groq's Mixtral model for:
- Text embeddings for semantic search
- Query enhancement and understanding
- Natural language explanations

You can configure the Groq client behavior:
```cpp
// In your environment or configuration file
export GROQ_API_KEY=your_api_key
export GROQ_MODEL=mixtral-8x7b-32768  // Default model
export GROQ_API_TIMEOUT=30  // Timeout in seconds
```

### Performance Tuning

```cpp
BookRecommender::RecommenderConfig config{
    .cache_size = 2000,         // Increase cache size
    .use_approximate_search = true,  // Faster but slightly less accurate
    .num_threads = 8            // OpenMP threads
};
```

## 📚 Documentation

### Key Components

1. **BookRecommender**: Main interface
2. **BookQueryEngine**: Query processing with Groq integration
3. **BookVectorStore**: FAISS-based similarity search
4. **GroqClient**: Handles Groq API interactions

### API Documentation

Generate the documentation:
```bash
cd build
cmake .. -DBUILD_DOCS=ON
make doc
```

## 🧪 Testing

```bash
# Build and run tests
cd build
cmake .. -DBUILD_TESTING=ON
make
ctest --output-on-failure
```

## 📝 License

This project is licensed under the MIT License - see [LICENSE](LICENSE).

## 🙏 Acknowledgments

- Groq for their powerful LLM API
- FAISS by Facebook Research
- cpprestsdk by Microsoft
- spdlog by gabime
- JSON for Modern C++ by nlohmann