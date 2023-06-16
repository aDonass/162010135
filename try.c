#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <interface>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct arpreq arpreq;
    struct sockaddr_in *sin;
    struct in_addr ipaddr;
    struct hostent *host;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // 初始化arpreq结构体
    memset(&arpreq, 0, sizeof(arpreq));
    sin = (struct sockaddr_in *)&arpreq.arp_pa;
    sin->sin_family = AF_INET;
    strncpy(arpreq.arp_dev, argv[1], sizeof(arpreq.arp_dev) - 1);

    // 扫描局域网内的主机
    for (int i = 1; i <= 254; i++) {
        // 设置IP地址
        sin->sin_addr.s_addr = htonl((192 << 24) | (168 << 16) | (1 << 8) | i);

        // 发送ARP请求
        if (ioctl(sockfd, SIOCGARP, &arpreq) == -1) {
            continue;
        }

        // 获取主机信息
        ipaddr = ((struct sockaddr_in *)&arpreq.arp_pa)->sin_addr;
        host = gethostbyaddr(&ipaddr, sizeof(ipaddr), AF_INET);

        // 打印主机名和IP地址
        printf("Hostname: %s\n", host ? host->h_name : "unknown");
        printf("IP: %s\n", inet_ntoa(ipaddr));
    }

    close(sockfd);

    return 0;
}
