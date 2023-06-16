#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_HOSTS 256
#define PORT 80

// 结构体，用于保存主机信息
struct Host {
    char hostname[NI_MAXHOST];
    char ip[INET_ADDRSTRLEN];
};

// 扫描局域网内的主机
void scanLAN(struct Host* hosts, int* numHosts) {
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    int sockfd, i;
    
    // 创建原始套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }
    
    // 获取网络接口的IP地址
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    
    // 获取本地IP地址
    ipaddr = (struct sockaddr_in *)&(ifr.ifr_addr);
    char* localIP = inet_ntoa(ipaddr->sin_addr);
    printf("Local IP: %s\n", localIP);
    
    // 遍历局域网内的IP地址，尝试连接主机并获取主机名
    for (i = 1; i <= 254; i++) {
        char ip[INET_ADDRSTRLEN];
        snprintf(ip, sizeof(ip), "%s%d", localIP, i);
        
        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(PORT);
        inet_pton(AF_INET, ip, &(target.sin_addr));
        
        int status = connect(sockfd, (struct sockaddr*)&target, sizeof(target));
        if (status == 0) {
            struct Host host;
            strncpy(host.ip, ip, sizeof(host.ip));
            getnameinfo((struct sockaddr*)&target, sizeof(target), host.hostname, sizeof(host.hostname), NULL, 0, 0);
            hosts[*numHosts] = host;
            (*numHosts)++;
        }
    }
    
    close(sockfd);
}

// 扫描主机的开放端口
void scanPorts(const struct Host* hosts, int numHosts) {
    int sockfd, i;
    struct sockaddr_in server;
    struct hostent *host;
    
    for (i = 0; i < numHosts; i++) {
        printf("\nHost: %s (%s)\n", hosts[i].hostname, hosts[i].ip);
        
        host = gethostbyname(hosts[i].hostname);
        if (host == NULL) {
            perror("gethostbyname() failed");
            continue;
        }
        
        for (int port = 1; port <= 65535; port++) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                perror("socket creation failed");
                exit(1);
            }
            
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
            
            int status = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
            if (status == 0) {
                printf("Port %d is open\n", port);
            }
            
            close(sockfd);
        }
    }
}

int main() {
    struct Host hosts[MAX_HOSTS];
    int numHosts = 0;
    
    scanLAN(hosts, &numHosts);
    
    printf("Hosts in LAN:\n");
    for (int i = 0; i < numHosts; i++) {
        printf("%d. %s (%s)\n", i+1, hosts[i].hostname, hosts[i].ip);
    }
    
    scanPorts(hosts, numHosts);
    
    return 0;
}
