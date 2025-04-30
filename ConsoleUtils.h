#pragma once

#include <windows.h>

// 设置控制台编码为 UTF-8
inline void setConsoleEncoding() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
} 