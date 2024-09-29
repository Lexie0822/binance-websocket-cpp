#pragma once

#include <boost/asio.hpp>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <functional>
#include "OrderbookManager.h"

class MessageProcessor {
private:
    boost::asio::io_context& ioc;
    std::queue<std::pair<bool, std::string>> messageQueue;
    std::mutex queueMutex;
    std::unordered_set<std::string> processedMessages;
    OrderbookManager& orderbookManager;
    bool running = true;

    void processMessage(bool isWebSocket, const std::string& message);

public:
    MessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbookManager);
    void addMessage(bool isWebSocket, const std::string& message);
    void run();
    void stop();
};