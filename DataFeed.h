#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <random>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>

class DataFeed {
public:
    // 配置结构体
    struct Config {
        double minPrice{0.0};        // 最小价格
        double maxPrice{0.0};        // 最大价格
        int updateIntervalMs{100};   // 更新间隔（毫秒）
        double volatility{0.01};     // 波动率
    };

    // 价格更新回调函数类型
    using PriceCallback = std::function<void(const std::string&, double)>;

    explicit DataFeed(const Config& config);
    ~DataFeed();

    // 订阅/取消订阅价格更新
    void subscribe(const std::string& symbol, PriceCallback callback);
    void unsubscribe(const std::string& symbol);

    // 启动/停止数据流
    void start();
    void stop();

    // 更新配置
    void updateConfig(const Config& newConfig);

    // 手动发布价格更新
    void publishPriceUpdate(const std::string& symbol, double price);

private:
    void updateLoop();
    void updatePrices();

    Config config_;
    std::atomic<bool> running_{false};
    std::chrono::steady_clock::time_point lastUpdateTime_;
    std::thread updateThread_;
    std::mutex mutex_;
    std::unordered_map<std::string, PriceCallback> callbacks_;
    std::unordered_map<std::string, double> lastPrices_;
    std::mt19937 rng_;
    std::normal_distribution<> priceDist_;
}; 