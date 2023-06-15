#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define START_PORT 1
#define END_PORT 65535

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <IP>\n", argv[0]);
        exit(1);
    }

    struct in_addr ipaddr;
    struct hostent *host;
    int sockfd;
    struct sockaddr_in dest_addr;

    // 将点分十进制IP地址转换为网络字节序
    inet_pton(AF_INET, argv[1], &ipaddr);

    // 获取主机信息
    host = gethostbyaddr(&ipaddr, sizeof(ipaddr), AF_INET);
    if (host == NULL) {
        printf("Host not found\n");
        exit(1);
    }

    // 打印主机名和IP地址
    printf("Hostname: %s\n", host->h_name);
    printf("IP: %s\n", inet_ntoa(ipaddr));

    // 初始化目标地址结构体
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = ipaddr.s_addr;

    // 扫描端口
    for (int port = START_PORT; port <= END_PORT; port++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("socket");
            exit(1);
        }

        dest_addr.sin_port = htons(port);

        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == 0) {
            printf("Port %d is open\n", port);
        }

        close(sockfd);
    }

    return 0;
}
