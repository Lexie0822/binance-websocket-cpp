#include <gtest/gtest.h>
#include "../BinanceClient.h"

TEST(BinanceClientTest, Initialization) {
    BinanceClient client(4);
    EXPECT_NO_THROW(client.start({"BTCUSDT", "ETHUSDT"}));
}

TEST(BinanceClientTest, AddRemoveSymbol) {
    BinanceClient client(4);
    client.start({"BTCUSDT"});
    
    EXPECT_NO_THROW(client.add_symbol("ETHUSDT"));
    auto symbols = client.get_active_symbols();
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "ETHUSDT") != symbols.end());

    EXPECT_NO_THROW(client.remove_symbol("ETHUSDT"));
    symbols = client.get_active_symbols();
    EXPECT_TRUE(std::find(symbols.begin(), symbols.end(), "ETHUSDT") == symbols.end());
}
