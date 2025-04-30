#pragma once

#include <string>
#include <chrono>

// 订单ID类型
using OrderID = uint64_t;

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

enum class OrderType {
    LIMIT,      // 限价单
    MARKET,     // 市价单
    STOP,       // 止损单
    STOP_LIMIT  // 止损限价单
};

struct Order {
    OrderID id;             // 订单ID
    std::string symbol;     // 交易对
    OrderType type;         // 订单类型
    OrderSide side;         // 买卖方向
    double price;           // 价格
    double quantity;        // 数量
    double filledQuantity{0.0}; // 已成交数量
    OrderStatus status;     // 订单状态
    int64_t timestamp{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count()};
}; 