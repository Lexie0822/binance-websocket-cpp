#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../WebSocketHandler.h"
#include "../MessageProcessor.h"
#include "../OrderbookManager.h"

class MockMessageProcessor : public MessageProcessor {
public:
    MockMessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbook_manager)
        : MessageProcessor(ioc, orderbook_manager) {}
    MOCK_METHOD(void, add_message, (bool, std::string&&));
};

class WebSocketHandlerTest : public ::testing::Test {
protected:
    boost::asio::io_context ioc;
    std::unique_ptr<OrderbookManager> orderbook_manager;
    std::unique_ptr<MockMessageProcessor> mock_processor;
    std::shared_ptr<WebSocketHandler> handler;

    WebSocketHandlerTest()
        : orderbook_manager(std::make_unique<OrderbookManager>()),
          mock_processor(std::make_unique<MockMessageProcessor>(ioc, *orderbook_manager))
    {
        handler = std::make_shared<WebSocketHandler>(ioc, "BTCUSDT", *mock_processor);
    }
};

TEST_F(WebSocketHandlerTest, ConnectAndDisconnect) {
    EXPECT_CALL(*mock_processor, add_message(testing::_, testing::_)).Times(testing::AtLeast(0));
    
    handler->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(handler->is_connected());
    
    handler->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(handler->is_connected());
}

TEST_F(WebSocketHandlerTest, SetCpuAffinity) {
    handler->set_cpu_affinity(0);
    // Note: We can't easily test the actual CPU affinity, so we just ensure it doesn't crash
    SUCCEED();
}
