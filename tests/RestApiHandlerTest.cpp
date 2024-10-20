#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../RestApiHandler.h"
#include "../MessageProcessor.h"
#include "../OrderbookManager.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>
#include <chrono>

class MockMessageProcessor : public MessageProcessor {
public:
    MockMessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbook_manager)
        : MessageProcessor(ioc, orderbook_manager) {}
    MOCK_METHOD(void, add_message, (bool, std::string&&));
};

class RestApiHandlerTest : public ::testing::Test {
protected:
    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    std::unique_ptr<OrderbookManager> orderbook_manager;
    std::unique_ptr<MockMessageProcessor> mock_processor;
    std::shared_ptr<RestApiHandler> handler;

    RestApiHandlerTest()
        : orderbook_manager(std::make_unique<OrderbookManager>()),
          mock_processor(std::make_unique<MockMessageProcessor>(ioc, *orderbook_manager))
    {
        handler = std::make_shared<RestApiHandler>(ioc, ctx, "example.com", "443", "/api/v1/depth", *mock_processor);
    }
};

TEST_F(RestApiHandlerTest, StartPolling) {
    EXPECT_CALL(*mock_processor, add_message(testing::_, testing::_)).Times(testing::AtLeast(1));
    handler->start_polling();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    handler->stop();
}

TEST_F(RestApiHandlerTest, StopPolling) {
    handler->start_polling();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    handler->stop();
    EXPECT_FALSE(handler->is_connected());
}

TEST_F(RestApiHandlerTest, AdaptivePollingInterval) {
    handler->start_polling();
    EXPECT_EQ(handler->get_current_polling_interval(), handler->get_max_polling_interval());
    handler->decrease_polling_interval();
    EXPECT_LT(handler->get_current_polling_interval(), handler->get_max_polling_interval());
    handler->stop();
}

TEST_F(RestApiHandlerTest, SetCpuAffinity) {
    handler->set_cpu_affinity(0);
    // Note: We can't easily test the actual CPU affinity, so we just ensure it doesn't crash
    SUCCEED();
}
