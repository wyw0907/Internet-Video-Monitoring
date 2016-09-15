#ifndef __CONFIG_H
#define __CONFIG_H

#include "VideoCtrl.h"
#include "TcpSocket.h"
#include "PicDeal.h"
#include <syslog.h>


#define PICBUFSIZE      1024*1024

typedef unsigned char   u8_t;
typedef unsigned int    u32_t;
typedef unsigned short  u16_t;


enum{NOTSETED,ISSETED};
typedef struct {
    int    VideoId;
    int    isSet;      //用于表现摄像头是否已经绑定用户
    int    TcpFd;
    int    UdpFd;

}_global;

extern _global V_global;



#endif
