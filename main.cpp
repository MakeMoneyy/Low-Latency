#include "DataFeed.h"
#include "OrderBook.h"
#include "TradingEngine.h"
#include "RiskManagement.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

// 打印订单簿快照
void printOrderBook(const OrderBook& book) {
    std::cout << "\n=== 订单簿快照 - " << book.getSymbol() << " ===" << std::endl;
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

// 打印订单信息
void printOrder(const Order& order) {
    std::cout << "\n=== 订单信息 ===" << std::endl;
    std::cout << "ID: " << order.id << std::endl;
    std::cout << "交易对: " << order.symbol << std::endl;
    std::cout << "类型: " << (order.type == OrderType::LIMIT ? "限价单" : "市价单") << std::endl;
    std::cout << "方向: " << (order.side == OrderSide::BUY ? "买入" : "卖出") << std::endl;
    std::cout << "价格: " << std::fixed << std::setprecision(2) << order.price << std::endl;
    std::cout << "数量: " << order.quantity << std::endl;
    std::cout << "状态: " << static_cast<int>(order.status) << std::endl;
}

// 打印成交信息
void printTrade(const Trade& trade) {
    std::cout << "\n=== 成交信息 ===" << std::endl;
    std::cout << "交易对: " << trade.symbol << std::endl;
    std::cout << "价格: " << std::fixed << std::setprecision(2) << trade.price << std::endl;
    std::cout << "数量: " << trade.quantity << std::endl;
}

// 打印账户信息
void printAccountInfo(const RiskManagement& riskManager) {
    std::cout << "\n=== 账户信息 ===" << std::endl;
    std::cout << "总余额: " << std::fixed << std::setprecision(2) 
              << riskManager.getAccountBalance() << " USDT" << std::endl;
    std::cout << "可用余额: " << riskManager.getAvailableBalance() << " USDT" << std::endl;
    std::cout << "未实现盈亏: " << riskManager.getUnrealizedPnL() << " USDT" << std::endl;
    std::cout << "已实现盈亏: " << riskManager.getRealizedPnL() << " USDT" << std::endl;
    std::cout << "最大回撤: " << (riskManager.getMaxDrawdown() * 100) << "%" << std::endl;
}

int main() {
    // 设置控制台编码
    setConsoleEncoding();
    
    // 创建数据源配置
    DataFeed::Config config;
    config.minPrice = 50000.0;      // 最小价格 50000
    config.maxPrice = 60000.0;      // 最大价格 60000
    config.updateIntervalMs = 100;  // 每100ms更新一次
    config.volatility = 0.01;       // 1%的波动率
    
    // 创建数据源实例
    DataFeed feed(config);
    
    // 创建交易引擎
    TradingEngine engine;
    
    // 创建风险管理器（设置初始资金为100000 USDT）
    RiskLimits limits;
    limits.maxPositionSize = 10.0;      // 最大持仓10 BTC
    limits.maxDrawdown = 0.1;           // 最大回撤10%
    limits.maxLoss = 10000.0;           // 最大亏损10000 USDT
    limits.maxOrderValue = 50000.0;     // 单笔订单最大价值50000 USDT
    limits.maxLeverage = 3.0;           // 最大杠杆3倍
    limits.maxSymbolPosition = 5.0;     // 单个交易对最大持仓5 BTC
    limits.maxSymbolOrderSize = 1.0;    // 单个交易对最大订单1 BTC
    limits.minSymbolOrderSize = 0.01;   // 单个交易对最小订单0.01 BTC
    
    RiskManagement riskManager(limits, 100000.0);  // 设置10万USDT初始资金
    
    // 打印初始账户信息
    std::cout << "初始账户状态:" << std::endl;
    printAccountInfo(riskManager);
    
    // 设置交易引擎回调
    engine.setOrderCallback([](const Order& order) {
        std::cout << "\n收到订单更新通知" << std::endl;
        printOrder(order);
    });
    
    engine.setTradeCallback([](const Trade& trade) {
        std::cout << "\n收到成交通知" << std::endl;
        printTrade(trade);
    });
    
    engine.setOrderBookCallback([](const OrderBook& book) {
        printOrderBook(book);
    });
    
    // 初始化并启动交易引擎
    engine.initialize();
    engine.start();
    
    // 订阅数据源
    feed.subscribe("BTC/USDT", [&engine, &riskManager](const std::string& symbol, double price) {
        // 更新风险管理器的价格
        riskManager.updatePrice(symbol, price);
        
        // 更新交易引擎的价格
        engine.onPriceUpdate(symbol, price);
        
        // 打印最新价格和账户信息
        std::cout << "\n收到价格更新: " << std::fixed << std::setprecision(2) << price << " USDT" << std::endl;
        printAccountInfo(riskManager);
    });
    
    // 启动数据源
    feed.start();
    
    // 等待数据源启动
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 创建并提交买单
    Order buyOrder;
    buyOrder.symbol = "BTC/USDT";
    buyOrder.type = OrderType::LIMIT;
    buyOrder.side = OrderSide::BUY;
    buyOrder.price = 55000.0;
    buyOrder.quantity = 0.5;
    buyOrder.status = OrderStatus::NEW;
    
    // 检查订单风险
    auto riskCheck = riskManager.checkOrder(buyOrder);
    if (riskCheck.passed) {
        std::cout << "\n提交买单" << std::endl;
        engine.submitOrder(buyOrder);
    } else {
        std::cout << "\n订单被风险控制拒绝: " << riskCheck.message << std::endl;
    }
    
    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 创建并提交卖单
    Order sellOrder;
    sellOrder.symbol = "BTC/USDT";
    sellOrder.type = OrderType::LIMIT;
    sellOrder.side = OrderSide::SELL;
    sellOrder.price = 56000.0;
    sellOrder.quantity = 0.5;
    sellOrder.status = OrderStatus::NEW;
    
    // 检查订单风险
    riskCheck = riskManager.checkOrder(sellOrder);
    if (riskCheck.passed) {
        std::cout << "\n提交卖单" << std::endl;
        engine.submitOrder(sellOrder);
    } else {
        std::cout << "\n订单被风险控制拒绝: " << riskCheck.message << std::endl;
    }
    
    // 继续运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 停止数据源和交易引擎
    feed.stop();
    engine.stop();
    
    // 打印最终账户信息
    std::cout << "\n最终账户状态:" << std::endl;
    printAccountInfo(riskManager);
    
    // 打印最终持仓信息
    auto position = riskManager.getPosition("BTC/USDT");
    std::cout << "\n=== 最终持仓信息 ===" << std::endl;
    std::cout << "数量: " << position.quantity << std::endl;
    std::cout << "均价: " << position.averagePrice << std::endl;
    std::cout << "未实现盈亏: " << position.unrealizedPnL << std::endl;
    std::cout << "已实现盈亏: " << position.realizedPnL << std::endl;
    std::cout << "最大回撤: " << (position.maxDrawdown * 100) << "%" << std::endl;
    
    return 0;
} 