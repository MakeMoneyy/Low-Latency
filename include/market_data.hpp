#pragma once

#include <string>
#include <chrono>

namespace financial {

struct MarketData {
    std::string symbol;
    double price;
    double volume;
    std::chrono::system_clock::time_point timestamp;
};

} // namespace financial 