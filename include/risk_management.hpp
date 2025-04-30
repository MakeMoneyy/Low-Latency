#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "order.hpp"

namespace financial {

struct RiskLimits {
    double maxPositionSize;
    double maxOrderSize;
    double maxDailyLoss;
    double maxDrawdown;
    double maxLeverage;
    double maxExposure;
};

struct RiskMetrics {
    double currentPosition;
    double unrealizedPnL;
    double realizedPnL;
    double dailyPnL;
    double maxDrawdown;
    double exposure;
    double leverage;
};

class RiskManagement {
public:
    virtual ~RiskManagement() = default;

    // 初始化风险管理
    virtual void initialize(const RiskLimits& limits) = 0;

    // 检查订单风险
    virtual bool checkOrderRisk(const Order& order) = 0;

    // 更新风险指标
    virtual void updateRiskMetrics(const std::string& symbol, const RiskMetrics& metrics) = 0;

    // 获取风险限制
    virtual const RiskLimits& getRiskLimits() const = 0;

    // 获取风险指标
    virtual const RiskMetrics& getRiskMetrics(const std::string& symbol) const = 0;

    // 检查是否超过风险限制
    virtual bool isRiskLimitExceeded() const = 0;

    // 获取风险警报
    virtual std::vector<std::string> getRiskAlerts() const = 0;

    // 重置风险指标
    virtual void resetDailyMetrics() = 0;
};

// 工厂函数创建风险管理实例
std::unique_ptr<RiskManagement> createRiskManagement();

} // namespace financial 