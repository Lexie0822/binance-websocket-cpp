#include "BinanceClient.h"
#include "MessageProcessor.h"
#include "OrderbookManager.h"
#include <iostream>
#include <thread>
#include <boost/asio/ssl.hpp>

int main() {
    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);

    std::cout << "Initializing BinanceClient..." << std::endl;
    BinanceClient client(ioc, ctx);  // 使用默认的线程数

    // 获取 MessageProcessor 的引用
    MessageProcessor& messageProcessor = client.getMessageProcessor();

    // 添加测试消息
    messageProcessor.addMessage(true, "Test WebSocket message");
    messageProcessor.addMessage(false, "Test REST API message");

    std::vector<std::string> symbols = {"btcusdt", "ethusdt"};
    std::cout << "Starting BinanceClient..." << std::endl;
    client.start(symbols);

    std::cout << "BinanceClient started. Press Enter to exit..." << std::endl;
    std::cin.get();

    std::cout << "Stopping BinanceClient..." << std::endl;
    client.stop();

    std::cout << "BinanceClient stopped." << std::endl;
    return 0;
}