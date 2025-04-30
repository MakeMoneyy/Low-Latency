#include "RiskManagement.h"
#include "ConsoleUtils.h"
#include <iostream>
#include <iomanip>

void printRiskCheckResult(const RiskCheckResult& result) {
    std::cout << "风险检查结果: " << (result.passed ? "通过" : "拒绝") << std::endl;
    if (!result.message.empty()) {
        std::cout << "原因: " << result.message << std::endl;
    }
}

void printPosition(const std::string& symbol, const Position& pos) {
    std::cout << "\n=== " << symbol << " 持仓信息 ===" << std::endl;
    std::cout << "数量: " << std::fixed << std::setprecision(4) << pos.quantity << std::endl;
    std::cout << "均价: " << pos.averagePrice << std::endl;
    std::cout << "未实现盈亏: " << pos.unrealizedPnL << std::endl;
    std::cout << "已实现盈亏: " << pos.realizedPnL << std::endl;
    std::cout << "最大回撤: " << (pos.maxDrawdown * 100) << "%" << std::endl;
}

void test_risk_limits() {
    std::cout << "\n=== 测试风险限制 ===" << std::endl;
    
    // 设置风险限制
    RiskLimits limits;
    // 账户级别限制
    limits.maxPositionSize = 10.0;
    limits.maxDrawdown = 0.1;      // 10%
    limits.maxLoss = 1000.0;
    limits.maxOrderValue = 50000.0;
    limits.maxLeverage = 3.0;
    
    // 单个交易对限制
    limits.maxSymbolPosition = 2.0;    // 最大持仓2 BTC
    limits.maxSymbolOrderSize = 1.0;   // 单笔最大1 BTC
    limits.minSymbolOrderSize = 0.01;  // 单笔最小0.01 BTC
    
    RiskManagement riskManager(limits);
    
    // 测试超出单笔订单限制
    Order order1;
    order1.symbol = "BTC/USDT";
    order1.type = OrderType::LIMIT;
    order1.side = OrderSide::BUY;
    order1.price = 50000.0;
    order1.quantity = 1.5;  // 超出单笔最大限制1.0 BTC
    
    std::cout << "\n测试超出单笔订单限制:" << std::endl;
    auto result1 = riskManager.checkOrder(order1);
    printRiskCheckResult(result1);
    
    // 测试超出最小订单限制
    Order order2;
    order2.symbol = "BTC/USDT";
    order2.type = OrderType::LIMIT;
    order2.side = OrderSide::BUY;
    order2.price = 50000.0;
    order2.quantity = 0.005;  // 小于最小限制0.01 BTC
    
    std::cout << "\n测试小于最小订单限制:" << std::endl;
    auto result2 = riskManager.checkOrder(order2);
    printRiskCheckResult(result2);
    
    // 测试超出持仓限制
    Order order3;
    order3.symbol = "BTC/USDT";
    order3.type = OrderType::LIMIT;
    order3.side = OrderSide::BUY;
    order3.price = 50000.0;
    order3.quantity = 1.0;
    
    // 先执行一笔正常订单
    std::cout << "\n执行第一笔正常订单:" << std::endl;
    auto result3 = riskManager.checkOrder(order3);
    printRiskCheckResult(result3);
    
    // 模拟成交
    Trade trade;
    trade.symbol = order3.symbol;
    trade.price = order3.price;
    trade.quantity = order3.quantity;
    riskManager.updatePortfolio(trade);
    
    // 再尝试一笔会超出持仓限制的订单
    std::cout << "\n测试超出持仓限制:" << std::endl;
    auto result4 = riskManager.checkOrder(order3);
    printRiskCheckResult(result4);
    
    // 测试正常订单
    Order order4;
    order4.symbol = "BTC/USDT";
    order4.type = OrderType::LIMIT;
    order4.side = OrderSide::BUY;
    order4.price = 50000.0;
    order4.quantity = 0.5;
    
    std::cout << "\n测试正常订单:" << std::endl;
    auto result5 = riskManager.checkOrder(order4);
    printRiskCheckResult(result5);
}

void test_portfolio_update() {
    std::cout << "\n=== 测试投资组合更新 ===" << std::endl;
    
    RiskLimits limits;
    limits.maxPositionSize = 10.0;
    limits.maxDrawdown = 0.1;
    limits.maxLoss = 1000.0;
    limits.maxOrderValue = 50000.0;
    limits.maxLeverage = 3.0;
    limits.maxSymbolPosition = 5.0;
    limits.maxSymbolOrderSize = 2.0;
    limits.minSymbolOrderSize = 0.01;
    
    RiskManagement riskManager(limits);
    
    // 模拟一系列交易
    Trade trade1;
    trade1.symbol = "BTC/USDT";
    trade1.price = 50000.0;
    trade1.quantity = 1.0;
    
    std::cout << "\n执行买入交易:" << std::endl;
    riskManager.updatePortfolio(trade1);
    printPosition(trade1.symbol, riskManager.getPosition(trade1.symbol));
    
    // 更新市场价格
    std::cout << "\n更新市场价格到 52000.0:" << std::endl;
    riskManager.updatePrice(trade1.symbol, 52000.0);
    printPosition(trade1.symbol, riskManager.getPosition(trade1.symbol));
    
    // 部分平仓
    Trade trade2;
    trade2.symbol = "BTC/USDT";
    trade2.price = 52000.0;
    trade2.quantity = -0.5;
    
    std::cout << "\n执行部分平仓:" << std::endl;
    riskManager.updatePortfolio(trade2);
    printPosition(trade2.symbol, riskManager.getPosition(trade2.symbol));
}

int main() {
    setConsoleEncoding();
    
    test_risk_limits();
    test_portfolio_update();
    
    return 0;
} 