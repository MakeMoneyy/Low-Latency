#include <iostream>
#include <iomanip>
#include <ctime>
#include "market_data.hpp"

int main() {
    financial::MarketData data;
    data.symbol = "AAPL";
    data.price = 150.25;
    data.volume = 1000.0;
    data.timestamp = std::chrono::system_clock::now();

    // Basic output
    std::cout << "Basic Output Example:" << std::endl;
    std::cout << "Symbol: " << data.symbol << std::endl;
    std::cout << "Price: " << data.price << std::endl;
    std::cout << "Volume: " << data.volume << std::endl;

    // Formatted output
    std::cout << "\nFormatted Output Example:" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Price: $" << std::setw(10) << data.price << std::endl;
    std::cout << "Volume: " << std::setw(10) << data.volume << std::endl;

    // Error output using std::cerr
    std::cerr << "\nError Message Example:" << std::endl;
    std::cerr << "Warning: High price volatility detected" << std::endl;

    // Log output using std::clog
    std::clog << "\nLog Message Example:" << std::endl;
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::clog << "System startup time: " << std::ctime(&now_time);

    return 0;
} 