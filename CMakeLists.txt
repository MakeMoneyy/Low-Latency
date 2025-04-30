cmake_minimum_required(VERSION 3.10)
project(DataFeedTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(DataFeedTest
    main.cpp
    DataFeed.cpp
)

# 包含头文件目录
target_include_directories(DataFeedTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 