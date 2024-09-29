#include "BinanceClient.h"
#include <iostream>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

BinanceClient::BinanceClient(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, int thread_count) 
    : ioc_(ioc), ctx_(ctx),
      work_guard_(boost::asio::make_work_guard(ioc_)),
      orderbookManager_(std::make_unique<OrderbookManager>()),
      messageProcessor_(std::make_unique<MessageProcessor>(ioc_, *orderbookManager_)) {
    for (int i = 0; i < thread_count; ++i) {
        threads_.emplace_back([this]() { ioc_.run(); });
    }
}

BinanceClient::~BinanceClient() {
    stop();
}

void BinanceClient::start(const std::vector<std::string>& symbols) {
    std::cout << "Starting BinanceClient..." << std::endl;
    for (const auto& symbol : symbols) {
        std::cout << "Creating WebSocketHandler for symbol: " << symbol << std::endl;
        auto handler = std::make_shared<WebSocketHandler>(ioc_, ctx_, symbol, *messageProcessor_);
        wsHandlers_.push_back(handler);
        handler->run("fstream.binance.com", "443");
    }
    std::cout << "All WebSocketHandlers created and running." << std::endl;
}

void BinanceClient::stop() {
    ioc_.stop();
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void BinanceClient::connect(const std::string& host, const std::string& port, const std::string& target) {
    ws_ = std::make_shared<WebSocketHandler>(ioc_, ctx_, target, *messageProcessor_);
    ws_->run(host, port);
}