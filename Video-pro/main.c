
#include <getopt.h>
#include "include/config.h"
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

#define SQLUSER      ""
#define SQLPASSWD    ""
#define DATABASE  ""

_global V_global;

static void help(char *arg)
{
    printf("---------------------------------------------\n");
    printf("arqument: -v/-video  + \"\" open    choosed video\n");
    printf("arqument: -s/-sql    + \"\" connect choosed sql\n");
    printf("arqument: -h/-http   + \"\" connect choosed http-service\n");
    printf("arqument: -r/-reset         reset this video\n");
    printf("arqument: -h/-help          print help message\n");
    printf("arqument: -p/-people + \"\" add a new people into group\n");
    printf("arqument: -l/-local         use off-line mode\n");
    printf("----------------------------------------------\n");
    printf("\n");
    printf("\n");
    return;
}
static int sql_connect(char *opt)
{
    MYSQL *conn = mysql_init(NULL);
    char value = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, (char *)&value);
    //连接数据库
    if(! mysql_real_connect(conn,opt,SQLUSER,SQLPASSWD,DATABASE, 0, NULL, 0))
    {
        LOG("connect mysql")
        return -1;
    }
    return 0;
}




int main(int argc,char **argv)
{
    int sql_ret;
    int http_ret;
    V_global.mode = ONLINE;
    V_global.isSet = NOTSETED;
    V_global.PicFd = -1;
    V_global.TcpFd = -1;
    V_global.UdpFd = -1;
    V_global.ConnectFd = -1;


    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {"v", required_argument, 0, 0},
            {"video", required_argument, 0, 0},
            {"s", required_argument, 0, 0},
            {"sql", required_argument, 0, 0},
            {"h", required_argument, 0, 0},
            {"http", required_argument, 0, 0},
            {"r", no_argument, 0, 0},
            {"reset", no_argument, 0, 0},
            {"p", required_argument, 0, 0},
            {"people", required_argument, 0, 0},
            {"l",no_argument,0,0},
            {"local",no_argument,0,0},
            {0, 0, 0, 0}
        };

        c = getopt_long_only(argc, argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            help(argv[0]);
            return 0;
        }

        switch(option_index) {
            /* h, help */
        case 0:
        case 1:
            help(argv[0]);
            break;

            /* v, video */
        case 2:
        case 3:
        //    video_init(optarg);
            break;
            /* s,sql */
        case 4:
        case 5:
            sql_ret = sql_connect(optarg);
            break;
            /* h,http */
        case 6:
        case 7:
            http_ret = http_connect(optarg);
            break;
            /* r,reset */
        case 8:
        case 9:
          //  video_reset();
            break;
            /* p,people */
        case 10:
        case 11:
          //  add_people(optarg);
            break;
            /* -l.-local */
        case 12:
        case 13:
            V_global.mode = OFFLINE;
        }
    }



    if(V_global.mode == OFFLINE){
        off_line_process();
        return 0;
    }
    if(sql_ret < 0 || http_ret < 0){
        LOG("connect to sql or http-service error!")
        return 1;
    }

    int maxFd = -1;
    fd_set rdset;
    struct timeval timeout = {5,0};
    char rdbuf[BUFSIZE] = {0};
    while(1)
    {
        memset(rdbuf,0,BUFSIZE);
        maxFd = (V_global.TcpFd>V_global.UdpFd?V_global.TcpFd:V_global.UdpFd) + 1;
        FD_ZERO(&rdset);
        FD_SET(V_global.TcpFd,&rdset);
        FD_SET(V_global.UdpFd,&rdset);
        if(V_global.ConnectFd > 0){
            maxFd = V_global.ConnectFd;
            FD_SET(V_global.ConnectFd,&rdset);
        }

        select(maxFd,&rdset,NULL,NULL,&timeout);
        if(FD_ISSET(V_global.TcpFd,&rdset))
        {
            if(V_global.ConnectFd != -1)
            {
                int fd = accept(V_global.TcpFd,NULL,NULL);
                if(fd > 0)
                    close(fd);
            }
            V_global.ConnectFd = accept(V_global.TcpFd,NULL,NULL);
        }
        if(FD_ISSET(V_global.UdpFd,&rdset))
        {

        }

    }


    return 0;
}

