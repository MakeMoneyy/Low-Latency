#include <gtest/gtest.h>
#include "market_data.hpp"
#include <chrono>

using namespace financial;

TEST(MarketDataTest, BasicInitialization) {
    MarketData data;
    data.symbol = "AAPL";
    data.price = 150.25;
    data.volume = 1000.0;
    data.timestamp = std::chrono::system_clock::now();

    EXPECT_EQ(data.symbol, "AAPL");
    EXPECT_DOUBLE_EQ(data.price, 150.25);
    EXPECT_DOUBLE_EQ(data.volume, 1000.0);
    EXPECT_NE(data.timestamp, std::chrono::system_clock::time_point());
}

TEST(MarketDataTest, TimestampComparison) {
    MarketData data1, data2;
    data1.timestamp = std::chrono::system_clock::now();
    data2.timestamp = data1.timestamp + std::chrono::seconds(1);

    EXPECT_LT(data1.timestamp, data2.timestamp);
}

TEST(MarketDataTest, PriceValidation) {
    MarketData data;
    data.price = -1.0;
    EXPECT_LT(data.price, 0.0); // 负价格应该被检测到

    data.price = 0.0;
    EXPECT_DOUBLE_EQ(data.price, 0.0);

    data.price = 1000.0;
    EXPECT_GT(data.price, 0.0);
}

TEST(MarketDataTest, VolumeValidation) {
    MarketData data;
    data.volume = -100.0;
    EXPECT_LT(data.volume, 0.0); // 负交易量应该被检测到

    data.volume = 0.0;
    EXPECT_DOUBLE_EQ(data.volume, 0.0);

    data.volume = 1000.0;
    EXPECT_GT(data.volume, 0.0);
} 