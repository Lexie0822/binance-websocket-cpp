#pragma once

#include <vector>
#include <string>
#include <functional>

class BloomFilter {
public:
    BloomFilter(size_t size, size_t num_hashes)
        : bits_(size), num_hashes_(num_hashes) {}

    void add(const std::string& item) {
        for (size_t i = 0; i < num_hashes_; ++i) {
            size_t index = hash(item, i) % bits_.size();
            bits_[index] = true;
        }
    }

    bool probably_contains(const std::string& item) const {
        for (size_t i = 0; i < num_hashes_; ++i) {
            size_t index = hash(item, i) % bits_.size();
            if (!bits_[index]) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<bool> bits_;
    size_t num_hashes_;

    size_t hash(const std::string& item, size_t seed) const {
        std::hash<std::string> hasher;
        return hasher(item + std::to_string(seed));
    }
};
