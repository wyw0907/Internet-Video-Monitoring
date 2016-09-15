#include <stdio.h>
#include <getopt.h>
#include "config.h"
#include <mysql/mysql.h>

#define SQLUSER      ""
#define SQLPASSWD    ""
#define DATABASE  ""

static void help(char *arg)
{
    printf("---------------------------------------------\n");
    printf("arqument: -v/-video  + \"\" open    choosed video\n");
    printf("arqument: -s/-sql    + \"\" connect choosed sql\n");
    printf("arqument: -h/-http   + \"\" connect choosed http-service\n");
    printf("arqument: -r/-reset         reset this video\n");
    printf("arqument: -h/-help          print help message\n");
    printf("arqument: -p/-people + \"\" add a new people into group\n");
    printf("----------------------------------------------\n");
    printf("\n");
    printf("\n");
    return;
}
static void sql_connect(char *opt)
{
    MYSQL *conn = mysql_init(NULL);
    char value = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, (char *)&value);
    //连接数据库
    if (!mysql_real_connect(conn,opt,SQLUSER,SQLPASSWD,DATABASE, 0, NULL, 0))
    {
        perror(conn);
        exit(1);
    }
}


int main(int argc,char **argv)
{
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
            return 0;
            break;

            /* v, video */
        case 2:
        case 3:
        //    video_init();
            break;
            /* s,sql */
        case 4:
        case 5:
            sql_connect(optarg);
            break;
            /* h,http */
        case 6:
        case 7:
          //  http_connect(optarg);
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
        }
    }
    return 0;
}

