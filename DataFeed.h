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

    DataFeed();
    ~DataFeed();

    // 订阅指定交易对的价格更新
    void subscribe(const std::string& symbol, PriceUpdateCallback callback);
    
    // 取消订阅
    void unsubscribe(const std::string& symbol);
    
    // 开始数据流
    void start();
    
    // 停止数据流
    void stop();

private:
    // 模拟数据生成线程
    void generateMockData();
    
    // 发布价格更新
    void publishPriceUpdate(const std::string& symbol, double price);

    std::unordered_map<std::string, PriceUpdateCallback> callbacks_;
    std::unique_ptr<std::thread> dataThread_;
    std::atomic<bool> running_{false};
}; 