#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <tbb/concurrent_hash_map.h>
#include <simdjson.h>
#include <immintrin.h>
#include "MemoryPool.h"

struct PriceLevel {
    double price;
    double quantity;
};

struct Orderbook {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

class OrderbookManager {
public:
    OrderbookManager(size_t shard_count = 16);
    void OnOrderbookWs(const std::string& symbol, const simdjson::dom::element& message);
    void OnOrderbookRest(const std::string& symbol, const simdjson::dom::element& message);
    std::string getOrderbookSnapshot(const std::string& symbol, int depth) const;

private:
    struct Shard {
        tbb::concurrent_hash_map<std::string, Orderbook> orderbooks;
        mutable std::mutex mutex;
    };
    std::vector<Shard> shards;

    void updateOrderbook(const std::string& symbol, const std::vector<PriceLevel>& bids, const std::vector<PriceLevel>& asks);
    void updatePriceLevels(std::vector<PriceLevel>& existing, const std::vector<PriceLevel>& updates);
};
