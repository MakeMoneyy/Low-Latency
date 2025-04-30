#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "OrderBook.h"
#include <iomanip>

// 价格更新回调函数
void onPriceUpdate(const std::string& symbol, double price) {
    static std::mutex coutMutex;
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "[" << symbol << "] 收到价格更新: " << std::fixed << std::setprecision(2) << price 
              << " (线程ID: " << std::this_thread::get_id() << ")" << std::endl;
}

// 打印订单信息
void printOrder(const std::shared_ptr<Order>& order) {
    if (!order) {
        std::cout << "订单不存在" << std::endl;
        return;
    }
    
    std::cout << "订单ID: " << order->id << std::endl;
    std::cout << "交易对: " << order->symbol << std::endl;
    std::cout << "订单类型: " << 
        (order->type == OrderType::LIMIT ? "限价单" : 
         order->type == OrderType::MARKET ? "市价单" : "其他") << std::endl;
    std::cout << "买卖方向: " << 
        (order->side == OrderSide::BUY ? "买入" : "卖出") << std::endl;
    std::cout << "价格: " << std::fixed << std::setprecision(2) << order->price << std::endl;
    std::cout << "数量: " << order->quantity << std::endl;
    std::cout << "已成交数量: " << order->filledQuantity << std::endl;
    std::cout << "订单状态: ";
    switch (order->status) {
        case OrderStatus::NEW: std::cout << "新订单"; break;
        case OrderStatus::PARTIALLY_FILLED: std::cout << "部分成交"; break;
        case OrderStatus::FILLED: std::cout << "完全成交"; break;
        case OrderStatus::CANCELLED: std::cout << "已取消"; break;
        case OrderStatus::REJECTED: std::cout << "已拒绝"; break;
    }
    std::cout << std::endl << std::endl;
}

// 打印订单簿快照
void printOrderBook(const OrderBook& book) {
    std::map<double, double> bids, asks;
    book.getOrderBookSnapshot(bids, asks);
    
    std::cout << "=== 订单簿快照 ===" << std::endl;
    std::cout << "最佳买价: " << std::fixed << std::setprecision(2) << book.getBestBid() << std::endl;
    std::cout << "最佳卖价: " << book.getBestAsk() << std::endl;
    std::cout << "买盘深度: " << book.getBidDepth() << std::endl;
    std::cout << "卖盘深度: " << book.getAskDepth() << std::endl;
    
    std::cout << "\n买盘:" << std::endl;
    for (auto it = bids.rbegin(); it != bids.rend(); ++it) {
        std::cout << "价格: " << std::fixed << std::setprecision(2) << it->first 
                  << " 数量: " << it->second << std::endl;
    }
    
    std::cout << "\n卖盘:" << std::endl;
    for (const auto& [price, quantity] : asks) {
        std::cout << "价格: " << std::fixed << std::setprecision(2) << price 
                  << " 数量: " << quantity << std::endl;
    }
    std::cout << "=================" << std::endl << std::endl;
}

int main() {
    setConsoleEncoding();
    std::cout << "开始测试多线程 DataFeed 模块..." << std::endl;
    
    // 创建自定义配置
    DataFeed::Config config;
    config.minPrice = 50.0;        // 最小价格 50
    config.maxPrice = 150.0;       // 最大价格 150
    config.updateIntervalMs = 100; // 每 100ms 更新一次
    config.volatility = 0.01;      // 1% 的波动率
    
    // 使用自定义配置创建 DataFeed 实例
    DataFeed feed(config);
    
    // 创建订单簿
    OrderBook book("BTC/USD");
    
    // 订阅价格更新
    feed.subscribe("BTC/USD", [&book](const std::string& symbol, double price) {
        // 当收到价格更新时，可以在这里添加自动交易逻辑
        std::cout << "收到价格更新，当前价格: " << std::fixed << std::setprecision(2) << price << std::endl;
    });
    
    // 启动数据流
    feed.start();
    
    // 添加一些测试订单
    Order buyOrder;
    buyOrder.symbol = "BTC/USD";
    buyOrder.type = OrderType::LIMIT;
    buyOrder.side = OrderSide::BUY;
    buyOrder.price = 100.0;
    buyOrder.quantity = 1.0;
    OrderID buyId = book.addOrder(buyOrder);
    
    std::cout << "添加买单:" << std::endl;
    printOrder(book.getOrder(buyId));
    printOrderBook(book);
    
    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 添加卖单
    Order sellOrder;
    sellOrder.symbol = "BTC/USD";
    sellOrder.type = OrderType::LIMIT;
    sellOrder.side = OrderSide::SELL;
    sellOrder.price = 99.0;
    sellOrder.quantity = 1.0;
    OrderID sellId = book.addOrder(sellOrder);
    
    std::cout << "添加卖单:" << std::endl;
    printOrder(book.getOrder(sellId));
    printOrderBook(book);
    
    // 等待订单匹配
    std::cout << "等待订单匹配..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 检查订单状态
    std::cout << "匹配后买单状态:" << std::endl;
    printOrder(book.getOrder(buyId));
    
    std::cout << "匹配后卖单状态:" << std::endl;
    printOrder(book.getOrder(sellId));
    printOrderBook(book);
    
    // 继续运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 停止数据流
    feed.stop();
    
    std::cout << "测试完成" << std::endl;

    return 0;
} 