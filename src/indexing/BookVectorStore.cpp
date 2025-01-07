#include "BookVectorStore.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <spdlog/spdlog.h>
#include <faiss/impl/AuxIndexStructures.h>
#include <faiss/index_io.h>

namespace book_recommender {

BookVectorStore::BookVectorStore(int dimension, int cache_size)
    : dimension_(dimension)
    , cache_size_(cache_size)
    , is_trained_(false) {
    initializeFlatIndex();
    initializeIVFIndex();
}

BookVectorStore::~BookVectorStore() = default;

void BookVectorStore::initializeFlatIndex() {
    flat_index_ = std::make_unique<faiss::IndexFlatIP>(dimension_);
}

void BookVectorStore::initializeIVFIndex() {
    faiss::IndexFlatIP* quantizer = new faiss::IndexFlatIP(dimension_);
    ivf_index_ = std::make_unique<faiss::IndexIVFFlat>(
        quantizer, dimension_, 100, faiss::METRIC_INNER_PRODUCT
    );
}

void BookVectorStore::saveIndex(const std::string& path) {
    try {
        // Save FAISS indices
        faiss::write_index(flat_index_.get(), (path + ".flat").c_str());
        faiss::write_index(ivf_index_.get(), (path + ".ivf").c_str());

        // Save document mappings
        std::ofstream mapping_file(path + ".mapping", std::ios::binary);
        size_t doc_count = document_store_.size();
        mapping_file.write(reinterpret_cast<const char*>(&doc_count), sizeof(size_t));

        for (const auto& [id, doc] : document_store_) {
            nlohmann::json j = doc.toJson();
            std::string json_str = j.dump();
            size_t str_len = json_str.length();
            mapping_file.write(reinterpret_cast<const char*>(&str_len), sizeof(size_t));
            mapping_file.write(json_str.c_str(), str_len);
        }

        spdlog::info("Saved index to {}", path);
    } catch (const std::exception& e) {
        spdlog::error("Failed to save index: {}", e.what());
        throw;
    }
}

void BookVectorStore::loadIndex(const std::string& path) {
    try {
        // Load FAISS indices
        flat_index_.reset(faiss::read_index((path + ".flat").c_str()));
        ivf_index_.reset(dynamic_cast<faiss::IndexIVFFlat*>(
            faiss::read_index((path + ".ivf").c_str())
        ));

        // Load document mappings
        std::ifstream mapping_file(path + ".mapping", std::ios::binary);
        size_t doc_count;
        mapping_file.read(reinterpret_cast<char*>(&doc_count), sizeof(size_t));

        document_store_.clear();
        doc_id_to_index_.clear();
        index_to_doc_id_.clear();

        for (size_t i = 0; i < doc_count; ++i) {
            size_t str_len;
            mapping_file.read(reinterpret_cast<char*>(&str_len), sizeof(size_t));
            
            std::string json_str(str_len, '\0');
            mapping_file.read(&json_str[0], str_len);
            
            auto j = nlohmann::json::parse(json_str);
            Document doc = Document::fromJson(j);
            
            updateDocumentMapping(doc.getId(), i);
            document_store_[doc.getId()] = doc;
        }

        is_trained_ = true;
        spdlog::info("Loaded index from {}", path);
    } catch (const std::exception& e) {
        spdlog::error("Failed to load index: {}", e.what());
        throw;
    }
}

std::vector<float> BookVectorStore::getDocumentVector(const Document& doc) const {
    if (!doc.getEmbedding()) {
        throw std::runtime_error("Document does not have an embedding");
    }
    return *doc.getEmbedding();
}

void BookVectorStore::updateDocumentMapping(const std::string& doc_id, size_t index) {
    doc_id_to_index_[doc_id] = index;
    if (index >= index_to_doc_id_.size()) {
        index_to_doc_id_.resize(index + 1);
    }
    index_to_doc_id_[index] = doc_id;
}

std::vector<BookVectorStore::SearchResult> BookVectorStore::processSearchResults(
    const float* distances,
    const faiss::idx_t* indices,
    size_t n_results
) const {
    std::vector<SearchResult> results;
    results.reserve(n_results);

    for (size_t i = 0; i < n_results; ++i) {
        if (indices[i] < 0 || indices[i] >= index_to_doc_id_.size()) {
            continue;
        }

        const auto& doc_id = index_to_doc_id_[indices[i]];
        auto doc_it = document_store_.find(doc_id);
        if (doc_it == document_store_.end()) {
            continue;
        }

        results.push_back({
            doc_id,
            distances[i],
            doc_it->second
        });
    }

    return results;
}

std::string BookVectorStore::generateCacheKey(
    const std::vector<float>& query_vector,
    int top_k
) const {
    std::stringstream ss;
    ss << std::hex;
    for (float v : query_vector) {
        ss << std::hash<float>{}(v);
    }
    ss << top_k;
    return ss.str();
}

void BookVectorStore::addToCache(
    const std::string& key,
    const std::vector<SearchResult>& results
) {
    cleanupCache();
    
    if (search_cache_.size() >= cache_size_) {
        // Remove oldest entry
        auto oldest = std::min_element(
            search_cache_.begin(),
            search_cache_.end(),
            [](const auto& a, const auto& b) {
                return a.second.timestamp < b.second.timestamp;
            }
        );
        search_cache_.erase(oldest);
    }

    search_cache_[key] = {
        results,
        std::chrono::system_clock::now()
    };
}

std::optional<std::vector<BookVectorStore::SearchResult>> BookVectorStore::getFromCache(
    const std::string& key
) const {
    auto it = search_cache_.find(key);
    if (it != search_cache_.end()) {
        return it->second.results;
    }
    return std::nullopt;
}

void BookVectorStore::cleanupCache() {
    auto now = std::chrono::system_clock::now();
    auto it = search_cache_.begin();
    
    while (it != search_cache_.end()) {
        auto age = std::chrono::duration_cast<std::chrono::minutes>(
            now - it->second.timestamp
        ).count();
        
        if (age > 60) {  // Remove entries older than 1 hour
            it = search_cache_.erase(it);
        } else {
            ++it;
        }
    }
}

void BookVectorStore::clearCache() {
    search_cache_.clear();
}

void BookVectorStore::setCacheSize(int size) {
    cache_size_ = size;
    cleanupCache();
}

}