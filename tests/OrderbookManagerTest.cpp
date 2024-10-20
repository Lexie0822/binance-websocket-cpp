#include <gtest/gtest.h>
#include "../OrderbookManager.h"
#include <simdjson.h>

class OrderbookManagerTest : public ::testing::Test {
protected:
    OrderbookManager manager;
};

TEST_F(OrderbookManagerTest, UpdateOrderbook) {
    // 创建一个模拟的 JSON 字符串
    std::string json = R"({
        "bids": [["10000.00", "1.00000000"], ["9999.99", "1.00000000"]],
        "asks": [["10000.01", "1.00000000"], ["10000.02", "1.00000000"]]
    })";

    // 使用 simdjson 解析 JSON
    simdjson::dom::parser parser;
    simdjson::dom::element doc = parser.parse(json);

    // 调用 OnOrderbookWs 方法
    EXPECT_NO_THROW(manager.OnOrderbookWs("BTCUSDT", doc));

    // 获取订单簿快照并验证
    std::string snapshot = manager.getOrderbookSnapshot("BTCUSDT", 2);
    EXPECT_NE(snapshot.find("\"bids\":[[\"10000.00\",\"1.00000000\"],[\"9999.99\",\"1.00000000\"]]"), std::string::npos);
    EXPECT_NE(snapshot.find("\"asks\":[[\"10000.01\",\"1.00000000\"],[\"10000.02\",\"1.00000000\"]]"), std::string::npos);
}

TEST_F(OrderbookManagerTest, EmptyOrderbook) {
    std::string snapshot = manager.getOrderbookSnapshot("ETHUSDT", 2);
    EXPECT_EQ(snapshot, "{}");
}
