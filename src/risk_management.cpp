#include "risk_management.hpp"

namespace financial {

class RiskManagementImpl : public RiskManagement {
public:
    void initialize(const RiskLimits& limits) override {
        limits_ = limits;
    }

    bool checkOrderRisk(const Order& order) override {
        return order.quantity <= limits_.maxOrderSize;
    }

    void updateRiskMetrics(const std::string& symbol, const RiskMetrics& metrics) override {
        metrics_[symbol] = metrics;
    }

    const RiskLimits& getRiskLimits() const override {
        return limits_;
    }

    const RiskMetrics& getRiskMetrics(const std::string& symbol) const override {
        static const RiskMetrics emptyMetrics{};
        auto it = metrics_.find(symbol);
        return it != metrics_.end() ? it->second : emptyMetrics;
    }

    bool isRiskLimitExceeded() const override {
        for (const auto& [symbol, metrics] : metrics_) {
            if (metrics.currentPosition > limits_.maxPositionSize ||
                metrics.exposure > limits_.maxExposure ||
                metrics.leverage > limits_.maxLeverage ||
                metrics.dailyPnL < -limits_.maxDailyLoss ||
                metrics.maxDrawdown > limits_.maxDrawdown) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> getRiskAlerts() const override {
        std::vector<std::string> alerts;
        for (const auto& [symbol, metrics] : metrics_) {
            if (metrics.currentPosition > limits_.maxPositionSize) {
                alerts.push_back(symbol + ": Position size exceeded");
            }
            if (metrics.exposure > limits_.maxExposure) {
                alerts.push_back(symbol + ": Exposure limit exceeded");
            }
            if (metrics.leverage > limits_.maxLeverage) {
                alerts.push_back(symbol + ": Leverage limit exceeded");
            }
            if (metrics.dailyPnL < -limits_.maxDailyLoss) {
                alerts.push_back(symbol + ": Daily loss limit exceeded");
            }
            if (metrics.maxDrawdown > limits_.maxDrawdown) {
                alerts.push_back(symbol + ": Drawdown limit exceeded");
            }
        }
        return alerts;
    }

    void resetDailyMetrics() override {
        for (auto& [symbol, metrics] : metrics_) {
            metrics.dailyPnL = 0.0;
        }
    }

private:
    RiskLimits limits_;
    std::unordered_map<std::string, RiskMetrics> metrics_;
};

std::unique_ptr<RiskManagement> createRiskManagement() {
    return std::make_unique<RiskManagementImpl>();
}

} // namespace financial 