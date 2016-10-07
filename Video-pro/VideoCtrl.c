#include "include/config.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>


int video_init(char *opt)
{
    return 0;
}

int jsonparse_reset()
{
    cJSON *cJson = cJSON_Parse(JsonBuf);
    if(!cJson){
        LOG("cJSOn_parse error!@\n")
        return -1;
    }
    cJSON *c1 = NULL;

    c1 = cJSON_GetObjectItem(cJson,"success");
    if(c1){
        if(c1->type == cJSON_True){
            cJSON_Delete(cJson);
            return 0;
        }
    }
    cJSON_Delete(cJson);
    return -1;
}


int video_reset()
{
    CURL *curl = NULL;
    CURLcode code;
 //   CURLFORMcode formcode;
    int timeout = 10;
    int ret;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;

    curl_easy_setopt(curl,CURLOPT_NOSIGNAL,0);

    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"group_name",\
                 CURLFORM_COPYCONTENTS,V_global.group,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"person_id",\
                 CURLFORM_COPYCONTENTS,"all",CURLFORM_END);
    /*delect all persons in the group*/


    curl = curl_easy_init();
    if(!curl){
        LOG("curl init error!\n")
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/group/remove_person");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值



    code = curl_easy_perform(curl);  //这里会阻塞至回调函数执行完毕或者timeout
    if(code != CURLE_OK){
        LOG("curl easy perform error!\n")
        return -1;
    }


    ret = jsonparse_reset();
    if(ret == 0){
        LOG("remove all person in group success\n")
    }
    else{
        LOG("remvoe all person in group false\n")

        curl_easy_cleanup(curl);
        curl_formfree(post);
        return -1;
    }

    curl_easy_cleanup(curl);
    curl_formfree(post);


    MYSQL_RES *sqlres;
    char sqlcmd[32] = {0};
    sprintf(sqlcmd,"select name from videouser where videoid=%s",V_global.VideoId);
    mysql_query(V_global.conn,sqlcmd);
    sqlres = mysql_store_result(V_global.conn);
    if(!sqlres){
        LOG("select table error!\n")
        return -1;
    }
    MYSQL_ROW sqlrow;
    sqlrow = mysql_fetch_row(sqlres);
    if(!sqlrow){
        LOG("this video has not setted\n")
    //    help(NULL);
        return -1;
    }
    sprintf(sqlcmd,"update videouser set videoid='%s' where videoid='%s'",\
            "null",V_global.VideoId);
    mysql_query(V_global.conn,sqlcmd);
    sqlres = mysql_store_result(V_global.conn);
    if(mysql_affected_rows(V_global.conn)!=1){
        LOG("update videouser error!\n")
        return -1;
    }
    return 0;
}

//off_line mode
void off_line_process()
{
    if(V_global.mode != OFFLINE)
        return;

    char *filename = AVINAME;
    IplImage* frame = NULL;
    CvCapture* capture = NULL;

    capture = cvCaptureFromCAM(-1);//open video capture

    if(!capture)
    {
        LOG("Could not initialize capturing...\n");
        return;
    }

    cvNamedWindow("Video Capture", 1);//create show window

    frame = cvQueryFrame(capture);

    CvVideoWriter *writer =cvCreateVideoWriter(filename, CV_FOURCC('D','I','V','X'), 25, cvSize(frame->width,frame->height),1);//create writer
    if(!writer)
    {
        LOG("cvCreateVideoWriter error\n");
        return;
    }
//      CvVideoWriter *writer = cvCreateVideoWriter("camera.avi",CV_FOURCC('D','I','V','X'),25,cvSize(frame->width,frame->height));直接指定视频格式时出错。

    while(capture)
    {
        frame = cvQueryFrame(capture);

        cvWriteFrame(writer,frame);

        cvShowImage("Video Capture",frame);

        if(cvWaitKey(1) == 'q')
            break;
    }

    cvReleaseCapture(&capture);
    cvReleaseVideoWriter(&writer);
    cvDestroyWindow("Video Capture");

    exit(0);
}
