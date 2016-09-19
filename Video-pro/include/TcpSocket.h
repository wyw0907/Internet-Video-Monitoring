#ifndef __TCPSOCKET_H
#define __TCOSOCKET_H
#include <sys/socket.h>
int http_connect(char *opt);
int data_deal_handle(char *,struct sockaddr *, socklen_t);

#endif
