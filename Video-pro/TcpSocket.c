#include "include/config.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

int geteth0_ip(char *ipaddr)
{
    int ret;
    int sockfd;

    struct sockaddr_in *sin;
    struct ifreq ifr_ip;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        LOG("get eth0 error")
        return -1;
    }

    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);

    ret = ioctl(sockfd, SIOCGIFADDR, &ifr_ip);
    if(ret < 0){
        LOG("get eth0 error")
        return -1;
    }

    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr, inet_ntoa(sin->sin_addr));

    //printf("local ip:%s\n", ipaddr);
    return 0;
}


int http_connect(char* opt)
{
    int ret;
    char serverip[32];
    ret = geteth0_ip(serverip);
    if(ret < 0){
        LOG("get eth0 ip error!\n")
        return ret;
    }


    V_global.UdpFd = socket(AF_INET,SOCK_DGRAM,0);
    if(V_global.UdpFd < 0){
        LOG("socket error\n")
        return V_global.UdpFd;
    }



    V_global.TcpFd = socket(AF_INET,SOCK_STREAM,0);
    if(V_global.TcpFd < 0){
        LOG("socket error\n")
        return V_global.TcpFd;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    inet_aton(serverip,&server_addr.sin_addr);

    ret = bind(V_global.TcpFd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(ret < 0){
        LOG("bind tcpfd error\n")
        return ret;
    }
    ret = listen(V_global.TcpFd,5);
    if(ret < 0){
        LOG("listen tcpfd error\n")
        return ret;
    }
    printf("tcpsocket start listen ...");
    return 0;
}
