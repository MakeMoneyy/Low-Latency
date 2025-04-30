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
file(GLOB SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 