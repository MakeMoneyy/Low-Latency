#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <thread>

DataFeed::DataFeed(const Config& config) 
    : config_(config)
    , running_(false)
    , lastUpdateTime_(std::chrono::steady_clock::now()) {
    
    // 初始化随机数生成器
    std::random_device rd;
    rng_ = std::mt19937(rd());
    priceDist_ = std::normal_distribution<>(0.0, config.volatility);
    setConsoleEncoding();
    std::cout << "DataFeed 初始化完成" << std::endl;
}

DataFeed::~DataFeed() {
    stop();
}

void DataFeed::subscribe(const std::string& symbol, PriceCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[symbol] = callback;
    lastPrices_[symbol] = (config_.minPrice + config_.maxPrice) / 2.0;
    std::cout << "已订阅: " << symbol << std::endl;
}

void DataFeed::unsubscribe(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.erase(symbol);
    lastPrices_.erase(symbol);
    std::cout << "已取消订阅: " << symbol << std::endl;
}

void DataFeed::start() {
    if (running_) return;
    
    running_ = true;
    updateThread_ = std::thread(&DataFeed::updateLoop, this);
}

void DataFeed::stop() {
    if (!running_) return;
    
    running_ = false;
    if (updateThread_.joinable()) {
        updateThread_.join();
    }
    std::cout << "DataFeed 已停止" << std::endl;
}

void DataFeed::updateConfig(const Config& newConfig) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = newConfig;
    priceDist_ = std::normal_distribution<>(0.0, config_.volatility);
    std::cout << "配置已更新" << std::endl;
}

void DataFeed::updateLoop() {
    while (running_) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdateTime_).count();
            
        if (elapsed >= config_.updateIntervalMs) {
            updatePrices();
            lastUpdateTime_ = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void DataFeed::updatePrices() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [symbol, callback] : callbacks_) {
        // 生成新的价格
        double priceChange = priceDist_(rng_);
        double newPrice = lastPrices_[symbol] * (1.0 + priceChange);
        
        // 确保价格在有效范围内
        newPrice = std::max(config_.minPrice, std::min(config_.maxPrice, newPrice));
        
        // 更新最后价格
        lastPrices_[symbol] = newPrice;
        
        // 调用回调
        callback(symbol, newPrice);
    }
}

void DataFeed::publishPriceUpdate(const std::string& symbol, double price) {
    auto it = callbacks_.find(symbol);
    if (it != callbacks_.end()) {
        it->second(symbol, price);
    }
} 