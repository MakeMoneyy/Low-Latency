#pragma once

#include <string>
#include <functional>
#include <memory>
#include "market_data.hpp"

namespace financial {

class DataFeed {
public:
    using MarketDataCallback = std::function<void(const MarketData&)>;

    virtual ~DataFeed() = default;

    // 订阅市场数据
    virtual void subscribe(const std::string& symbol, MarketDataCallback callback) = 0;

    // 取消订阅
    virtual void unsubscribe(const std::string& symbol) = 0;

    // 启动数据源
    virtual void start() = 0;

    // 停止数据源
    virtual void stop() = 0;

    // 获取数据源状态
    virtual bool isRunning() const = 0;
};

// 工厂函数创建数据源实例
std::unique_ptr<DataFeed> createDataFeed(const std::string& type);

} // namespace financial 