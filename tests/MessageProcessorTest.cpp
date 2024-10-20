#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../MessageProcessor.h"
#include "../OrderbookManager.h"
#include <boost/asio.hpp>

class MockOrderbookManager : public OrderbookManager {
public:
    MockOrderbookManager() : OrderbookManager() {}
    MOCK_METHOD(void, OnOrderbookWs, (const std::string&, const simdjson::dom::element&));
    MOCK_METHOD(void, OnOrderbookRest, (const std::string&, const simdjson::dom::element&));
};

TEST(MessageProcessorTest, ProcessMessage) {
    boost::asio::io_context ioc;
    MockOrderbookManager mock_orderbook_manager;
    MessageProcessor processor(ioc, mock_orderbook_manager);

    EXPECT_CALL(mock_orderbook_manager, OnOrderbookWs(testing::_, testing::_)).Times(1);
    EXPECT_CALL(mock_orderbook_manager, OnOrderbookRest(testing::_, testing::_)).Times(1);

    processor.add_message(true, R"({"type": "websocket", "data": "test"})");
    processor.add_message(false, R"({"type": "rest", "data": "test"})");

    processor.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Give some time for processing
    processor.stop();
}
