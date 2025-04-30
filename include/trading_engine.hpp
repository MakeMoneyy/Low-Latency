#pragma once

#include <string>
#include <memory>
#include <functional>
#include "order_book.hpp"
#include "market_data.hpp"

namespace financial {

class TradingEngine {
public:
    using OrderCallback = std::function<void(const Order&, bool success)>;
    using MarketDataCallback = std::function<void(const MarketData&)>;

    virtual ~TradingEngine() = default;

    // 初始化交易引擎
    virtual void initialize() = 0;

    // 启动交易引擎
    virtual void start() = 0;

    // 停止交易引擎
    virtual void stop() = 0;

    // 提交订单
    virtual void submitOrder(const Order& order, OrderCallback callback) = 0;

    // 取消订单
    virtual void cancelOrder(const std::string& orderId) = 0;

    // 修改订单
    virtual void modifyOrder(const Order& order) = 0;

    // 注册市场数据回调
    virtual void registerMarketDataCallback(MarketDataCallback callback) = 0;

    // 获取订单簿
    virtual std::shared_ptr<OrderBook> getOrderBook(const std::string& symbol) = 0;

    // 获取交易状态
    virtual bool isRunning() const = 0;

    // 获取交易统计信息
    virtual struct TradingStats {
        uint64_t totalOrders;
        uint64_t filledOrders;
        uint64_t cancelledOrders;
        double totalVolume;
        double totalValue;
    } getTradingStats() const = 0;
};

// 工厂函数创建交易引擎实例
std::unique_ptr<TradingEngine> createTradingEngine();

} // namespace financial 