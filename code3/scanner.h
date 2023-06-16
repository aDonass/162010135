#pragma once

#include <netinet/in.h>
#include <string>

// 扫描器
class Scanner {
public:
    // 构造函数
    Scanner(const char* ip, const char* begin, const char* end);

    // 端口扫描
    virtual int scan(void);

protected:
    // 校验和结构
    typedef struct tag_Checksum {  
        uint32_t saddr; 
        uint32_t daddr; 
        uint8_t  useless; 
        uint8_t  protocol; 
        uint16_t length; 
    }   CHECKSUM;

    // 计算校验和
    uint16_t checksum(const void* buf, size_t len) const;

    // 初始化
    int init(void);
    // 目标主机是否存在
    int exist(void) const;

    const string ip;    // 目标主机IP
    const string begin; // 起始端口号
    const string end;   // 终止端口号

    in_addr_t saddr; // 源IP地址
    in_addr_t daddr; // 目的IP地址
    in_port_t bport; // 起始端口号
    in_port_t eport; // 终止端口号
};