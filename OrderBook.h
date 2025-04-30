#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>

// 订单ID类型
using OrderID = uint64_t;

// 订单类型枚举
enum class OrderType {
    LIMIT,      // 限价单
    MARKET,     // 市价单
    STOP,       // 止损单
    STOP_LIMIT  // 止损限价单
};

// 订单方向枚举
enum class OrderSide {
    BUY,
    SELL
};

// 订单状态枚举
enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

// 订单结构体
struct Order {
    OrderID id;             // 订单ID
    std::string symbol;     // 交易对
    OrderType type;         // 订单类型
    OrderSide side;         // 买卖方向
    double price;           // 价格
    double quantity;        // 数量
    double filledQuantity;  // 已成交数量
    OrderStatus status;     // 订单状态
    uint64_t timestamp;     // 时间戳
};

// 订单簿类
class OrderBook {
public:
    OrderBook(const std::string& symbol);
    ~OrderBook();

    // 添加订单
    OrderID addOrder(const Order& order);
    
    // 取消订单
    bool cancelOrder(OrderID orderId);
    
    // 修改订单
    bool modifyOrder(OrderID orderId, double newPrice, double newQuantity);
    
    // 获取最佳买价
    double getBestBid() const;
    
    // 获取最佳卖价
    double getBestAsk() const;
    
    // 获取买盘深度
    size_t getBidDepth() const;
    
    // 获取卖盘深度
    size_t getAskDepth() const;
    
    // 获取订单信息
    std::shared_ptr<Order> getOrder(OrderID orderId) const;
    
    // 获取订单簿快照
    void getOrderBookSnapshot(
        std::map<double, double>& bids,
        std::map<double, double>& asks
    ) const;

private:
    // 比较器结构体
    struct BidComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            if (a->price != b->price) {
                return a->price < b->price; // 价格低的优先级低
            }
            return a->timestamp > b->timestamp; // 时间早的优先级高
        }
    };

    struct AskComparator {
        bool operator()(const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) const {
            if (a->price != b->price) {
                return a->price > b->price; // 价格高的优先级低
            }
            return a->timestamp > b->timestamp; // 时间早的优先级高
        }
    };

    // 处理限价单
    void processLimitOrder(const Order& order);
    
    // 处理市价单
    void processMarketOrder(const Order& order);
    
    // 匹配订单
    void matchOrders();
    
    // 更新订单状态
    void updateOrderStatus(OrderID orderId, OrderStatus status, double filledQuantity = 0.0);
    
    // 生成新的订单ID
    OrderID generateOrderId();

    const std::string symbol_;
    std::atomic<OrderID> nextOrderId_{1};
    
    // 订单存储
    std::unordered_map<OrderID, std::shared_ptr<Order>> orders_;
    
    // 优先队列存储
    std::priority_queue<std::shared_ptr<Order>, 
                       std::vector<std::shared_ptr<Order>>, 
                       BidComparator> bids_;
    std::priority_queue<std::shared_ptr<Order>, 
                       std::vector<std::shared_ptr<Order>>, 
                       AskComparator> asks_;
    
    // 线程安全
    mutable std::mutex mutex_;
}; 