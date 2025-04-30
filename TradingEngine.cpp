#include "TradingEngine.h"
#include <iostream>

bool TradingEngine::initialize() {
    std::cout << "初始化交易引擎..." << std::endl;
    return true;
}

void TradingEngine::start() {
    std::cout << "启动交易引擎..." << std::endl;
    running_ = true;
}

void TradingEngine::stop() {
    std::cout << "停止交易引擎..." << std::endl;
    running_ = false;
}

bool TradingEngine::submitOrder(const Order& order) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 获取或创建订单簿
    auto& orderBook = orderBooks_[order.symbol];
    if (!orderBook) {
        orderBook = std::make_shared<OrderBook>(order.symbol);
    }
    
    // 提交订单
    bool success = orderBook->addOrder(order);
    if (success) {
        notifyOrderUpdate(order);
        processOrderBookUpdate(order.symbol);
    }
    
    return success;
}

bool TradingEngine::cancelOrder(OrderID orderId) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 遍历所有订单簿查找订单
    for (auto& [symbol, orderBook] : orderBooks_) {
        auto order = orderBook->getOrder(orderId);
        if (order) {
            bool success = orderBook->cancelOrder(orderId);
            if (success) {
                notifyOrderUpdate(*order);
                processOrderBookUpdate(symbol);
            }
            return success;
        }
    }
    
    return false;
}

bool TradingEngine::modifyOrder(const Order& order) {
    if (!running_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 获取订单簿
    auto it = orderBooks_.find(order.symbol);
    if (it == orderBooks_.end()) {
        return false;
    }
    
    // 修改订单
    bool success = it->second->modifyOrder(order.id, order.price, order.quantity);
    if (success) {
        notifyOrderUpdate(order);
        processOrderBookUpdate(order.symbol);
    }
    
    return success;
}

void TradingEngine::onPriceUpdate(const std::string& symbol, double price) {
    if (!running_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 获取订单簿
    auto it = orderBooks_.find(symbol);
    if (it == orderBooks_.end()) {
        return;  // 如果没有找到对应的订单簿，直接返回
    }

    // 更新订单簿价格
    it->second->updatePrice(price);
    
    // 获取订单簿快照
    const OrderBook& book = *(it->second);
    
    // 检查是否有可匹配的订单
    double bestBid = book.getBestBid();
    double bestAsk = book.getBestAsk();
    
    // 如果买卖价差存在，检查是否可以匹配
    if (bestBid >= bestAsk) {
        // 获取所有可匹配的订单
        auto bids = book.getBids();
        auto asks = book.getAsks();
        
        // 遍历买盘和卖盘，寻找可匹配的订单
        for (auto bidIt = bids.begin(); bidIt != bids.end(); ++bidIt) {
            for (auto askIt = asks.begin(); askIt != asks.end(); ++askIt) {
                double bidPrice = bidIt->first;
                double askPrice = askIt->first;
                
                // 如果买价大于等于卖价，可以匹配
                if (bidPrice >= askPrice) {
                    double bidQuantity = bidIt->second;
                    double askQuantity = askIt->second;
                    double matchQuantity = std::min(bidQuantity, askQuantity);
                    
                    // 创建成交记录
                    Trade trade;
                    trade.symbol = symbol;
                    trade.price = askPrice;  // 使用卖价作为成交价
                    trade.quantity = matchQuantity;
                    
                    // 通知成交
                    notifyTrade(trade);
                    
                    // 更新订单簿
                    if (bidQuantity > askQuantity) {
                        // 买单部分成交
                        bidIt->second -= askQuantity;
                        asks.erase(askIt);
                    } else if (bidQuantity < askQuantity) {
                        // 卖单部分成交
                        askIt->second -= bidQuantity;
                        bids.erase(bidIt);
                    } else {
                        // 完全成交
                        bids.erase(bidIt);
                        asks.erase(askIt);
                    }
                    
                    // 通知订单簿更新
                    notifyOrderBookUpdate(book);
                    
                    // 如果订单簿发生变化，继续检查
                    if (bids.empty() || asks.empty()) {
                        break;
                    }
                } else {
                    // 如果买价小于卖价，停止检查
                    break;
                }
            }
        }
    }
}

double TradingEngine::getBestBid(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orderBooks_.find(symbol);
    return it != orderBooks_.end() ? it->second->getBestBid() : 0.0;
}

double TradingEngine::getBestAsk(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orderBooks_.find(symbol);
    return it != orderBooks_.end() ? it->second->getBestAsk() : 0.0;
}

const OrderBook& TradingEngine::getOrderBookSnapshot(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orderBooks_.find(symbol);
    if (it == orderBooks_.end()) {
        throw std::runtime_error("Symbol not found: " + symbol);
    }
    return *(it->second);
}

void TradingEngine::processOrderBookUpdate(const std::string& symbol) {
    auto it = orderBooks_.find(symbol);
    if (it != orderBooks_.end()) {
        notifyOrderBookUpdate(*it->second);
    }
}

void TradingEngine::notifyOrderUpdate(const Order& order) {
    if (orderCallback_) {
        orderCallback_(order);
    }
}

void TradingEngine::notifyTrade(const Trade& trade) {
    if (tradeCallback_) {
        tradeCallback_(trade);
    }
}

void TradingEngine::notifyOrderBookUpdate(const OrderBook& book) {
    if (orderBookCallback_) {
        orderBookCallback_(book);
    }
} 