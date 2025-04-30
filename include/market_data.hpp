#pragma once

#include <string>
#include <chrono>

namespace financial {

struct MarketData {
    std::string symbol;
    double price{0.0};
    double volume{0.0};
    std::chrono::system_clock::time_point timestamp;
};

} // namespace financial 