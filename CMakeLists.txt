cmake_minimum_required(VERSION 3.15)
project(FinancialApp VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译优化选项
if(MSVC)
    add_compile_options(/O2 /Oi /Ot /Oy /GL /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -mtune=native)
endif()

# 添加包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加源文件
set(LIB_SOURCES
    src/order_book.cpp
    src/trading_engine.cpp
    src/risk_management.cpp
)

# 创建库
add_library(FinancialAppLib STATIC ${LIB_SOURCES})

# 添加Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# 启用测试
enable_testing()

# 添加测试可执行文件
add_executable(
    FinancialAppTests
    tests/market_data_test.cpp
    tests/order_book_test.cpp
    tests/trading_engine_test.cpp
    tests/risk_management_test.cpp
)

# 链接测试可执行文件
target_link_libraries(
    FinancialAppTests
    GTest::gtest_main
    FinancialAppLib
)

# 注册测试
add_test(NAME FinancialAppTests COMMAND FinancialAppTests)

# 创建主程序可执行文件
add_executable(FinancialApp src/main.cpp)
target_link_libraries(FinancialApp FinancialAppLib)
set_target_properties(FinancialApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin") 