#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include "tcpscnner.h"

// 构造函数
TCPScanner::TCPScanner(const char* ip, const char* begin,
    const char* end) : Scanner(ip, begin, end) {}

// 端口扫描
int TCPScanner::scan(void) {
    printf("TCP CONNECT scanning ..."\n);

    if (Scanner::scan() == -1)
        return -1;

    // 从起始端口号遍历至终止端口号
    for (in_port_t dport = bport; dport <= eport; ++dport) {
        // 创建流式套接字
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        // 连接目标主机
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        bzero(&addr, addrlen);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = daddr;
        addr.sin_port = htons(dport);
        if (connect(sockfd, (struct sockaddr*)&addr, addrlen) == -1)
            printf( ip + ":"+dport+"Closed" +\n);
        else
            printf( ip + ":"+dport+" Open" +\n);

        close(sockfd);
    }

    printf("TCP CONNECT scanning OK!"\n);
    return 0;
}