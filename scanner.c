#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
 
#define Number_of_LAN  16//定义要扫描的主机数量
#define Number_of_ports 255//定义要扫描的端口数量
#define pianyi 0 //从第多少个ip开始扫描（最后一位数）
 

struct Address{
    char hostname[20];
    char address[20];
    int port;
    int open;
}addr[Number_of_LAN][Number_of_ports];
 
char *TranNum(int num)//输入一个整形数组0-999，将其转换为字符串
{
    int a , b, c;
 
    c = num % 10;
    num = num - c;
    b = num % 100;
    b = b / 10;
    num = num - b * 10;
    a = num / 100;
 
    char *strNum = (char *)malloc(4 * sizeof(char));
 
    if(a != 0)
    {
        strNum[0] = a + 48;
        strNum[1] = b + 48;
        strNum[2] = c + 48;
        strNum[3] = '\0';
    }
    else{
        if(b == 0)
        {
            strNum[0] = c + 48;
            strNum[1] = '\0';
        }
        else{
            strNum[0] = b + 48;
            strNum[1] = c + 48;
            strNum[2] = '\0';
        }
    }
 
    return strNum;
}
 
void TranAddr(char *ad)//输入一个地址，输出其局域网内的前n个地址，到address结构体中
{
    char Addr[20];
    strcpy(Addr, ad);//由于函数输入的指针存在于堆栈中，不能修改，所以申请字符数组存放
 
    struct in_addr strAddr;
	struct hostent *phost;
 
    int count, i, j, z;
    
    for(i = 0; i < Number_of_LAN; i++)
    {
        for(count = 0, z = 0; count < 3; z++)
            if(Addr[z] == '.')
                count++;
        
        Addr[z] = '\0';
        strcpy(Addr, strcat(Addr, TranNum(i +pianyi+ 1)));
 
        inet_pton(AF_INET, Addr, &strAddr);//将ip地址转换为二进制
 
        phost = gethostbyaddr((const char*)&strAddr, sizeof(strAddr), AF_INET);//获取主机名
        if (phost != NULL)
            strcpy(addr[i][0].hostname, phost->h_name);
 
        for(j = 0; j < Number_of_ports; j++)
            strcpy(addr[i][j].address, Addr);
    }
}
 
void  sockconnect(struct Address *b)
{
    int sockfd;
    struct sockaddr_in seraddr;
 
    char addr[20];
    strcpy(addr, b->address);
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("sock");
        exit(-1);
    }
 
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(addr);
    seraddr.sin_port = htons(b->port);
// 向服务器发起连接请求
    if (connect(sockfd, (struct sockaddr *)&seraddr,sizeof(seraddr)) != 0)
        b->open = 0;
    else
        b->open = 1;
    
    close(sockfd);
}
 
void *pth_main(void *addr)
{
    sockconnect((struct Address*)addr);
 
    pthread_exit(0);
}
 
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Missing parameter\n");
        exit(-1);
    }
    
    pthread_t pth[Number_of_LAN * Number_of_ports];
    int i, j, Ipcount, Portscount, number;
 
    TranAddr(argv[1]);
 
    for(i = 0; i < Number_of_LAN; i++)//调整连接的端口值，将没获得主机名的地址赋名为none
    {
        if(strlen(addr[i][0].hostname) == 0)
            strcpy(addr[i][0].hostname, "nameless");
        for(j = 0; j < Number_of_ports; j++)
        {
            addr[i][j].port = j + 1;
            addr[i][j].open = 0;
        }
    }
 
 
    for(number = 0; number < Number_of_LAN; number = number + 4)
    {
            for(i = number; i < number + 4; i++)
            {
                for(j = 0; j < Number_of_ports; j++)
                {
                    if((pthread_create(&pth[i * Number_of_ports + j], NULL, pth_main, (void *)&addr[i][j])) != 0)
                    {
                        printf("进程%d创建失败\n", i * Number_of_ports + j);
                        exit(-1);
                    }
                }
            }
            for(i = number * Number_of_ports; i < (number + 4) * Number_of_ports; i++)
            {
                pthread_join(pth[i], NULL);
            }
            
    }
 
//输出
    Ipcount = 0;
    printf("主机名                  ip地址               开放端口\n");
    for(i = 0; i < Number_of_LAN; i++)
    {
        printf("%-17s       %-16s    ", addr[i][0].hostname, addr[i][0].address);
        Portscount = 0;
        for(j = 0; j < Number_of_ports; j++)
        {
            if(addr[i][j].open != 0){
                printf(" '%d' ", addr[i][j].port);
                Portscount ++;
            }
        }
        if(Portscount == 0)
            printf(" none");
        else    Ipcount++;
        printf("\n");
    }
    printf("共有%d个主机开放\n", Ipcount);
    return 0;
}
