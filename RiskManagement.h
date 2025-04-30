#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include "Order.h"
#include "Trade.h"

// 持仓信息
struct Position {
    double quantity{0.0};           // 持仓数量
    double averagePrice{0.0};       // 平均持仓价格
    double unrealizedPnL{0.0};      // 未实现盈亏
    double realizedPnL{0.0};        // 已实现盈亏
    double maxDrawdown{0.0};        // 最大回撤
    double highWaterMark{0.0};      // 历史最高价值
};

// 风险限制配置
struct RiskLimits {
    // 账户级别限制
    double maxPositionSize{0.0};    // 最大持仓规模
    double maxDrawdown{0.0};        // 最大回撤限制
    double maxLoss{0.0};            // 最大亏损限制
    double maxOrderValue{0.0};      // 单笔订单最大价值
    double maxLeverage{1.0};        // 最大杠杆倍数
    
    // 单个交易对限制
    double maxSymbolPosition{0.0};  // 单个交易对最大持仓
    double maxSymbolOrderSize{0.0}; // 单个交易对最大订单数量
    double minSymbolOrderSize{0.0}; // 单个交易对最小订单数量
};

// 风险检查结果
struct RiskCheckResult {
    bool passed{false};             // 是否通过风险检查
    std::string message;            // 风险提示信息
};

class RiskManagement {
public:
    explicit RiskManagement(const RiskLimits& limits);
    ~RiskManagement() = default;

    // 风险检查
    RiskCheckResult checkOrder(const Order& order);
    
    // 更新持仓
    void updatePortfolio(const Trade& trade);
    
    // 更新市场价格
    void updatePrice(const std::string& symbol, double price);
    
    // 查询接口
    Position getPosition(const std::string& symbol) const;
    double getTotalValue() const;
    double getTotalPnL() const;
    double getUnrealizedPnL() const;
    double getRealizedPnL() const;
    double getMaxDrawdown() const;
    
    // 风险限制设置
    void setRiskLimits(const RiskLimits& limits);
    RiskLimits getRiskLimits() const;

private:
    // 计算订单风险
    RiskCheckResult checkPositionLimit(const Order& order) const;
    RiskCheckResult checkDrawdownLimit(const Order& order) const;
    RiskCheckResult checkOrderValueLimit(const Order& order) const;
    RiskCheckResult checkLeverageLimit(const Order& order) const;
    RiskCheckResult checkSymbolLimits(const Order& order) const;
    
    // 更新风险指标
    void updateDrawdown(const std::string& symbol);
    void updateUnrealizedPnL(const std::string& symbol);
    
    // 成员变量
    RiskLimits limits_;
    std::unordered_map<std::string, Position> positions_;
    std::unordered_map<std::string, double> lastPrices_;
    mutable std::mutex mutex_;
    
    // 账户级别的风险指标
    double totalValue_{0.0};
    double totalUnrealizedPnL_{0.0};
    double totalRealizedPnL_{0.0};
    double accountHighWaterMark_{0.0};
    double accountMaxDrawdown_{0.0};
}; 