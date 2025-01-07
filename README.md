# Book Recommender

A modern C++ library for building intelligent book recommendation systems using semantic search and machine learning.

## 🌟 Features

- **Smart Recommendations**: Uses advanced semantic search to understand book themes and reader preferences
- **Flexible Filtering**: Filter by genre, rating, publication year, language, and more
- **Similar Book Discovery**: Find books similar to your favorites
- **Author Recommendations**: Explore other works by authors you enjoy
- **Series Support**: Track and recommend book series
- **Rich Metadata**: Comprehensive book information including ratings, reviews, and publication details
- **Fast Search**: Efficient vector similarity search using FAISS
- **Modern C++**: Built with C++17, emphasizing modern practices and performance

## 🚀 Quick Start

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- FAISS library
- LibTorch
- spdlog
- nlohmann_json
- Python 3.7+ (for model conversion)
- PyTorch and Transformers (for model conversion)

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/username/book-recommender.git
cd book-recommender
```

2. **Set up the model**
```bash
# Install Python dependencies
pip install torch transformers

# Convert the model
cd models
python convert_model.py
cd ..
```

3. **Build the project**
```bash
mkdir build && cd build
cmake ..
make -j8

# Run tests
ctest

# Install
sudo make install
```

### Model Setup Details

The book recommender uses the BGE (BAAI General Embeddings) model for text embedding. The `convert_model.py` script will:

1. Download the pre-trained model from Hugging Face
2. Convert it to TorchScript format
3. Save the model and tokenizer in the correct format
4. Verify the conversion

If you prefer to set up the model manually:

```bash
# Create the models directory
mkdir -p models/BAAI/bge-small-en-v1.5

# Download directly from Hugging Face
wget https://huggingface.co/BAAI/bge-small-en-v1.5/resolve/main/model.safetensors
wget https://huggingface.co/BAAI/bge-small-en-v1.5/resolve/main/tokenizer.json

# Then convert using the provided script
python models/convert_model.py
```

The model files should be structured as:
```
models/
└── BAAI/
    └── bge-small-en-v1.5/
        ├── model.pt         # The converted TorchScript model
        └── tokenizer.json   # The tokenizer configuration
```

### Basic Usage

```cpp
#include <book_recommender/BookRecommender.hpp>

int main() {
    // Initialize recommender
    book_recommender::BookRecommender::RecommenderConfig config{
        .data_file = "books.csv",
        .embedding_dimension = 384,
        .cache_size = 1000
    };
    
    book_recommender::BookRecommender recommender(config);

    // Get recommendations
    auto recommendations = recommender.getRecommendations(
        "fantasy books with magic and adventure",
        {
            .genres = {"fantasy"},
            .min_rating = 4.0
        }
    );

    // Process results
    for (const auto& rec : recommendations) {
        std::cout << "Title: " << rec.book.getTitle() << "\n";
        std::cout << "Author: " << rec.book.getAuthor() << "\n";
        std::cout << "Rating: " << rec.book.getAverageRating() << "/5.0\n";
        std::cout << "Why recommended: " << rec.explanation << "\n\n";
    }

    return 0;
}
```

## 📚 Documentation

### Key Components

1. **BookRecommender**: Main interface for recommendation functionality
2. **BookQueryEngine**: Handles query processing and recommendation logic
3. **BookVectorStore**: Manages vector similarity search using FAISS
4. **BookDataLoader**: Handles data loading and preprocessing
5. **Book**: Core data structure for book information

### Common Tasks

#### Getting Recommendations
```cpp
// Get basic recommendations
auto recommendations = recommender.getRecommendations(
    "science fiction with AI themes"
);

// Get recommendations with filters
BookQueryEngine::QueryFilter filter{
    .genres = {"science fiction"},
    .min_rating = 4.0,
    .min_ratings_count = 1000,
    .publication_year_start = 2000
};
auto filtered_recommendations = recommender.getRecommendations(query, filter);
```

#### Finding Similar Books
```cpp
// Find similar books
auto similar_books = recommender.getSimilarBooks("book_id");
```

#### Author Recommendations
```cpp
// Get author recommendations
auto author_books = recommender.getAuthorRecommendations("Author Name");
```

## 🛠️ Building from Source

### Dependencies

1. **FAISS**
   ```bash
   sudo apt-get install libfaiss-dev
   ```

2. **LibTorch**
   ```bash
   wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.9.0%2Bcpu.zip
   unzip libtorch-cxx11-abi-shared-with-deps-1.9.0+cpu.zip
   ```

3. **Other Dependencies**
   ```bash
   sudo apt-get install libspdlog-dev nlohmann-json3-dev
   ```

4. **Python Dependencies** (for model conversion)
   ```bash
   pip install torch transformers
   ```

### Building

```bash
# Setup model
cd models
python convert_model.py
cd ..

# Build project
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/libtorch
make -j8
```

### Running Tests

```bash
cd build
cmake .. -DBUILD_TESTING=ON
make
ctest --output-on-failure
```

## 🤝 Contributing

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- FAISS by Facebook Research
- LibTorch by PyTorch
- BGE Model by BAAI
- spdlog by gabime
- JSON for Modern C++ by nlohmann