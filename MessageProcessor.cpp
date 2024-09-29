#include "MessageProcessor.h"
#include "OrderbookManager.h"
#include <iostream>
#include <chrono>

MessageProcessor::MessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbookManager) 
    : ioc(ioc), orderbookManager(orderbookManager) {
    std::cout << "MessageProcessor initialized" << std::endl;
}

void MessageProcessor::addMessage(bool isWebSocket, const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    messageQueue.push({isWebSocket, message});
    std::cout << "Message added to queue. Queue size: " << messageQueue.size() << std::endl;
}

void MessageProcessor::run() {
    std::cout << "MessageProcessor::run() started" << std::endl;
    while (running) {
        std::pair<bool, std::string> message;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!messageQueue.empty()) {
                message = messageQueue.front();
                messageQueue.pop();
                std::cout << "Message popped from queue. Remaining messages: " << messageQueue.size() << std::endl;
            }
        }
        if (!message.second.empty()) {
            if (processedMessages.find(message.second) == processedMessages.end()) {
                processedMessages.insert(message.second);
                processMessage(message.first, message.second);
            } else {
                std::cout << "Duplicate message skipped" << std::endl;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    std::cout << "MessageProcessor::run() stopped" << std::endl;
}

void MessageProcessor::processMessage(bool isWebSocket, const std::string& message) {
    std::cout << "Processing " << (isWebSocket ? "WebSocket" : "REST API") << " message: " << message.substr(0, 50) << "..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    if (isWebSocket) {
        orderbookManager.OnOrderbookWs("", message);
    } else {
        orderbookManager.OnOrderbookRest("", message);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    
    std::cout << "Message processed in " << elapsed.count() << " ms" << std::endl;
}

void MessageProcessor::stop() {
    running = false;
}