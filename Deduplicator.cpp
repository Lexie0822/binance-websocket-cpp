#include "Deduplicator.h"

Deduplicator::Deduplicator(size_t bloom_filter_size, size_t lru_cache_size)
    : bloom_filter_(bloom_filter_size, 5), lru_cache_size_(lru_cache_size) {}

bool Deduplicator::is_duplicate(const std::string& message) {
    uint64_t hash = XXH64(message.data(), message.size(), 0);
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (bloom_filter_.probably_contains(std::to_string(hash))) {
        if (lru_map_.find(hash) != lru_map_.end()) {
            return true;
        }
    }
    
    bloom_filter_.add(std::to_string(hash));
    add_to_lru_cache(hash);
    return false;
}

void Deduplicator::add_to_lru_cache(uint64_t hash) {
    if (lru_list_.size() >= lru_cache_size_) {
        uint64_t oldest = lru_list_.back();
        lru_list_.pop_back();
        lru_map_.erase(oldest);
    }
    
    lru_list_.push_front(hash);
    lru_map_[hash] = lru_list_.begin();
}
