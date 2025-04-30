#pragma once

#include <string>
#include <vector>
#include <memory>
#include "market_data.hpp"

namespace financial {

struct Order {
    enum class Side { BUY, SELL };
    enum class Type { MARKET, LIMIT };

    std::string orderId;
    std::string symbol;
    Side side;
    Type type;
    double price;
    double quantity;
    std::chrono::system_clock::time_point timestamp;
};

struct OrderBookLevel {
    double price;
    double quantity;
    int orderCount;
};

class OrderBook {
public:
    virtual ~OrderBook() = default;

    // 添加订单
    virtual void addOrder(const Order& order) = 0;

    // 取消订单
    virtual void cancelOrder(const std::string& orderId) = 0;

    // 修改订单
    virtual void modifyOrder(const Order& order) = 0;

    // 获取最佳买价
    virtual double getBestBid() const = 0;

    // 获取最佳卖价
    virtual double getBestAsk() const = 0;

    // 获取买卖价差
    virtual double getSpread() const = 0;

    // 获取指定深度的订单簿
    virtual std::vector<OrderBookLevel> getBids(int depth = 10) const = 0;
    virtual std::vector<OrderBookLevel> getAsks(int depth = 10) const = 0;

    // 获取市场深度
    virtual double getMarketDepth(Order::Side side, double price) const = 0;
};

// 工厂函数创建订单簿实例
std::unique_ptr<OrderBook> createOrderBook(const std::string& symbol);

} // namespace financial 