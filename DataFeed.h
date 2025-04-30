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
    using PriceUpdateCallback = std::function<void(const std::string& symbol, double price)>;

    // 配置结构体
    struct Config {
        double minPrice;           // 最小价格
        double maxPrice;           // 最大价格
        int updateIntervalMs;      // 更新间隔（毫秒）
        double volatility;         // 价格波动率
        size_t threadPoolSize;     // 线程池大小

        // 默认构造函数
        Config() : 
            minPrice(100.0),
            maxPrice(200.0),
            updateIntervalMs(1000),
            volatility(0.02),
            threadPoolSize(4) {}
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
    // 工作线程函数
    void workerThread();
    
    // 生成随机价格
    double generateRandomPrice(const std::string& symbol);
    
    // 发布价格更新
    void publishPriceUpdate(const std::string& symbol, double price);

    // 任务结构体
    struct UpdateTask {
        std::string symbol;
        PriceUpdateCallback callback;
        double price;
    };

    Config config_;
    std::unordered_map<std::string, PriceUpdateCallback> callbacks_;
    std::unordered_map<std::string, double> lastPrices_;
    
    // 线程池相关
    std::vector<std::thread> workerThreads_;
    std::queue<UpdateTask> taskQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    
    // 控制标志
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};

    // 随机数生成器
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<> priceDist_;

    // 同步原语
    mutable std::mutex mutex_;
    std::condition_variable cv_;
}; 