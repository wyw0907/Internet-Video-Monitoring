#include "include/config.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>
#include <include/cJSON.h>

const char Json_data[] = {
  "{\"video\" : {"
  " \"from\" : \"%s\","
  " \"to\"   : \"%s\","
  " \"cmd\"  : \"%s\","
  " \"value\": \"%s\" "
  "}"
  "}"
};


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

void *alive_func(void *arg)
{
    struct sockaddr_in addr = *((struct sockaddr_in *)arg);
    while(1){
        sendto(V_global.UdpFd,"alive",strlen("alive"),0,\
               (struct sockaddr *)&addr,sizeof(addr));
 #ifdef __DEBUG
        sleep(5);
 #else
        sleep(3600);
 #endif
    }
    pthread_exit((void *)0);
}

int http_connect(char* opt)
{
 //   int ret;
  //  char serverip[32];
//    ret = geteth0_ip(serverip);
//    if(ret < 0){
//        LOG("get eth0 ip error!\n")
//        return ret;
//    }

    int ret;
    V_global.UdpFd = socket(AF_INET,SOCK_DGRAM,0);
    if(V_global.UdpFd < 0){
        LOG("udp socket error\n")
        return V_global.UdpFd;
    }

    V_global.TcpFd = socket(AF_INET,SOCK_STREAM,0);
    if(V_global.TcpFd < 0){
        LOG("tcp socket error\n")
        return V_global.TcpFd;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    inet_pton(AF_INET,opt,&server_addr.sin_addr);

    pthread_t tid;
    ret = pthread_create(&tid,NULL,alive_func,(void *)server_addr);
    if(ret != 0){
        LOG("alive pthread create error!\n")
        return -1;
    }
    pthread_detach(tid);
//    ret = bind(V_global.TcpFd,(struct sockaddr *)&server_addr,sizeof(server_addr));
//    if(ret < 0){
//        LOG("bind tcpfd error\n")
//        return ret;
//    }
//    ret = listen(V_global.TcpFd,5);
//    if(ret < 0){
//        LOG("listen tcpfd error\n")
//        return ret;
//    }
//    printf("tcpsocket start listen ...");

    return 0;
}

int data_deal_handle(char *data,struct sockaddr *service,socklen_t length)
{
    int ret;
    cJSON *cJson = cJSON_Parse(data);
    if(!cJson){
        LOG("cJSON error!\n")
        goto _err;
    }
    cJSON *c1=NULL,*c3=NULL;

    c1 = cJSON_GetObjectItem(cJson,"video");
    if(!c1 || c1->type != cJSON_Object){
        LOG("get video error!\n")
        goto _err;
    }

//    c2 = cJSON_GetObjectItem(c1,"interaction");
//    if(!c2 || c2->type != cJSON_Object){
//        LOG("get interaction error!\n")
//        goto _err;
//    }
    c3 = cJSON_GetObjectItem(c1,"from");
    if(c3 && c3->type == cJSON_String){
        printf("from : %s\n",c3->valuestring);
    }
    c3 = cJSON_GetObjectItem(c1,"to");
    if(c3 && c3->type == cJSON_String){
        printf("to : %s\n",c3->valuestring);
    }
    c3 = cJSON_GetObjectItem(c1,"cmd");
    if(c3 && c3->type == cJSON_String){
        printf("cmd : %s\n",c3->valuestring);
        if(strcmp(c3->valuestring,"alive") == 0)
        {
            ret = sendto(V_global.UdpFd,"alive",strlen("alive"),0,\
                   (struct sockaddr *)&service,length);
            if(ret <= 0){
                LOG("send \"alive\" to sevice error!\n")
            }
        }
        if(strcmp(c3->valuestring,"video")==0)
        {
            ret = connect(V_global.TcpFd,(struct sockaddr *)&service,length);
            if(ret < 0){
                LOG("connect service error!\n")
                sendto(V_global.UdpFd,"cannot connect",strlen("cannot connect"),0,\
                       (struct sockaddr *)&service,length);
            }
            else
                V_global.videoReq = REQUEST;
        }
        if(strcmp(c3->valuestring,"stop")==0)
        {
            V_global.videoReq = NOREQUEST;
            close(V_global.TcpFd);
            V_global.TcpFd = socket(AF_INET,SOCK_STREAM,0);
            if(V_global.TcpFd < 0){
                LOG("tcp socket error\n")
                exit(1);
            }
        }
    }
    c3 = cJSON_GetObjectItem(c1,"value");
    if(c3 && c3->type == cJSON_String){
        printf("value : %s\n",c3->valuestring);
    }


_err:
    cJSON_Delete(cJson);
    return 0;
}

