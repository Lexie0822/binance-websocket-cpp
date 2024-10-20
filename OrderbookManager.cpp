#include "OrderbookManager.h"
#include <immintrin.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <simdjson.h>

OrderbookManager::OrderbookManager(size_t shard_count) : shards(shard_count) {}

void OrderbookManager::updateOrderbook(const std::string& symbol, const std::vector<PriceLevel>& bids, const std::vector<PriceLevel>& asks) {
    size_t shard_index = std::hash<std::string>{}(symbol) % shards.size();
    auto& shard = shards[shard_index];
    std::lock_guard<std::mutex> lock(shard.mutex);
    
    tbb::concurrent_hash_map<std::string, Orderbook>::accessor acc;
    if (shard.orderbooks.insert(acc, symbol)) {
        acc->second.bids = bids;
        acc->second.asks = asks;
    } else {
        updatePriceLevels(acc->second.bids, bids);
        updatePriceLevels(acc->second.asks, asks);
    }
    
    std::sort(acc->second.bids.begin(), acc->second.bids.end(), 
              [](const PriceLevel& a, const PriceLevel& b) { return a.price > b.price; });
    std::sort(acc->second.asks.begin(), acc->second.asks.end(), 
              [](const PriceLevel& a, const PriceLevel& b) { return a.price < b.price; });
}

void OrderbookManager::updatePriceLevels(std::vector<PriceLevel>& existing, const std::vector<PriceLevel>& updates) {
    const int vectorSize = 4; // AVX2 supports 4 doubles per vector
    const int numVectors = updates.size() / vectorSize;

    for (int i = 0; i < numVectors; ++i) {
        __m256d updatePrices = _mm256_loadu_pd(&updates[i * vectorSize].price);
        __m256d updateQuantities = _mm256_loadu_pd(&updates[i * vectorSize].quantity);

        for (auto& level : existing) {
            __m256d existingPrice = _mm256_set1_pd(level.price);
            __m256d mask = _mm256_cmp_pd(existingPrice, updatePrices, _CMP_EQ_OQ);

            if (_mm256_movemask_pd(mask)) {
                int index = __builtin_ctz(_mm256_movemask_pd(mask));
                level.quantity = updates[i * vectorSize + index].quantity;
            }
        }

        // Handle new price levels
        __m256d zeroMask = _mm256_cmp_pd(updateQuantities, _mm256_setzero_pd(), _CMP_NEQ_OQ);
        int newLevelsMask = _mm256_movemask_pd(zeroMask);

        while (newLevelsMask) {
            int index = __builtin_ctz(newLevelsMask);
            existing.push_back(updates[i * vectorSize + index]);
            newLevelsMask &= newLevelsMask - 1; // Clear the least significant bit
        }
    }

    // Handle remaining updates
    for (size_t i = numVectors * vectorSize; i < updates.size(); ++i) {
        auto it = std::find_if(existing.begin(), existing.end(),
                               [&](const PriceLevel& level) { return level.price == updates[i].price; });
        
        if (it != existing.end()) {
            it->quantity = updates[i].quantity;
        } else if (updates[i].quantity != 0.0) {
            existing.push_back(updates[i]);
        }
    }
    
    // Remove price levels with zero quantity
    existing.erase(std::remove_if(existing.begin(), existing.end(),
                                  [](const PriceLevel& level) { return level.quantity == 0.0; }),
                   existing.end());
}

void OrderbookManager::OnOrderbookWs(const std::string& symbol, const simdjson::dom::element& message) {
    std::vector<PriceLevel> bids, asks;
    
    simdjson::dom::array bids_array;
    simdjson::dom::array asks_array;
    
    auto error = message["bids"].get(bids_array);
    if (!error) {
        for (auto bid : bids_array) {
            simdjson::dom::array bid_array;
            if (!bid.get(bid_array) && bid_array.size() >= 2) {
                double price, quantity;
                if (!bid_array.at(0).get(price) && !bid_array.at(1).get(quantity)) {
                    bids.push_back({price, quantity});
                }
            }
        }
    }
    
    error = message["asks"].get(asks_array);
    if (!error) {
        for (auto ask : asks_array) {
            simdjson::dom::array ask_array;
            if (!ask.get(ask_array) && ask_array.size() >= 2) {
                double price, quantity;
                if (!ask_array.at(0).get(price) && !ask_array.at(1).get(quantity)) {
                    asks.push_back({price, quantity});
                }
            }
        }
    }
    
    updateOrderbook(symbol, bids, asks);
}

void OrderbookManager::OnOrderbookRest(const std::string& symbol, const simdjson::dom::element& message) {
    OnOrderbookWs(symbol, message);
}

std::string OrderbookManager::getOrderbookSnapshot(const std::string& symbol, int depth) const {
    size_t shard_index = std::hash<std::string>{}(symbol) % shards.size();
    const auto& shard = shards[shard_index];
    
    tbb::concurrent_hash_map<std::string, Orderbook>::const_accessor acc;
    if (!shard.orderbooks.find(acc, symbol)) {
        return "{}";
    }
    
    const auto& orderbook = acc->second;
    std::stringstream ss;
    ss << "{\"bids\":[";
    
    for (int i = 0; i < std::min(depth, static_cast<int>(orderbook.bids.size())); ++i) {
        if (i > 0) ss << ",";
        ss << "[\"" << orderbook.bids[i].price << "\",\"" << orderbook.bids[i].quantity << "\"]";
    }
    
    ss << "],\"asks\":[";
    
    for (int i = 0; i < std::min(depth, static_cast<int>(orderbook.asks.size())); ++i) {
        if (i > 0) ss << ",";
        ss << "[\"" << orderbook.asks[i].price << "\",\"" << orderbook.asks[i].quantity << "\"]";
    }
    
    ss << "]}";
    
    return ss.str();
}
