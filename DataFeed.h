#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>

class DataFeed {
public:
    using PriceUpdateCallback = std::function<void(const std::string& symbol, double price)>;

    // 配置结构体
    struct Config {
        double minPrice = 100.0;      // 最小价格
        double maxPrice = 200.0;      // 最大价格
        int updateIntervalMs = 1000;  // 更新间隔（毫秒）
        double volatility = 0.02;     // 价格波动率（2%）
    };

    DataFeed(const Config& config = Config());
    ~DataFeed();

    // 订阅指定交易对的价格更新
    void subscribe(const std::string& symbol, PriceUpdateCallback callback);
    
    // 取消订阅
    void unsubscribe(const std::string& symbol);
    
    // 开始数据流
    void start();
    
    // 停止数据流
    void stop();

    // 更新配置
    void updateConfig(const Config& config);

private:
    // 模拟数据生成线程
    void generateMockData();
    
    // 发布价格更新
    void publishPriceUpdate(const std::string& symbol, double price);

    // 生成随机价格
    double generateRandomPrice(const std::string& symbol);

    Config config_;
    std::unordered_map<std::string, PriceUpdateCallback> callbacks_;
    std::unordered_map<std::string, double> lastPrices_;
    std::unique_ptr<std::thread> dataThread_;
    std::atomic<bool> running_{false};
}; 