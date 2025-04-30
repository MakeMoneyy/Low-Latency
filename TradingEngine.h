#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include "OrderBook.h"
#include "Order.h"
#include "Trade.h"

// 订单回调函数类型
using OrderCallback = std::function<void(const Order&)>;
using TradeCallback = std::function<void(const Trade&)>;
using OrderBookCallback = std::function<void(const OrderBook&)>;

class TradingEngine {
public:
    TradingEngine() = default;
    ~TradingEngine() = default;

    // 生命周期管理
    bool initialize();
    void start();
    void stop();

    // 订单管理
    bool submitOrder(const Order& order);
    bool cancelOrder(OrderID orderId);
    bool modifyOrder(const Order& order);

    // 价格更新
    void onPriceUpdate(const std::string& symbol, double price);

    // 订单簿查询
    double getBestBid(const std::string& symbol) const;
    double getBestAsk(const std::string& symbol) const;
    const OrderBook& getOrderBookSnapshot(const std::string& symbol) const;

    // 回调设置
    void setOrderCallback(OrderCallback callback) { orderCallback_ = callback; }
    void setTradeCallback(TradeCallback callback) { tradeCallback_ = callback; }
    void setOrderBookCallback(OrderBookCallback callback) { orderBookCallback_ = callback; }

private:
    // 处理订单簿更新
    void processOrderBookUpdate(const std::string& symbol);
    
    // 通知订单更新
    void notifyOrderUpdate(const Order& order);
    
    // 通知成交
    void notifyTrade(const Trade& trade);
    
    // 通知订单簿更新
    void notifyOrderBookUpdate(const OrderBook& book);

    // 成员变量
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> orderBooks_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_{false};

    // 回调函数
    OrderCallback orderCallback_;
    TradeCallback tradeCallback_;
    OrderBookCallback orderBookCallback_;
}; 