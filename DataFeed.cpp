#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

DataFeed::DataFeed(const Config& config) : config_(config) {
    setConsoleEncoding();
    std::cout << "DataFeed 初始化" << std::endl;
}

DataFeed::~DataFeed() {
    stop();
}

void DataFeed::subscribe(const std::string& symbol, PriceUpdateCallback callback) {
    callbacks_[symbol] = callback;
    // 初始化该交易对的最后价格
    lastPrices_[symbol] = (config_.minPrice + config_.maxPrice) / 2.0;
    std::cout << "已订阅 " << symbol << " 的价格更新" << std::endl;
}

void DataFeed::unsubscribe(const std::string& symbol) {
    callbacks_.erase(symbol);
    lastPrices_.erase(symbol);
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

void DataFeed::updateConfig(const Config& config) {
    config_ = config;
    std::cout << "配置已更新" << std::endl;
}

double DataFeed::generateRandomPrice(const std::string& symbol) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> normal_dist(0.0, 1.0);
    
    // 获取上次价格
    double lastPrice = lastPrices_[symbol];
    
    // 生成随机波动
    double randomChange = normal_dist(gen) * config_.volatility * lastPrice;
    double newPrice = lastPrice + randomChange;
    
    // 确保价格在指定范围内
    newPrice = std::max(config_.minPrice, std::min(config_.maxPrice, newPrice));
    
    // 更新最后价格
    lastPrices_[symbol] = newPrice;
    
    return newPrice;
}

void DataFeed::generateMockData() {
    while (running_) {
        for (const auto& [symbol, callback] : callbacks_) {
            double newPrice = generateRandomPrice(symbol);
            publishPriceUpdate(symbol, newPrice);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.updateIntervalMs));
    }
}

void DataFeed::publishPriceUpdate(const std::string& symbol, double price) {
    auto it = callbacks_.find(symbol);
    if (it != callbacks_.end()) {
        it->second(symbol, price);
    }
} 