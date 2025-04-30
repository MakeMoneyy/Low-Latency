#include "OrderBook.h"
#include <iostream>
#include <chrono>
#include <algorithm>

OrderBook::OrderBook(const std::string& symbol) : symbol_(symbol) {
    std::cout << "创建订单簿: " << symbol << std::endl;
}

OrderBook::~OrderBook() {
    std::cout << "销毁订单簿: " << symbol_ << std::endl;
}

bool OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 创建订单副本并分配ID
    auto newOrder = std::make_shared<Order>(order);
    if (newOrder->id == 0) {
        newOrder->id = generateOrderId();
    }
    
    // 存储订单
    orders_[newOrder->id] = newOrder;
    
    // 根据订单类型处理
    if (newOrder->type == OrderType::LIMIT) {
        processLimitOrder(*newOrder);
    } else if (newOrder->type == OrderType::MARKET) {
        processMarketOrder(*newOrder);
    }
    
    return true;
}

bool OrderBook::cancelOrder(OrderID orderId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return false;
    }
    
    auto order = it->second;
    if (order->status == OrderStatus::FILLED || 
        order->status == OrderStatus::CANCELLED) {
        return false;
    }
    
    order->status = OrderStatus::CANCELLED;
    return true;
}

bool OrderBook::modifyOrder(OrderID orderId, double newPrice, double newQuantity) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return false;
    }
    
    auto order = it->second;
    if (order->status == OrderStatus::FILLED || 
        order->status == OrderStatus::CANCELLED) {
        return false;
    }
    
    // 取消原订单
    order->status = OrderStatus::CANCELLED;
    
    // 创建新订单
    Order newOrder = *order;
    newOrder.price = newPrice;
    newOrder.quantity = newQuantity;
    newOrder.status = OrderStatus::NEW;
    
    return addOrder(newOrder);
}

double OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bids_.empty() ? 0.0 : bids_.top().first;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return asks_.empty() ? 0.0 : asks_.top().first;
}

size_t OrderBook::getBidDepth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return bids_.size();
}

size_t OrderBook::getAskDepth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return asks_.size();
}

std::shared_ptr<Order> OrderBook::getOrder(OrderID orderId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = orders_.find(orderId);
    return it != orders_.end() ? it->second : nullptr;
}

void OrderBook::getOrderBookSnapshot(
    std::map<double, double>& bids,
    std::map<double, double>& asks
) const {
    bids = getBids();
    asks = getAsks();
}

void OrderBook::updatePrice(double price) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastPrice_ = price;
    matchOrders();
}

void OrderBook::matchOrders() {
    while (!bids_.empty() && !asks_.empty()) {
        auto bestBid = bids_.top();
        auto bestAsk = asks_.top();
        
        if (bestBid.first >= bestAsk.first) {
            // 可以成交
            auto bidOrder = orders_[bestBid.second];
            auto askOrder = orders_[bestAsk.second];
            
            double matchPrice = bestAsk.first;
            double matchQuantity = std::min(bidOrder->quantity - bidOrder->filledQuantity,
                                          askOrder->quantity - askOrder->filledQuantity);
            
            // 更新订单状态
            bidOrder->filledQuantity += matchQuantity;
            askOrder->filledQuantity += matchQuantity;
            
            if (bidOrder->filledQuantity >= bidOrder->quantity) {
                bidOrder->status = OrderStatus::FILLED;
                bids_.pop();
            } else {
                bidOrder->status = OrderStatus::PARTIALLY_FILLED;
            }
            
            if (askOrder->filledQuantity >= askOrder->quantity) {
                askOrder->status = OrderStatus::FILLED;
                asks_.pop();
            } else {
                askOrder->status = OrderStatus::PARTIALLY_FILLED;
            }
        } else {
            break;
        }
    }
}

void OrderBook::processLimitOrder(const Order& order) {
    if (order.side == OrderSide::BUY) {
        bids_.push({order.price, order.id});
    } else {
        asks_.push({order.price, order.id});
    }
    matchOrders();
}

void OrderBook::processMarketOrder(const Order& order) {
    if (order.side == OrderSide::BUY) {
        if (!asks_.empty()) {
            bids_.push({std::numeric_limits<double>::max(), order.id});
        }
    } else {
        if (!bids_.empty()) {
            asks_.push({0.0, order.id});
        }
    }
    matchOrders();
}

OrderID OrderBook::generateOrderId() {
    return nextOrderId_++;
}

std::map<double, double> OrderBook::getBids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<double, double> result;
    auto tempBids = bids_;
    while (!tempBids.empty()) {
        auto [price, _] = tempBids.top();
        result[price] += 1.0;  // 简化处理，每个价格只显示一个订单
        tempBids.pop();
    }
    return result;
}

std::map<double, double> OrderBook::getAsks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<double, double> result;
    auto tempAsks = asks_;
    while (!tempAsks.empty()) {
        auto [price, _] = tempAsks.top();
        result[price] += 1.0;  // 简化处理，每个价格只显示一个订单
        tempAsks.pop();
    }
    return result;
} 