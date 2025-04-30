#include <gtest/gtest.h>
#include "risk_management.hpp"
#include "order_book.hpp"
#include <memory>

using namespace financial;

class RiskManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        riskManager = createRiskManagement();
        
        RiskLimits limits;
        limits.maxPositionSize = 1000.0;
        limits.maxOrderSize = 100.0;
        limits.maxDailyLoss = 10000.0;
        limits.maxDrawdown = 5000.0;
        limits.maxLeverage = 5.0;
        limits.maxExposure = 100000.0;
        
        riskManager->initialize(limits);
    }

    std::unique_ptr<RiskManagement> riskManager;
};

TEST_F(RiskManagementTest, OrderRiskCheck) {
    Order order;
    order.orderId = "1";
    order.symbol = "AAPL";
    order.side = Order::Side::BUY;
    order.type = Order::Type::LIMIT;
    order.price = 150.0;
    order.quantity = 50.0;  // 在限制范围内
    order.timestamp = std::chrono::system_clock::now();

    EXPECT_TRUE(riskManager->checkOrderRisk(order));

    order.quantity = 200.0;  // 超过最大订单大小
    EXPECT_FALSE(riskManager->checkOrderRisk(order));
}

TEST_F(RiskManagementTest, RiskMetricsUpdate) {
    RiskMetrics metrics;
    metrics.currentPosition = 500.0;
    metrics.unrealizedPnL = 1000.0;
    metrics.realizedPnL = 2000.0;
    metrics.dailyPnL = 3000.0;
    metrics.maxDrawdown = 1000.0;
    metrics.exposure = 50000.0;
    metrics.leverage = 2.0;

    riskManager->updateRiskMetrics("AAPL", metrics);
    const auto& updatedMetrics = riskManager->getRiskMetrics("AAPL");

    EXPECT_DOUBLE_EQ(updatedMetrics.currentPosition, 500.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.unrealizedPnL, 1000.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.realizedPnL, 2000.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.dailyPnL, 3000.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.maxDrawdown, 1000.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.exposure, 50000.0);
    EXPECT_DOUBLE_EQ(updatedMetrics.leverage, 2.0);
}

TEST_F(RiskManagementTest, RiskLimitExceeded) {
    RiskMetrics metrics;
    metrics.currentPosition = 2000.0;  // 超过最大持仓
    metrics.exposure = 200000.0;       // 超过最大风险敞口
    metrics.leverage = 10.0;           // 超过最大杠杆

    riskManager->updateRiskMetrics("AAPL", metrics);
    EXPECT_TRUE(riskManager->isRiskLimitExceeded());
}

TEST_F(RiskManagementTest, RiskAlerts) {
    RiskMetrics metrics;
    metrics.currentPosition = 2000.0;  // 超过最大持仓
    metrics.exposure = 200000.0;       // 超过最大风险敞口
    metrics.leverage = 10.0;           // 超过最大杠杆

    riskManager->updateRiskMetrics("AAPL", metrics);
    auto alerts = riskManager->getRiskAlerts();
    
    EXPECT_FALSE(alerts.empty());
    EXPECT_GT(alerts.size(), 0);
}

TEST_F(RiskManagementTest, DailyMetricsReset) {
    RiskMetrics metrics;
    metrics.dailyPnL = 3000.0;
    riskManager->updateRiskMetrics("AAPL", metrics);
    
    riskManager->resetDailyMetrics();
    const auto& updatedMetrics = riskManager->getRiskMetrics("AAPL");
    
    EXPECT_DOUBLE_EQ(updatedMetrics.dailyPnL, 0.0);
} 