#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

// 构造函数
Scanner::Scanner(const char* ip, const char* begin, const char* end) :
    ip(ip), begin(begin), end(end),
    saddr(INADDR_NONE), daddr(INADDR_NONE), bport(0), eport(0) {}

// 端口扫描
int Scanner::scan(void) {
    // 初始化
    if (init() == -1)
        return -1;

    // 目标主机是否存在
    if (exist() == -1)
        return -1;

    return 0;
}

// 计算校验和
uint16_t Scanner::checksum(const void* buf, size_t len) const {
    const uint16_t* ptr = (const uint16_t*)buf;
    uint32_t sum = 0;

    for (; len > 1; len -= 2)
        sum += *ptr++;

    if (len)
        sum += *(const uint8_t*)ptr;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

// 初始化
int Scanner::init(void) {
    //
    // 源IP地址
    //
    FILE* fp = popen("/sbin/ifconfig | grep inet | grep -v 127 | "
        "awk '{print $2}' | cut -d \":\" -f 2", "r");
    if (! fp) {
        perror("popen");
        return -1;
    }

    char s[16];
    fscanf(fp, "%s", s);
    pclose(fp);

    if ((saddr = inet_addr(s)) == INADDR_NONE) {
        perror("inet_addr");
        return -1;
    }
    //
    // 目的IP地址
    //
    if ((daddr = inet_addr(ip.c_str())) == INADDR_NONE) {
        perror("inet_addr");
        return -1;
    }
    //
    // 端口范围
    //
    int b = atoi(begin.c_str()), e = atoi(end.c_str());
    if (b < 1 || 65535 < b || e < 1 || 65535 < e || e < b) {
        cerr << "Invalid port range" << endl;
        return -1;
    }

    bport = b;
    eport = e;

    printf(string(inet_ntoa(*((struct in_addr*)&saddr)))+ '-'+
        string(inet_ntoa(*((struct in_addr*)&daddr))) +':' +
        bport + '-' + eport + endl);

    printf("Initializing OK!"\n) ;
    return 0;
}

// 目标主机是否存在
int Scanner::exist(void) const {
    cout << "Ping destination host ..." << endl;

    // 创建基于ICMP协议的原始套接字
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    // 自己组织IP包头
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on,
        sizeof(on)) == -1) {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    // 组织IP包头
    uint8_t buf[1024];
    struct iphdr* iph = (struct iphdr*)buf;
    iph->version  = 4;            // 版本(4位)
    iph->ihl      = 5;            // 包头长度(4位)
    iph->tos      = 0;            // 服务类型
    iph->tot_len  = htons(
        sizeof(struct iphdr) +
        sizeof(struct icmphdr) +
        sizeof(struct timeval));  // 包长度
    iph->id       = rand();       // 标识符
    iph->frag_off = 0x40;         // 标志(3位)和偏移(13位)
    iph->ttl      = 64;           // 生存时间
    iph->protocol = IPPROTO_ICMP; // 上层协议
    iph->check    = 0;            // 校验和
    iph->saddr    = saddr;        // 源IP地址
    iph->daddr    = daddr;        // 目的IP地址

    // 组织ICMP包头
    struct icmphdr* icmph = (struct icmphdr*)(iph + 1);
    icmph->type             = ICMP_ECHO;    // 类型
    icmph->code             = 0;            // 代码
    icmph->checksum         = 0;            // 校验和
    icmph->un.echo.id       = htons(10086); // 标识符
    icmph->un.echo.sequence = 0;            // 序号

    // 组织数据载荷
    struct timeval* data = (struct timeval*)(icmph + 1);
    gettimeofday(data, NULL);

    // 计算校验和
    icmph->checksum = checksum(icmph, sizeof(*icmph) + sizeof(*data));

    // 发送ICMP请求
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    bzero(&addr, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = daddr;
    if (sendto(sockfd, buf, sizeof(*iph) + sizeof(*icmph) +
        sizeof(*data), 0, (struct sockaddr*)&addr, addrlen) == -1) {
        perror("sendto");
        close(sockfd);
        return -1;
    }

    // 将套接字设置为非阻塞模式
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl");
        close(sockfd);
        return -1;
    }

    // 接收循环
    struct timeval start;
    gettimeofday(&start, NULL);
    for (;;) {
        // 接收ICMP响应
        if (recvfrom(sockfd, buf, sizeof(buf), 0,
            (struct sockaddr*)&addr, &addrlen) > 0) {
			iph = (struct iphdr*)buf;
			icmph = (struct icmphdr*)(buf + iph->ihl * 4);
            if (iph->saddr == daddr && iph->daddr == saddr &&
                icmph->type == ICMP_ECHOREPLY) {
                struct timeval now;
                gettimeofday(&now, NULL);
                cout << noshowpoint << ((now.tv_sec - start.tv_sec) * 1e6 +
                    now.tv_usec - start.tv_usec) / 1e3 << " ms" << endl;
                break;
            }
        }

        // 超时目标主机不可达
        struct timeval now;
        gettimeofday(&now, NULL);
        if (((now.tv_sec - start.tv_sec) * 1e6 +
            now.tv_usec - start.tv_usec) / 1e6 > 5) {
            cerr << "Destination host Unreachable" << endl;
            close(sockfd);
            return -1;
        }
    }

    close(sockfd);

    printf( "Ping destination host OK!" );
    return 0;
}