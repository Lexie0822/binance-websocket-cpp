#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include "WebSocketHandler.h"
#include "RestApiHandler.h"
#include "MessageProcessor.h"
#include "OrderbookManager.h"

class BinanceClient {
public:
    BinanceClient(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, int thread_count = 4);
    ~BinanceClient();
    void start(const std::vector<std::string>& symbols);
    void stop();
    void connect(const std::string& host, const std::string& port, const std::string& target);
    
    // 修改这个方法
    MessageProcessor& getMessageProcessor() { return *messageProcessor_; }
    OrderbookManager& getOrderbookManager() { return *orderbookManager_; }

private:
    boost::asio::io_context& ioc_;
    boost::asio::ssl::context& ctx_;
    std::unique_ptr<OrderbookManager> orderbookManager_;
    std::unique_ptr<MessageProcessor> messageProcessor_;
    std::vector<std::thread> threads_;
    std::vector<std::shared_ptr<WebSocketHandler>> wsHandlers_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::shared_ptr<WebSocketHandler> ws_;
};