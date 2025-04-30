#include "order_book.hpp"
#include <map>
#include <unordered_map>

namespace financial {

class OrderBookImpl : public OrderBook {
public:
    explicit OrderBookImpl(const std::string& symbol) : symbol_(symbol) {}

    void addOrder(const Order& order) override {
        if (order.side == Order::Side::BUY) {
            if (bids_.find(order.price) == bids_.end()) {
                bids_[order.price] = order.quantity;
                orderCounts_[order.price] = 1;
            } else {
                bids_[order.price] = order.quantity;  // 更新数量而不是累加
                orderCounts_[order.price]++;
            }
            orderIdToPriceMap_[order.orderId] = order.price;
            orderIdToSideMap_[order.orderId] = order.side;
            orderIdToQuantityMap_[order.orderId] = order.quantity;
        } else {
            if (asks_.find(order.price) == asks_.end()) {
                asks_[order.price] = order.quantity;
                orderCounts_[order.price] = 1;
            } else {
                asks_[order.price] = order.quantity;  // 更新数量而不是累加
                orderCounts_[order.price]++;
            }
            orderIdToPriceMap_[order.orderId] = order.price;
            orderIdToSideMap_[order.orderId] = order.side;
            orderIdToQuantityMap_[order.orderId] = order.quantity;
        }
    }

    void cancelOrder(const std::string& orderId) override {
        auto priceIt = orderIdToPriceMap_.find(orderId);
        if (priceIt == orderIdToPriceMap_.end()) {
            return;  // 订单不存在
        }

        double price = priceIt->second;
        Order::Side side = orderIdToSideMap_[orderId];
        double quantity = orderIdToQuantityMap_[orderId];

        if (side == Order::Side::BUY) {
            auto it = bids_.find(price);
            if (it != bids_.end()) {
                it->second -= quantity;
                orderCounts_[price]--;
                if (orderCounts_[price] <= 0 || it->second <= 0) {
                    bids_.erase(it);
                    orderCounts_.erase(price);
                }
            }
        } else {
            auto it = asks_.find(price);
            if (it != asks_.end()) {
                it->second -= quantity;
                orderCounts_[price]--;
                if (orderCounts_[price] <= 0 || it->second <= 0) {
                    asks_.erase(it);
                    orderCounts_.erase(price);
                }
            }
        }

        orderIdToPriceMap_.erase(orderId);
        orderIdToSideMap_.erase(orderId);
        orderIdToQuantityMap_.erase(orderId);
    }

    void modifyOrder(const Order& order) override {
        cancelOrder(order.orderId);
        addOrder(order);
    }

    double getBestBid() const override {
        if (bids_.empty()) return 0.0;
        auto it = bids_.rbegin();
        while (it != bids_.rend()) {
            if (it->second > 0) {
                return it->first;
            }
            ++it;
        }
        return 0.0;
    }

    double getBestAsk() const override {
        if (asks_.empty()) return 0.0;
        auto it = asks_.begin();
        while (it != asks_.end()) {
            if (it->second > 0) {
                return it->first;
            }
            ++it;
        }
        return 0.0;
    }

    double getSpread() const override {
        double bestBid = getBestBid();
        double bestAsk = getBestAsk();
        if (bestBid == 0.0 || bestAsk == 0.0) return 0.0;
        return bestAsk - bestBid;
    }

    std::vector<OrderBookLevel> getBids(int depth) const override {
        std::vector<OrderBookLevel> result;
        int count = 0;
        for (auto it = bids_.rbegin(); it != bids_.rend() && count < depth; ++it) {
            if (it->second > 0) {
                result.push_back({it->first, it->second, orderCounts_.at(it->first)});
                count++;
            }
        }
        return result;
    }

    std::vector<OrderBookLevel> getAsks(int depth) const override {
        std::vector<OrderBookLevel> result;
        int count = 0;
        for (auto it = asks_.begin(); it != asks_.end() && count < depth; ++it) {
            if (it->second > 0) {
                result.push_back({it->first, it->second, orderCounts_.at(it->first)});
                count++;
            }
        }
        return result;
    }

    double getMarketDepth(Order::Side side, double price) const override {
        if (side == Order::Side::BUY) {
            auto it = bids_.find(price);
            return it != bids_.end() ? it->second : 0.0;
        } else {
            auto it = asks_.find(price);
            return it != asks_.end() ? it->second : 0.0;
        }
    }

private:
    std::string symbol_;
    std::map<double, double> bids_;  // price -> quantity
    std::map<double, double> asks_;  // price -> quantity
    std::unordered_map<double, int> orderCounts_;  // price -> number of orders
    std::unordered_map<std::string, double> orderIdToPriceMap_;  // orderId -> price
    std::unordered_map<std::string, Order::Side> orderIdToSideMap_;  // orderId -> side
    std::unordered_map<std::string, double> orderIdToQuantityMap_;  // orderId -> quantity
};

std::unique_ptr<OrderBook> createOrderBook(const std::string& symbol) {
    return std::make_unique<OrderBookImpl>(symbol);
}

} // namespace financial 