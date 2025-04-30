#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <random>
#include <chrono>

DataFeed::DataFeed() {
    setConsoleEncoding();
    std::cout << "DataFeed 初始化" << std::endl;
}

DataFeed::~DataFeed() {
    stop();
}

void DataFeed::subscribe(const std::string& symbol, PriceUpdateCallback callback) {
    callbacks_[symbol] = callback;
    std::cout << "已订阅 " << symbol << " 的价格更新" << std::endl;
}

void DataFeed::unsubscribe(const std::string& symbol) {
    callbacks_.erase(symbol);
    std::cout << "已取消订阅 " << symbol << " 的价格更新" << std::endl;
}

void DataFeed::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    dataThread_ = std::make_unique<std::thread>(&DataFeed::generateMockData, this);
    std::cout << "数据流已启动" << std::endl;
}

void DataFeed::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    if (dataThread_ && dataThread_->joinable()) {
        dataThread_->join();
    }
    std::cout << "数据流已停止" << std::endl;
}

void DataFeed::generateMockData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> price_dist(100.0, 200.0);
    
    while (running_) {
        for (const auto& [symbol, callback] : callbacks_) {
            double newPrice = price_dist(gen);
            publishPriceUpdate(symbol, newPrice);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 每秒更新一次
    }
}

void DataFeed::publishPriceUpdate(const std::string& symbol, double price) {
    auto it = callbacks_.find(symbol);
    if (it != callbacks_.end()) {
        it->second(symbol, price);
    }
} 