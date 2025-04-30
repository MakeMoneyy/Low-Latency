#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <chrono>
#include <thread>

// 价格更新回调函数
void onPriceUpdate(const std::string& symbol, double price) {
    std::cout << "[" << symbol << "] 收到价格更新: " << price << std::endl;
}

int main() {
    setConsoleEncoding();
    std::cout << "开始测试 DataFeed 模块..." << std::endl;
    
    // 创建自定义配置
    DataFeed::Config config;
    config.minPrice = 50.0;        // 最小价格 50
    config.maxPrice = 150.0;       // 最大价格 150
    config.updateIntervalMs = 500; // 每 500ms 更新一次
    config.volatility = 0.01;      // 1% 的波动率
    
    // 使用自定义配置创建 DataFeed 实例
    DataFeed feed(config);
    
    // 订阅几个交易对
    feed.subscribe("BTC/USD", onPriceUpdate);
    feed.subscribe("ETH/USD", onPriceUpdate);
    feed.subscribe("SOL/USD", onPriceUpdate);
    
    // 启动数据流
    feed.start();
    
    // 运行 5 秒钟
    std::cout << "数据流已启动，将运行 5 秒钟..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 更新配置
    config.updateIntervalMs = 1000; // 改为每秒更新一次
    config.volatility = 0.02;      // 增加波动率到 2%
    feed.updateConfig(config);
    std::cout << "配置已更新：更新间隔改为 1 秒，波动率改为 2%" << std::endl;
    
    // 再运行 5 秒钟
    std::cout << "继续运行 5 秒钟..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 停止数据流
    feed.stop();
    
    std::cout << "测试完成" << std::endl;
    return 0;
} 