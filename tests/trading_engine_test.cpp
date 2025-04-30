#include <gtest/gtest.h>
#include "trading_engine.hpp"
#include "order_book.hpp"
#include <memory>
#include <chrono>

using namespace financial;

class TradingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = createTradingEngine();
        engine->initialize();
    }

    std::unique_ptr<TradingEngine> engine;
};

TEST_F(TradingEngineTest, BasicInitialization) {
    EXPECT_FALSE(engine->isRunning());
    engine->start();
    EXPECT_TRUE(engine->isRunning());
    engine->stop();
    EXPECT_FALSE(engine->isRunning());
}

TEST_F(TradingEngineTest, OrderSubmission) {
    engine->start();
    
    bool orderCallbackCalled = false;
    Order order;
    order.orderId = "1";
    order.symbol = "AAPL";
    order.side = Order::Side::BUY;
    order.type = Order::Type::LIMIT;
    order.price = 150.0;
    order.quantity = 100.0;
    order.timestamp = std::chrono::system_clock::now();

    engine->submitOrder(order, [&](const Order&, bool success) {
        orderCallbackCalled = true;
        EXPECT_TRUE(success);
    });

    EXPECT_TRUE(orderCallbackCalled);
}

TEST_F(TradingEngineTest, MarketDataCallback) {
    engine->start();
    
    bool marketDataCallbackCalled = false;
    engine->registerMarketDataCallback([&](const MarketData& data) {
        marketDataCallbackCalled = true;
        EXPECT_EQ(data.symbol, "AAPL");
    });

    // 模拟市场数据更新
    MarketData data;
    data.symbol = "AAPL";
    data.price = 150.0;
    data.volume = 100.0;
    data.timestamp = std::chrono::system_clock::now();

    engine->onMarketData(data);
    
    EXPECT_TRUE(marketDataCallbackCalled);
}

TEST_F(TradingEngineTest, OrderBookAccess) {
    engine->start();
    
    auto orderBook = engine->getOrderBook("AAPL");
    EXPECT_NE(orderBook, nullptr);
    
    // 测试订单簿操作
    Order order;
    order.orderId = "1";
    order.symbol = "AAPL";
    order.side = Order::Side::BUY;
    order.type = Order::Type::LIMIT;
    order.price = 150.0;
    order.quantity = 100.0;
    order.timestamp = std::chrono::system_clock::now();

    orderBook->addOrder(order);
    EXPECT_DOUBLE_EQ(orderBook->getBestBid(), 150.0);
}

TEST_F(TradingEngineTest, TradingStats) {
    engine->start();
    
    auto stats = engine->getTradingStats();
    EXPECT_EQ(stats.totalOrders, 0);
    EXPECT_EQ(stats.filledOrders, 0);
    EXPECT_EQ(stats.cancelledOrders, 0);
    EXPECT_DOUBLE_EQ(stats.totalVolume, 0.0);
    EXPECT_DOUBLE_EQ(stats.totalValue, 0.0);
} 