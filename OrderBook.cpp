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
    
    // 从价格档位中移除订单数量
    if (order->side == OrderSide::BUY) {
        bids_[order->price] -= (order->quantity - order->filledQuantity);
        if (bids_[order->price] <= 0) {
            bids_.erase(order->price);
        }
    } else {
        asks_[order->price] -= (order->quantity - order->filledQuantity);
        if (asks_[order->price] <= 0) {
            asks_.erase(order->price);
        }
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
    return bids_.empty() ? 0.0 : bids_.rbegin()->first;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return asks_.empty() ? 0.0 : asks_.begin()->first;
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
    bids = bids_;
    asks = asks_;
}

void OrderBook::processLimitOrder(const Order& order) {
    if (order.side == OrderSide::BUY) {
        bids_[order.price] += order.quantity;
    } else {
        asks_[order.price] += order.quantity;
    }
    
    matchOrders();
}

void OrderBook::processMarketOrder(const Order& order) {
    matchOrders();
}

void OrderBook::matchOrders() {
    while (!bids_.empty() && !asks_.empty()) {
        double bestBid = bids_.rbegin()->first;
        double bestAsk = asks_.begin()->first;
        
        if (bestBid < bestAsk) {
            break;
        }
        
        // 找到可以匹配的订单
        for (auto& [orderId, order] : orders_) {
            if (order->status == OrderStatus::FILLED || 
                order->status == OrderStatus::CANCELED) {
                continue;
            }
            
            if ((order->side == OrderSide::BUY && order->price >= bestAsk) ||
                (order->side == OrderSide::SELL && order->price <= bestBid)) {
                // 执行匹配
                double matchPrice = order->side == OrderSide::BUY ? bestAsk : bestBid;
                double matchQuantity = std::min(
                    order->quantity - order->filledQuantity,
                    order->side == OrderSide::BUY ? asks_[bestAsk] : bids_[bestBid]
                );
                
                // 更新订单状态
                order->filledQuantity += matchQuantity;
                if (order->filledQuantity >= order->quantity) {
                    updateOrderStatus(orderId, OrderStatus::FILLED, order->filledQuantity);
                } else {
                    updateOrderStatus(orderId, OrderStatus::PARTIALLY_FILLED, order->filledQuantity);
                }
                
                // 更新价格档位
                if (order->side == OrderSide::BUY) {
                    asks_[bestAsk] -= matchQuantity;
                    if (asks_[bestAsk] <= 0) {
                        asks_.erase(bestAsk);
                    }
                } else {
                    bids_[bestBid] -= matchQuantity;
                    if (bids_[bestBid] <= 0) {
                        bids_.erase(bestBid);
                    }
                }
            }
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