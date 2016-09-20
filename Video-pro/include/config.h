#ifndef __CONFIG_H
#define __CONFIG_H
#include <stdio.h>
#include "VideoCtrl.h"
#include "TcpSocket.h"
#include "PicDeal.h"
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define AVINAME         "camera.avi"
#define PICNAME         "camera.jpg"
#define SERVERPORT      9999
//#define SERVERIP        "127.0.0.1"


#define PICBUFSIZE      1024*1024
#define BUFSIZE         128
#define LOG(...)        {char buff[BUFSIZE];snprintf(buff,BUFSIZE-1,__VA_ARGS__);\
                        fprintf(stderr,"%s",buff);syslog(LOG_INFO,"%s",buff);}

typedef unsigned char   u8_t;
typedef unsigned int    u32_t;
typedef unsigned short  u16_t;


enum{NOTSETED,ISSETED};
enum{ONLINE,OFFLINE};
enum{NOREQUEST,REQUEST};
typedef struct {
    int    VideoId;
    int    mode;        //离线模式或者联网,默认联网
    int    isSet;      //用于表现摄像头是否已经绑定用户
    int    TcpFd;
    int    UdpFd;
//    int    ConnectFd;
    int    videoReq;
    u8_t   VideoBuf[PICBUFSIZE];
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    pthread_t   getvideo;
    pthread_t   dealvideo;
    pthread_t   sendvideo;

    char    group[32];
}_global;

extern _global V_global;



#endif
