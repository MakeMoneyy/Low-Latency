cmake_minimum_required(VERSION 3.10)
project(LowLatency)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(DataFeedTest
    main.cpp
    OrderBook.cpp
    DataFeed.cpp
    TradingEngine.cpp
    RiskManagement.cpp
)

# 添加交易引擎测试程序
add_executable(TestEngine
    test_trading_engine.cpp
    TradingEngine.cpp
    OrderBook.cpp
    DataFeed.cpp
)

# 添加风险管理测试程序
add_executable(TestRiskManagement
    test_risk_management.cpp
    RiskManagement.cpp
)

# 添加头文件目录
target_include_directories(DataFeedTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(TestEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(TestRiskManagement PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 