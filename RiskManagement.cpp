#include "RiskManagement.h"
#include <algorithm>
#include <cmath>

RiskManagement::RiskManagement(const RiskLimits& limits, double initialBalance)
    : limits_(limits), account_(initialBalance) {
    accountHighWaterMark_ = initialBalance;
}

RiskCheckResult RiskManagement::checkSymbolLimits(const Order& order) const {
    // 检查订单数量是否在最小和最大限制之间
    if (std::abs(order.quantity) < limits_.minSymbolOrderSize) {
        return {false, "订单数量小于最小限制"};
    }
    
    if (std::abs(order.quantity) > limits_.maxSymbolOrderSize) {
        return {false, "订单数量大于最大限制"};
    }
    
    // 检查持仓是否超过限制
    auto posIt = positions_.find(order.symbol);
    double currentPosition = posIt != positions_.end() ? posIt->second.quantity : 0.0;
    double newPosition = currentPosition + order.quantity;
    
    if (std::abs(newPosition) > limits_.maxSymbolPosition) {
        return {false, "持仓数量超过单个交易对最大限制"};
    }
    
    return {true, ""};
}

RiskCheckResult RiskManagement::checkOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查订单金额是否超过可用余额
    auto marginCheck = checkMarginRequirement(order);
    if (!marginCheck.passed) {
        return marginCheck;
    }
    
    // 检查单个交易对的风险限制
    auto symbolCheck = checkSymbolLimits(order);
    if (!symbolCheck.passed) {
        return symbolCheck;
    }
    
    // 检查其他风险限制
    auto positionCheck = checkPositionLimit(order);
    if (!positionCheck.passed) {
        return positionCheck;
    }
    
    auto drawdownCheck = checkDrawdownLimit(order);
    if (!drawdownCheck.passed) {
        return drawdownCheck;
    }
    
    auto valueCheck = checkOrderValueLimit(order);
    if (!valueCheck.passed) {
        return valueCheck;
    }
    
    auto leverageCheck = checkLeverageLimit(order);
    if (!leverageCheck.passed) {
        return leverageCheck;
    }
    
    // 如果所有检查都通过，冻结订单所需的保证金
    double orderValue = std::abs(order.price * order.quantity);
    if (!account_.freezeAmount(orderValue)) {
        return {false, "冻结保证金失败"};
    }
    
    return {true, "订单通过风险检查"};
}

void RiskManagement::updatePortfolio(const Trade& trade) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& position = positions_[trade.symbol];
    double oldQuantity = position.quantity;
    double oldAvgPrice = position.averagePrice;
    
    // 更新持仓数量和均价
    if (oldQuantity == 0.0) {
        position.quantity = trade.quantity;
        position.averagePrice = trade.price;
    } else {
        double newQuantity = oldQuantity + trade.quantity;
        if (std::abs(newQuantity) < 1e-10) {  // 清仓
            position.quantity = 0.0;
            position.averagePrice = 0.0;
            // 计算已实现盈亏
            double pnl = (trade.price - oldAvgPrice) * trade.quantity;
            position.realizedPnL += pnl;
            totalRealizedPnL_ += pnl;
            
            // 解冻保证金
            account_.unfreezeAmount(std::abs(trade.price * trade.quantity));
            // 更新账户余额
            account_.deposit(pnl);
        } else {
            position.quantity = newQuantity;
            position.averagePrice = (oldQuantity * oldAvgPrice + trade.quantity * trade.price) / newQuantity;
        }
    }
    
    // 更新未实现盈亏
    updateUnrealizedPnL(trade.symbol);
    
    // 更新回撤
    updateDrawdown(trade.symbol);
}

void RiskManagement::updatePrice(const std::string& symbol, double price) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    lastPrices_[symbol] = price;
    
    // 更新未实现盈亏
    updateUnrealizedPnL(symbol);
    
    // 更新回撤
    updateDrawdown(symbol);
}

Position RiskManagement::getPosition(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = positions_.find(symbol);
    return it != positions_.end() ? it->second : Position{};
}

double RiskManagement::getTotalValue() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return totalValue_;
}

