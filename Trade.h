#pragma once

#include <string>

struct Trade {
    std::string buyOrderId;
    std::string sellOrderId;
    std::string symbol;
    double price;
    double quantity;
}; 