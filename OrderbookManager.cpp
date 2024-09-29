#include "OrderbookManager.h"

void OrderbookManager::updateOrderbook(const std::string& symbol, const std::string& data) {
    std::lock_guard<std::mutex> lock(orderbook_mutex);
    orderbooks[symbol] = data;
}

void OrderbookManager::OnOrderbookWs(const std::string& symbol, const std::string& data) {
    updateOrderbook(symbol, data);
    // Add any WebSocket-specific processing here
}

void OrderbookManager::OnOrderbookRest(const std::string& symbol, const std::string& data) {
    updateOrderbook(symbol, data);
    // Add any REST API-specific processing here
}