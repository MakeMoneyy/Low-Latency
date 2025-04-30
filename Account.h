#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

// 账户信息
class Account {
public:
    explicit Account(double initialBalance = 0.0) : balance_(initialBalance) {}

    // 获取账户余额
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_;
    }

    // 获取可用余额（考虑冻结资金）
    double getAvailableBalance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_ - frozenAmount_;
    }

    // 冻结资金
    bool freezeAmount(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (amount <= 0 || amount > getAvailableBalance()) {
            return false;
        }
        frozenAmount_ += amount;
        return true;
    }

    // 解冻资金
    bool unfreezeAmount(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (amount <= 0 || amount > frozenAmount_) {
            return false;
        }
        frozenAmount_ -= amount;
        return true;
    }

    // 存入资金
    bool deposit(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (amount <= 0) {
            return false;
        }
        balance_ += amount;
        return true;
    }

    // 提取资金
    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (amount <= 0 || amount > getAvailableBalance()) {
            return false;
        }
        balance_ -= amount;
        return true;
    }

private:
    double balance_{0.0};        // 账户余额
    double frozenAmount_{0.0};   // 冻结金额
    mutable std::mutex mutex_;   // 互斥锁
}; 