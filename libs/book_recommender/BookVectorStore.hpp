#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>
#include <faiss/utils/distances.h>
#include "../models/Document.hpp"

namespace book_recommender {

class BookVectorStore {
public:
    struct SearchResult {
        std::string doc_id;
        float similarity;
        Document document;
    };

    BookVectorStore(int dimension = 384, int cache_size = 1000);
    ~BookVectorStore();

    // Index operations
    void initializeIndex(const std::vector<Document>& documents = {});
    void addDocuments(const std::vector<Document>& documents);
    void removeDocument(const std::string& doc_id);
    void clearIndex();

    // Search operations
    std::vector<SearchResult> search(const std::vector<float>& query_vector, int top_k = 5, bool use_approximate = false);
    std::vector<SearchResult> searchSimilar(const std::string& doc_id, int top_k = 5);
    
    // Batch operations
    void batchAddDocuments(const std::vector<Document>& documents, int batch_size = 100);
    std::vector<std::vector<SearchResult>> batchSearch(
        const std::vector<std::vector<float>>& query_vectors,
        int top_k = 5
    );

    // Index management
    void optimizeIndex();
    void saveIndex(const std::string& path);
    void loadIndex(const std::string& path);
    
    // Cache management
    void clearCache();
    void setCacheSize(int size);

private:
    // Index configuration
    int dimension_;
    int cache_size_;
    bool is_trained_;

    // FAISS indices
    std::unique_ptr<faiss::IndexFlatIP> flat_index_;
    std::unique_ptr<faiss::IndexIVFFlat> ivf_index_;
    
    // Document storage
    std::unordered_map<std::string, Document> document_store_;
    std::unordered_map<std::string, size_t> doc_id_to_index_;
    std::vector<std::string> index_to_doc_id_;

    // Cache for search results
    struct CacheEntry {
        std::vector<SearchResult> results;
        std::chrono::system_clock::time_point timestamp;
    };
    std::unordered_map<std::string, CacheEntry> search_cache_;

    // Helper methods
    void initializeFlatIndex();
    void initializeIVFIndex();
    std::vector<float> getDocumentVector(const Document& doc) const;
    void updateDocumentMapping(const std::string& doc_id, size_t index);
    std::vector<SearchResult> processSearchResults(
        const float* distances,
        const faiss::idx_t* indices,
        size_t n_results
    ) const;
    
    // Cache helpers
    std::string generateCacheKey(const std::vector<float>& query_vector, int top_k) const;
    void addToCache(const std::string& key, const std::vector<SearchResult>& results);
    std::optional<std::vector<SearchResult>> getFromCache(const std::string& key) const;
    void cleanupCache();
};

}