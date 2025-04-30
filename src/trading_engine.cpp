#include "trading_engine.hpp"
#include <unordered_map>

namespace financial {

// 前向声明
class TradingEngineImpl;

// 将实现类声明为公共的
class TradingEngineImpl : public TradingEngine {
public:
    void initialize() override {}
    void start() override { running_ = true; }
    void stop() override { running_ = false; }
    
    void submitOrder(const Order& order, OrderCallback callback) override {
        if (callback) callback(order, true);
    }
    
    void cancelOrder(const std::string& orderId) override {}
    void modifyOrder(const Order& order) override {}
    
    void registerMarketDataCallback(MarketDataCallback callback) override {
        marketDataCallback_ = callback;
    }
    
    std::shared_ptr<OrderBook> getOrderBook(const std::string& symbol) override {
        auto it = orderBooks_.find(symbol);
        if (it == orderBooks_.end()) {
            auto orderBook = std::shared_ptr<OrderBook>(createOrderBook(symbol));
            orderBooks_[symbol] = orderBook;
            return orderBook;
        }
        return it->second;
    }
    
    bool isRunning() const override { return running_; }
    
    const TradingStats& getTradingStats() const override { return stats_; }

    // 添加用于测试的方法
    void onMarketData(const MarketData& data) {
        if (marketDataCallback_) {
            marketDataCallback_(data);
        }
    }

private:
    bool running_{false};
    TradingStats stats_;
    MarketDataCallback marketDataCallback_;
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> orderBooks_;
};

std::unique_ptr<TradingEngine> createTradingEngine() {
    return std::make_unique<TradingEngineImpl>();
}

} // namespace financial 