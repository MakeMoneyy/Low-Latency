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

OrderID OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 生成新的订单ID
    OrderID orderId = generateOrderId();
    auto newOrder = std::make_shared<Order>(order);
    newOrder->id = orderId;
    newOrder->timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    // 存储订单
    orders_[orderId] = newOrder;
    
    // 根据订单类型处理
    switch (newOrder->type) {
        case OrderType::LIMIT:
            processLimitOrder(*newOrder);
            break;
        case OrderType::MARKET:
            processMarketOrder(*newOrder);
            break;
        default:
            // 其他类型订单暂不支持
            updateOrderStatus(orderId, OrderStatus::REJECTED);
            break;
    }
    
    return orderId;
}

bool OrderBook::cancelOrder(OrderID orderId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return false;
    }
    
    auto order = it->second;
    if (order->status == OrderStatus::FILLED || 
        order->status == OrderStatus::CANCELED) {
        return false;
    }
    
    // 更新订单状态
    updateOrderStatus(orderId, OrderStatus::CANCELED);
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
        order->status == OrderStatus::CANCELED) {
        return false;
    }
    
    // 取消原订单
    cancelOrder(orderId);
    
    // 创建新订单
    Order newOrder = *order;
    newOrder.price = newPrice;
    newOrder.quantity = newQuantity;
    newOrder.filledQuantity = 0;
    newOrder.status = OrderStatus::NEW;
    
    // 添加新订单
    addOrder(newOrder);
    return true;
}

double OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (bids_.empty()) {
        return 0.0;
    }
    return bids_.top()->price;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (asks_.empty()) {
        return 0.0;
    }
    return asks_.top()->price;
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
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 清空输出参数
    bids.clear();
    asks.clear();
    
    // 复制买单队列
    auto bidsCopy = bids_;
    while (!bidsCopy.empty()) {
        auto order = bidsCopy.top();
        bids[order->price] += (order->quantity - order->filledQuantity);
        bidsCopy.pop();
    }
    
    // 复制卖单队列
    auto asksCopy = asks_;
    while (!asksCopy.empty()) {
        auto order = asksCopy.top();
        asks[order->price] += (order->quantity - order->filledQuantity);
        asksCopy.pop();
    }
}

void OrderBook::processLimitOrder(const Order& order) {
    if (order.side == OrderSide::BUY) {
        bids_.push(orders_[order.id]);
    } else {
        asks_.push(orders_[order.id]);
    }
    
    matchOrders();
}

void OrderBook::processMarketOrder(const Order& order) {
    matchOrders();
}

void OrderBook::matchOrders() {
    while (!bids_.empty() && !asks_.empty()) {
        auto bestBid = bids_.top();
        auto bestAsk = asks_.top();
        
        if (bestBid->price < bestAsk->price) {
            break;
        }
        
        // 计算可成交数量
        double matchQuantity = std::min(
            bestBid->quantity - bestBid->filledQuantity,
            bestAsk->quantity - bestAsk->filledQuantity
        );
        
        // 更新订单状态
        bestBid->filledQuantity += matchQuantity;
        bestAsk->filledQuantity += matchQuantity;
        
        if (bestBid->filledQuantity >= bestBid->quantity) {
            updateOrderStatus(bestBid->id, OrderStatus::FILLED, bestBid->filledQuantity);
            bids_.pop();
        } else {
            updateOrderStatus(bestBid->id, OrderStatus::PARTIALLY_FILLED, bestBid->filledQuantity);
        }
        
        if (bestAsk->filledQuantity >= bestAsk->quantity) {
            updateOrderStatus(bestAsk->id, OrderStatus::FILLED, bestAsk->filledQuantity);
            asks_.pop();
        } else {
            updateOrderStatus(bestAsk->id, OrderStatus::PARTIALLY_FILLED, bestAsk->filledQuantity);
        }
    }
}

void OrderBook::updateOrderStatus(OrderID orderId, OrderStatus status, double filledQuantity) {
    auto it = orders_.find(orderId);
    if (it != orders_.end()) {
        it->second->status = status;
        if (filledQuantity > 0) {
            it->second->filledQuantity = filledQuantity;
        }
    }
}

OrderID OrderBook::generateOrderId() {
    return nextOrderId_++;
} 