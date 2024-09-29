#pragma once

#include <string>
#include <map>
#include <mutex>
#include <functional>

class OrderbookManager {
private:
    std::map<std::string, std::string> orderbooks;
    std::mutex orderbook_mutex;

public:
    void updateOrderbook(const std::string& symbol, const std::string& data);
    void OnOrderbookWs(const std::string& symbol, const std::string& data);
    void OnOrderbookRest(const std::string& symbol, const std::string& data);
};