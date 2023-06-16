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
#define MAX_PORTS 65535

struct Host {
    char hostname[NI_MAXHOST];
    char ip[INET_ADDRSTRLEN];
    int openPorts[MAX_PORTS];
    int numOpenPorts;
};

void scanLAN(struct Host* hosts, int* numHosts) {
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    int sockfd, i;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }
    
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    
    ipaddr = (struct sockaddr_in *)&(ifr.ifr_addr);
    char* localIP = inet_ntoa(ipaddr->sin_addr);
    
    for (i = 1; i <= 254; i++) {
        char ip[INET_ADDRSTRLEN];
        snprintf(ip, sizeof(ip), "%s%d", localIP, i);
        
        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = 0;
        inet_pton(AF_INET, ip, &(target.sin_addr));
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 设置超时时间为1秒
        timeout.tv_usec = 0;
        
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(1);
        }
        
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        int status = connect(sockfd, (struct sockaddr*)&target, sizeof(target));
        if (status == 0) {
            struct Host host;
            strncpy(host.ip, ip, sizeof(host.ip));
            getnameinfo((struct sockaddr*)&target, sizeof(target), host.hostname, sizeof(host.hostname), NULL, 0, 0);
            host.numOpenPorts = 0;
            hosts[*numHosts] = host;
            (*numHosts)++;
        }
        
        close(sockfd);
    }
    
    close(sockfd);
}

void scanPorts(struct Host* hosts, int numHosts) {
    int i, j;
    for (i = 0; i < numHosts; i++) {
        struct hostent* host = gethostbyname(hosts[i].hostname);
        if (host == NULL) {
            perror("gethostbyname() failed");
            continue;
        }
        
        for (int port = 1; port <= MAX_PORTS; port++) {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                perror("socket creation failed");
                exit(1);
            }
            
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
            
            struct timeval timeout;
            timeout.tv_sec = 1;  // 设置超时时间为1秒
            timeout.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
            
            int status = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
            if (status == 0) {
                hosts[i].openPorts[hosts[i].numOpenPorts++] = port;
            }
            
            close(sockfd);
        }
        
        printf("\nHost: %s (%s)\n", hosts[i].hostname, hosts[i].ip);
        printf("Open Ports: ");
        for (j = 0; j < hosts[i].numOpenPorts; j++) {
            printf("%d ", hosts[i].openPorts[j]);
        }
        printf("\n");
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
