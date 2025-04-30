#include <gtest/gtest.h>
#include "order_book.hpp"
#include <memory>
#include <chrono>

using namespace financial;

class OrderBookTest : public ::testing::Test {
protected:
    void SetUp() override {
        orderBook = createOrderBook("AAPL");
    }

    std::unique_ptr<OrderBook> orderBook;
};

TEST_F(OrderBookTest, AddOrder) {
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

TEST_F(OrderBookTest, CancelOrder) {
    Order order;
    order.orderId = "1";
    order.symbol = "AAPL";
    order.side = Order::Side::BUY;
    order.type = Order::Type::LIMIT;
    order.price = 150.0;
    order.quantity = 100.0;
    order.timestamp = std::chrono::system_clock::now();

    orderBook->addOrder(order);
    orderBook->cancelOrder("1");
    EXPECT_DOUBLE_EQ(orderBook->getBestBid(), 0.0);
}

TEST_F(OrderBookTest, ModifyOrder) {
    Order order;
    order.orderId = "1";
    order.symbol = "AAPL";
    order.side = Order::Side::BUY;
    order.type = Order::Type::LIMIT;
    order.price = 150.0;
    order.quantity = 100.0;
    order.timestamp = std::chrono::system_clock::now();

    orderBook->addOrder(order);
    
    order.price = 155.0;
    orderBook->modifyOrder(order);
    EXPECT_DOUBLE_EQ(orderBook->getBestBid(), 155.0);
}

TEST_F(OrderBookTest, MarketDepth) {
    // 添加多个买单
    for (int i = 0; i < 5; ++i) {
        Order order;
        order.orderId = std::to_string(i);
        order.symbol = "AAPL";
        order.side = Order::Side::BUY;
        order.type = Order::Type::LIMIT;
        order.price = 150.0 - i;
        order.quantity = 100.0;
        order.timestamp = std::chrono::system_clock::now();
        orderBook->addOrder(order);
    }

    auto bids = orderBook->getBids(3);
    EXPECT_EQ(bids.size(), 3);
    EXPECT_DOUBLE_EQ(bids[0].price, 150.0);
    EXPECT_DOUBLE_EQ(bids[1].price, 149.0);
    EXPECT_DOUBLE_EQ(bids[2].price, 148.0);
}

TEST_F(OrderBookTest, SpreadCalculation) {
    // 添加买单
    Order buyOrder;
    buyOrder.orderId = "1";
    buyOrder.symbol = "AAPL";
    buyOrder.side = Order::Side::BUY;
    buyOrder.type = Order::Type::LIMIT;
    buyOrder.price = 150.0;
    buyOrder.quantity = 100.0;
    buyOrder.timestamp = std::chrono::system_clock::now();
    orderBook->addOrder(buyOrder);

    // 添加卖单
    Order sellOrder;
    sellOrder.orderId = "2";
    sellOrder.symbol = "AAPL";
    sellOrder.side = Order::Side::SELL;
    sellOrder.type = Order::Type::LIMIT;
    sellOrder.price = 151.0;
    sellOrder.quantity = 100.0;
    sellOrder.timestamp = std::chrono::system_clock::now();
    orderBook->addOrder(sellOrder);

    EXPECT_DOUBLE_EQ(orderBook->getSpread(), 1.0);
} 