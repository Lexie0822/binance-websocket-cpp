#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>
#include "xxhash.h"
#include "BloomFilter.h"

class Deduplicator {
public:
    Deduplicator(size_t bloom_filter_size = 100000, size_t lru_cache_size = 1000);
    bool is_duplicate(const std::string& message);

private:
    BloomFilter bloom_filter_;
    std::list<uint64_t> lru_list_;
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> lru_map_;
    size_t lru_cache_size_;
    std::mutex mutex_;

    void add_to_lru_cache(uint64_t hash);
};
