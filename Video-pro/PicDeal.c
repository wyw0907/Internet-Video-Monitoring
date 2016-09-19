
#include "include/config.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include "include/curl/curl.h"

#define APIKEY      "45ffa2a9f3f88066f27e727914804e9f"
#define APISCRT     "4K7T2MgAVl3vC72Gqv9X3QdqGw13dzmi"
#define IMGSRC
#define FACEURL     "apicn.faceplusplus.com "


void *getvideo_thread(void *arg)
{
    char *filename = AVINAME;
    IplImage* frame = NULL;
    CvCapture* capture = NULL;

    capture = cvCaptureFromCAM(-1);//open video capture

    if(!capture)
    {
        LOG("Could not initialize capturing...\n");
        return;
    }
    CvVideoWriter *writer =cvCreateVideoWriter(filename, -1, 25, cvSize(frame->width,frame->height),1);//create writer
    if(!writer)
    {
        LOG("cvCreateVideoWriter error\n");
        return;
    }

    while(1)
    {
        pthread_mutex_lock(&V_global.mutex);
        pthread_cond_wait(&V_global.cond,&V_global.mutex);

        frame = cvQueryFrame(capture);

        memcpy(V_global.VideoBuf,frame,PICBUFSIZE);

        cvWriteFrame(writer,frame);

        pthread_cond_broadcast(&V_global.cond);
        pthread_mutex_unlock(&V_global.mutex);

    }
    cvReleaseCapture(&capture);
    cvReleaseVideoWriter(&writer);
    pthread_exit((void *)0);
}


void *sendvideo_thread(void *arg)
{
//    pthread_cleanup_pop()
    int ret;
    while(1)
    {
        if(V_global.videoReq != REQUEST )
            continue;
        pthread_mutex_lock(&V_global.mutex);
        pthread_cond_wait(&V_global.cond,&V_global.mutex);

        ret = send(V_global.TcpFd,V_global.VideoBuf,PICBUFSIZE,0);
        if(ret < 0){
            LOG("send vedio to service error!\n")
        }

        pthread_cond_broadcast(&V_global.cond);
        pthread_mutex_unlock(&V_global.mutex);
    }
    pthread_exit((void *)0);
}

void *dealvideo_thread(void *arg)
{
    CURL *curl = NULL;
    CURLcode code;
    CURLFORMcode formcode;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    struct curl_slist *headerlist = NULL;
    while(1)
    {
        pthread_mutex_lock(&V_global.mutex);
        pthread_cond_wait(&V_global.cond,&V_global.mutex);
        /**......****/
        if(0)
        {
            pthread_cond_broadcast(&V_global.cond);
            pthread_mutex_unlock(&V_global.mutex);
            continue;
        }

        curl_easy_setopt(curl,CURLOPT_NOSIGNAL,0);

        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                     CURLFORM_COPYCONTENTS,APIKEY)

    }
}