double RiskManagement::getTotalPnL() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return totalUnrealizedPnL_ + totalRealizedPnL_;
}

double RiskManagement::getUnrealizedPnL() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return totalUnrealizedPnL_;
}

double RiskManagement::getRealizedPnL() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return totalRealizedPnL_;
}

double RiskManagement::getMaxDrawdown() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return accountMaxDrawdown_;
}

void RiskManagement::setRiskLimits(const RiskLimits& limits) {
    std::lock_guard<std::mutex> lock(mutex_);
    limits_ = limits;
}

RiskLimits RiskManagement::getRiskLimits() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return limits_;
}

RiskCheckResult RiskManagement::checkPositionLimit(const Order& order) const {
    auto it = positions_.find(order.symbol);
    double currentPos = it != positions_.end() ? it->second.quantity : 0.0;
    double newPos = currentPos + order.quantity;
    
    if (std::abs(newPos) > limits_.maxPositionSize) {
        return {false, "超出最大持仓限制"};
    }
    return {true, ""};
}

RiskCheckResult RiskManagement::checkDrawdownLimit(const Order& order) const {
    if (accountMaxDrawdown_ > limits_.maxDrawdown) {
        return {false, "超出最大回撤限制"};
    }
    return {true, ""};
}

RiskCheckResult RiskManagement::checkOrderValueLimit(const Order& order) const {
    double orderValue = order.price * std::abs(order.quantity);
    if (orderValue > limits_.maxOrderValue) {
        return {false, "超出单笔订单最大价值限制"};
    }
    return {true, ""};
}

RiskCheckResult RiskManagement::checkLeverageLimit(const Order& order) const {
    double totalPositionValue = 0.0;
    for (const auto& [symbol, position] : positions_) {
        auto priceIt = lastPrices_.find(symbol);
        if (priceIt != lastPrices_.end()) {
            totalPositionValue += std::abs(position.quantity * priceIt->second);
        }
    }
    
    // 添加新订单的价值
    totalPositionValue += std::abs(order.price * order.quantity);
    
    if (totalValue_ <= 0.0) {
        return {false, "账户价值为零或负数，无法计算杠杆"};
    }
    
    double leverage = totalPositionValue / totalValue_;
    if (leverage > limits_.maxLeverage) {
        return {false, "超出最大杠杆限制"};
    }
    return {true, ""};
}

void RiskManagement::updateDrawdown(const std::string& symbol) {
    auto it = positions_.find(symbol);
    if (it != positions_.end()) {
        auto& position = it->second;
        double currentValue = position.quantity * lastPrices_[symbol];
        
        // 更新单个交易对的最大回撤
        if (currentValue > position.highWaterMark) {
            position.highWaterMark = currentValue;
        }
        position.maxDrawdown = std::max(position.maxDrawdown,
            (position.highWaterMark - currentValue) / position.highWaterMark);
        
        // 更新账户级别的最大回撤
        double totalValue = getTotalValue();
        if (totalValue > accountHighWaterMark_) {
            accountHighWaterMark_ = totalValue;
        }
        accountMaxDrawdown_ = std::max(accountMaxDrawdown_,
            (accountHighWaterMark_ - totalValue) / accountHighWaterMark_);
    }
}

void RiskManagement::updateUnrealizedPnL(const std::string& symbol) {
    auto it = positions_.find(symbol);
    if (it != positions_.end()) {
        auto& position = it->second;
        auto priceIt = lastPrices_.find(symbol);
        if (priceIt != lastPrices_.end()) {
            double oldPnL = position.unrealizedPnL;
            position.unrealizedPnL = (priceIt->second - position.averagePrice) * position.quantity;
            totalUnrealizedPnL_ += position.unrealizedPnL - oldPnL;
        }
    }
}

RiskCheckResult RiskManagement::checkMarginRequirement(const Order& order) const {
    double orderValue = std::abs(order.price * order.quantity);
    if (orderValue > account_.getAvailableBalance()) {
        return {false, "可用余额不足"};
    }
    return {true, ""};
}

double RiskManagement::getAccountBalance() const {
    return account_.getBalance();
}

double RiskManagement::getAvailableBalance() const {
    return account_.getAvailableBalance();
} 