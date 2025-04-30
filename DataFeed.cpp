#include "DataFeed.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

DataFeed::DataFeed(const Config& config) : 
    config_(config),
    gen_(rd_()),
    priceDist_(config.minPrice, config.maxPrice) {
    setConsoleEncoding();
    std::cout << "DataFeed 初始化完成" << std::endl;
}

DataFeed::~DataFeed() {
    stop();
}

void DataFeed::subscribe(const std::string& symbol, PriceUpdateCallback callback) {
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
    if (running_.exchange(true)) {
        return;
    }

    stopRequested_ = false;
    
    // 启动工作线程
    for (size_t i = 0; i < config_.threadPoolSize; ++i) {
        workerThreads_.emplace_back(&DataFeed::workerThread, this);
    }
    
    // 启动数据生成线程
    std::thread([this]() {
        while (running_ && !stopRequested_) {
            std::unique_lock<std::mutex> lock(mutex_);
            for (const auto& [symbol, callback] : callbacks_) {
                double newPrice = generateRandomPrice(symbol);
                UpdateTask task{symbol, callback, newPrice};
                
                {
                    std::lock_guard<std::mutex> queueLock(queueMutex_);
                    taskQueue_.push(std::move(task));
                }
                queueCondition_.notify_one();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.updateIntervalMs));
        }
    }).detach();

    std::cout << "DataFeed 已启动" << std::endl;
}

void DataFeed::stop() {
    if (!running_.exchange(false)) {
        return;
    }

    stopRequested_ = true;
    queueCondition_.notify_all();
    
    // 等待所有线程结束
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads_.clear();

    std::cout << "DataFeed 已停止" << std::endl;
}

void DataFeed::updateConfig(const Config& newConfig) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = newConfig;
    priceDist_ = std::uniform_real_distribution<>(config_.minPrice, config_.maxPrice);
    std::cout << "配置已更新" << std::endl;
}

double DataFeed::generateRandomPrice(const std::string& symbol) {
    double lastPrice = lastPrices_[symbol];
    double randomChange = (priceDist_(gen_) - (config_.maxPrice + config_.minPrice) / 2.0) * config_.volatility;
    double newPrice = lastPrice * (1.0 + randomChange);
    newPrice = std::max(config_.minPrice, std::min(config_.maxPrice, newPrice));
    lastPrices_[symbol] = newPrice;
    return newPrice;
}

void DataFeed::workerThread() {
    while (running_ && !stopRequested_) {
        UpdateTask task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this] {
                return !taskQueue_.empty() || !running_ || stopRequested_;
            });
            
            if (!running_ || stopRequested_) {
                break;
            }
            
            if (!taskQueue_.empty()) {
                task = std::move(taskQueue_.front());
                taskQueue_.pop();
            }
        }
        
        if (task.callback) {
            task.callback(task.symbol, task.price);
        }
    }
}

void DataFeed::publishPriceUpdate(const std::string& symbol, double price) {
    auto it = callbacks_.find(symbol);
    if (it != callbacks_.end()) {
        it->second(symbol, price);
    }
} 