#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "fcntl.h"
#include "errno.h"

#define PORT_START 1
#define PORT_END 3306
#define MAX_BUFF_SIZE 4096

void scan_hosts(char *subnet, int port_start, int port_end) {
    struct hostent *host;
    struct sockaddr_in addr;
    int sockfd, result, i, j;

    for (i = 1; i <= 255; i++) {
        char ip[16];
        sprintf(ip, "%s.%d", subnet, i);
        if ((host = gethostbyname(ip)) == NULL) {
            fprintf(stderr, "Error: Could not resolve hostname %s.\n", ip);
        } else {
            memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);
            for (j = port_start; j <= port_end; j++) {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    fprintf(stderr, "Error: Could not create socket.\n");
                    exit(1);
                }
                addr.sin_family = AF_INET;
                addr.sin_port = htons(j);
                fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
                fd_set fds, read_fds;
                result = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
                if (result != 0) {
                    if(errno != EINPROGRESS)
                        printf("connect error :%s\n",strerror(errno));
                    FD_ZERO(&fds);
                    FD_ZERO(&read_fds);
                    FD_SET(sockfd, &fds);
                    FD_SET(sockfd, &read_fds);
                    struct timeval time = {0, 50};
                    int res = select(sockfd + 1, &read_fds, &fds, NULL, &time);
                    if (res == 1) {
                        printf("%s:%d is open.\n", ip, j);
                    } else {

                    }
                }
                close(sockfd);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s subnet\n", argv[0]);
        exit(1);
    }
    char *subnet = argv[1];
    printf("Scanning subnet %s...\n", subnet);
    scan_hosts(subnet, PORT_START, PORT_END);
    return 0;
}
