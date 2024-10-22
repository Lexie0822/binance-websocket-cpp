#include <gtest/gtest.h>
#include "../OrderbookManager.h"
#include <simdjson.h>

class OrderbookManagerTest : public ::testing::Test {
protected:
    OrderbookManager manager;
};

TEST_F(OrderbookManagerTest, UpdateOrderbook) {
    // Create a mock JSON string
    std::string json = R"({
        "bids": [["10000.00", "1.00000000"], ["9999.99", "1.00000000"]],
        "asks": [["10000.01", "1.00000000"], ["10000.02", "1.00000000"]]
    })";

    // Parse JSON using simdjson
    simdjson::dom::parser parser;
    simdjson::dom::element doc = parser.parse(json);

    // Call OnOrderbookWs method
    EXPECT_NO_THROW(manager.OnOrderbookWs("BTCUSDT", doc));

    // Get order book snapshot and verify
    std::string snapshot = manager.getOrderbookSnapshot("BTCUSDT", 2);
    EXPECT_NE(snapshot.find("\"bids\":[[\"10000.00\",\"1.00000000\"],[\"9999.99\",\"1.00000000\"]]"), std::string::npos);
    EXPECT_NE(snapshot.find("\"asks\":[[\"10000.01\",\"1.00000000\"],[\"10000.02\",\"1.00000000\"]]"), std::string::npos);
}

TEST_F(OrderbookManagerTest, EmptyOrderbook) {
    std::string snapshot = manager.getOrderbookSnapshot("ETHUSDT", 2);
    EXPECT_EQ(snapshot, "{}");
}
