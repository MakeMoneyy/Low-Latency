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
    
    // 创建 DataFeed 实例
    DataFeed feed;
    
    // 订阅几个交易对
    feed.subscribe("BTC/USD", onPriceUpdate);
    feed.subscribe("ETH/USD", onPriceUpdate);
    feed.subscribe("SOL/USD", onPriceUpdate);
    
    // 启动数据流
    feed.start();
    
    // 运行 10 秒钟
    std::cout << "数据流已启动，将运行 10 秒钟..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 停止数据流
    feed.stop();
    
    std::cout << "测试完成" << std::endl;
    return 0;
} 