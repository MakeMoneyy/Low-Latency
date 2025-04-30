#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>
#include "Order.h"

// 订单ID类型
using OrderID = uint64_t;

// 订单簿类
class OrderBook {
public:
    explicit OrderBook(const std::string& symbol);
    ~OrderBook();

    // 订单管理
    bool addOrder(const Order& order);
    bool cancelOrder(OrderID orderId);
    bool modifyOrder(OrderID orderId, double newPrice, double newQuantity);
    std::shared_ptr<Order> getOrder(OrderID orderId) const;

    // 价格更新
    void updatePrice(double price);

    // 订单匹配
    void matchOrders();

    // 查询接口
    double getBestBid() const;
    double getBestAsk() const;
    const std::string& getSymbol() const { return symbol_; }
    std::map<double, double> getBids() const;
    std::map<double, double> getAsks() const;
    size_t getBidDepth() const;
    size_t getAskDepth() const;
    void getOrderBookSnapshot(std::map<double, double>& bids, std::map<double, double>& asks) const;
    OrderID generateOrderId();

private:
    void processLimitOrder(const Order& order);
    void processMarketOrder(const Order& order);
    void matchOrder(const Order& order);

    std::string symbol_;
    std::unordered_map<OrderID, std::shared_ptr<Order>> orders_;
    std::priority_queue<std::pair<double, OrderID>> bids_;
    std::priority_queue<std::pair<double, OrderID>, 
                       std::vector<std::pair<double, OrderID>>,
                       std::greater<>> asks_;
    double lastPrice_{0.0};
    mutable std::mutex mutex_;
    std::atomic<OrderID> nextOrderId_{1};
}; 