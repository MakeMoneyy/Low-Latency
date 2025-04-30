#include "TradingEngine.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <iomanip>
#include <vector>

// 辅助函数：打印订单簿快照
void printOrderBook(const OrderBook& book) {
    std::cout << "\n订单簿快照 - " << book.getSymbol() << ":" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // 打印卖盘
    std::cout << "卖盘:" << std::endl;
    auto asks = book.getAsks();
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << std::fixed << std::setprecision(2)
                  << "价格: " << it->first 
                  << " 数量: " << it->second << std::endl;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    
    // 打印买盘
    std::cout << "买盘:" << std::endl;
    auto bids = book.getBids();
    for (const auto& [price, quantity] : bids) {
        std::cout << std::fixed << std::setprecision(2)
                  << "价格: " << price 
                  << " 数量: " << quantity << std::endl;
    }
    
    std::cout << "----------------------------------------" << std::endl;
}

// 测试基本功能
void test_basic_functionality() {
    std::cout << "\n=== 测试基本功能 ===" << std::endl;
    
    TradingEngine engine;
    
    // 设置回调
    engine.setOrderCallback([](const Order& order) {
        std::cout << "订单更新: ID=" << order.id 
                  << " 状态=" << static_cast<int>(order.status) << std::endl;
    });
    
    engine.setTradeCallback([](const Trade& trade) {
        std::cout << "成交: 价格=" << trade.price 
                  << " 数量=" << trade.quantity << std::endl;
    });
    
    engine.setOrderBookCallback([](const OrderBook& book) {
        std::cout << "订单簿更新: " << book.getSymbol() << std::endl;
    });
    
    // 初始化并启动引擎
    engine.initialize();
    engine.start();
    
    // 更新价格
    engine.onPriceUpdate("BTC/USDT", 50000.0);
    
    // 提交买单
    Order buyOrder;
    buyOrder.id = 1;
    buyOrder.symbol = "BTC/USDT";
    buyOrder.type = OrderType::LIMIT;
    buyOrder.side = OrderSide::BUY;
    buyOrder.price = 49900.0;
    buyOrder.quantity = 1.0;
    buyOrder.status = OrderStatus::NEW;
    
    engine.submitOrder(buyOrder);
    
    // 提交卖单
    Order sellOrder;
    sellOrder.id = 2;
    sellOrder.symbol = "BTC/USDT";
    sellOrder.type = OrderType::LIMIT;
    sellOrder.side = OrderSide::SELL;
    sellOrder.price = 50100.0;
    sellOrder.quantity = 1.0;
    sellOrder.status = OrderStatus::NEW;
    
    engine.submitOrder(sellOrder);
    
    // 显示订单簿快照
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    // 显示最佳买卖价
    std::cout << "最佳买价: " << engine.getBestBid("BTC/USDT") << std::endl;
    std::cout << "最佳卖价: " << engine.getBestAsk("BTC/USDT") << std::endl;
    
    // 取消订单
    engine.cancelOrder(1);
    
    // 再次显示订单簿
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    engine.stop();
}

