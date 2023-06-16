#pragma once

#include "scanner.h"

// TCP CONNECT扫描器
class TCPScanner : public Scanner {
public:
    // 构造函数
    TCPScanner(const char* ip, const char* begin, const char* end);

    // 端口扫描
    int scan(void);
};