// 测试订单匹配
void test_order_matching() {
    std::cout << "\n=== 测试订单匹配 ===" << std::endl;
    
    TradingEngine engine;
    engine.initialize();
    engine.start();
    
    // 提交多个买单
    std::vector<Order> buyOrders;
    Order buy1{1, "BTC/USDT", OrderType::LIMIT, OrderSide::BUY, 50000.0, 1.0, 0.0, OrderStatus::NEW, 0};
    Order buy2{2, "BTC/USDT", OrderType::LIMIT, OrderSide::BUY, 49900.0, 2.0, 0.0, OrderStatus::NEW, 0};
    Order buy3{3, "BTC/USDT", OrderType::LIMIT, OrderSide::BUY, 49800.0, 1.5, 0.0, OrderStatus::NEW, 0};
    buyOrders.push_back(buy1);
    buyOrders.push_back(buy2);
    buyOrders.push_back(buy3);
    
    // 提交多个卖单
    std::vector<Order> sellOrders;
    Order sell1{4, "BTC/USDT", OrderType::LIMIT, OrderSide::SELL, 50100.0, 1.0, 0.0, OrderStatus::NEW, 0};
    Order sell2{5, "BTC/USDT", OrderType::LIMIT, OrderSide::SELL, 50200.0, 2.0, 0.0, OrderStatus::NEW, 0};
    Order sell3{6, "BTC/USDT", OrderType::LIMIT, OrderSide::SELL, 50300.0, 1.5, 0.0, OrderStatus::NEW, 0};
    sellOrders.push_back(sell1);
    sellOrders.push_back(sell2);
    sellOrders.push_back(sell3);
    
    // 提交所有订单
    for (const auto& order : buyOrders) {
        engine.submitOrder(order);
    }
    
    for (const auto& order : sellOrders) {
        engine.submitOrder(order);
    }
    
    // 显示初始订单簿状态
    std::cout << "\n初始订单簿状态:" << std::endl;
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    // 提交一个可以匹配的市价单
    Order marketOrder;
    marketOrder.id = 7;
    marketOrder.symbol = "BTC/USDT";
    marketOrder.type = OrderType::MARKET;
    marketOrder.side = OrderSide::BUY;
    marketOrder.quantity = 2.0;
    marketOrder.status = OrderStatus::NEW;
    
    engine.submitOrder(marketOrder);
    
    // 显示最终订单簿状态
    std::cout << "\n最终订单簿状态:" << std::endl;
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    engine.stop();
}

// 测试价格更新和订单匹配
void test_price_update() {
    std::cout << "\n=== 测试价格更新和订单匹配 ===" << std::endl;
    
    TradingEngine engine;
    engine.initialize();
    engine.start();
    
    // 设置回调
    engine.setOrderCallback([](const Order& order) {
        std::cout << "订单更新: ID=" << order.id 
                  << " 状态=" << static_cast<int>(order.status) << std::endl;
    });
    
    engine.setTradeCallback([](const Trade& trade) {
        std::cout << "成交: 价格=" << trade.price 
                  << " 数量=" << trade.quantity << std::endl;
    });
    
    engine.setOrderBookCallback([](const OrderBook& book) {
        std::cout << "订单簿更新: " << book.getSymbol() << std::endl;
    });
    
    // 提交买单
    Order buyOrder;
    buyOrder.id = 1;
    buyOrder.symbol = "BTC/USDT";
    buyOrder.type = OrderType::LIMIT;
    buyOrder.side = OrderSide::BUY;
    buyOrder.price = 50000.0;
    buyOrder.quantity = 1.0;
    buyOrder.status = OrderStatus::NEW;
    
    engine.submitOrder(buyOrder);
    
    // 提交卖单
    Order sellOrder;
    sellOrder.id = 2;
    sellOrder.symbol = "BTC/USDT";
    sellOrder.type = OrderType::LIMIT;
    sellOrder.side = OrderSide::SELL;
    sellOrder.price = 50100.0;
    sellOrder.quantity = 1.0;
    sellOrder.status = OrderStatus::NEW;
    
    engine.submitOrder(sellOrder);
    
    // 显示初始订单簿状态
    std::cout << "\n初始订单簿状态:" << std::endl;
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    // 更新价格，触发订单匹配
    std::cout << "\n更新价格到 50000.0..." << std::endl;
    engine.onPriceUpdate("BTC/USDT", 50000.0);
    
    // 显示最终订单簿状态
    std::cout << "\n最终订单簿状态:" << std::endl;
    printOrderBook(engine.getOrderBookSnapshot("BTC/USDT"));
    
    engine.stop();
}

int main() {
    setConsoleEncoding();
    test_basic_functionality();
    test_order_matching();
    test_price_update();
    return 0;
} 