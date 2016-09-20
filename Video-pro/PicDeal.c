
#include "include/config.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include "include/curl/curl.h"
#include "include/cJSON.h"

#define APIKEY      "45ffa2a9f3f88066f27e727914804e9f"
#define APISCRT     "4K7T2MgAVl3vC72Gqv9X3QdqGw13dzmi"
#define IMGSRC
#define FACEURL     "apicn.faceplusplus.com/v2"
//#define GROUPNAME   "vedio99"

#define JSONBUFSIZE 1024*4
static char JsonBuf[JSONBUFSIZE] = {0};

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


size_t read_data(void* buffer,size_t size,size_t nmemb,void *stream)
{
 //   printf("%s\n",buffer);
    memcpy(JsonBuf,buffer,size*nmemb);
    return size*nmemb;
}

int jsonparse_identify()
{
    return 0;
}

void *dealvideo_thread(void *arg)
{
    CURL *curl = NULL;
    CURLcode code;
 //   CURLFORMcode formcode;
    int timeout = 10;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
 //   struct curl_slist *headerlist = NULL;
    while(1)
    {
        sleep(10);
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
                     CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                     CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"img",\
                     CURLFORM_FILECONTENT,V_global.VideoBuf,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"group_name",\
                     CURLFORM_COPYCONTENTS,V_global.group,CURLFORM_END);

        curl = curl_easy_init();
        if(!curl){
            LOG("curl init error!\n")
            continue;
        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/recognition/identify");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        //curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr); //这是write_data的第四个参数>值
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

        code = curl_easy_perform(curl);  //这里会阻塞至回调函数执行完毕或者timeout
        if(code != CURLE_OK){
            LOG("curl easy perform error!\n")
        }
        jsonparse_identify();

        curl_easy_cleanup(curl);
        curl_formfree(post);
        pthread_cond_broadcast(&V_global.cond);
        pthread_mutex_unlock(&V_global.mutex);
    }

    pthread_exit((void *)0);
}

enum{PCREATE,GROUPADD,FACEADD,FACEGET,TRAIN};
int jsonparse_addpeople(char *name,char *groupname,char *faceid,int flag)
{
    if(JsonBuf[0] == '\0')
        return -1;
    cJSON *cJson = cJSON_Parse(JsonBuf);
    cJSON *c1 = NULL,*c2 = NULL;
    switch(flag){
        case PCREATE:
        {
            c1 = cJSON_GetObjectItem(cJson,"person_name");
            if(c1){
                if(c1->type == cJSON_String){
                    if(strcmp(c1->valuestring,name)==0){
                        LOG("create person success!\n")
                        return 0;
                    }
                }
            }
        }
        break;
        case GROUPADD:
        {
            c1 = cJSON_GetObjectItem(cJson,"success");
            if(c1){
                if(c1->type == cJSON_True){
                    LOG("add person success!\n")
                    return 0;
                }
            }
        }
        break;
        case FACEGET:
        {
            c1 = cJSON_GetObjectItem(cJson,"face");
            if(c1){
                if(c1->type == cJSON_Array){
                    int c1_size = cJSON_GetArraySize(c1);
                    int i = 0;
                    for(i=0;i<c1_size;i++)
                    {
//                        printf("at %d\n",i);
//                        sleep(1);
                        c2 = cJSON_GetArrayItem(c1,i);
                        cJSON *c3 = NULL;
                        if(c2->type == cJSON_Object)
                        {
                            c3 = cJSON_GetObjectItem(c2,"face_id");
                            if(c3){
                                if(c3->type == cJSON_String){
                                    LOG("identify face img success!\n")
                                    //sleep(1);
                                    strcpy(faceid,c3->valuestring);
                                    return 0;
                                }

                            }
                        }
                    }
                }
            }
        }
        break;
        case FACEADD:
        {
            c1 = cJSON_GetObjectItem(cJson,"success");
            if(c1){
                if(c1->type == cJSON_True){
                    LOG("add faces to person success!\n")
                    return 0;
                }
            }
        }
        break;
        case TRAIN:
        {
            c1 = cJSON_GetObjectItem(cJson,"session_id");
            if(c1){
                if(c1->type == cJSON_String){
                    LOG("train identify success!\n")
                    return 0;
                }
            }

        }
        break;
    }
    cJSON_Delete(cJson);
    return -1;

}

int add_people(char *name)
{
    int ret;
    char faceid[3][64] = {{0},{0},{0}};
    CURL *curl = NULL;
    CURLcode code;
    int timeout = 10;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;

    curl_easy_setopt(curl,CURLOPT_NOSIGNAL,0);

    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"person_name",\
                 CURLFORM_COPYCONTENTS,name,CURLFORM_END);

    curl = curl_easy_init();
    if(!curl){
        LOG("curl init error!\n")
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/person/create");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

    code = curl_easy_perform(curl);
    if(code != CURLE_OK){
        LOG("curl perform error!\n")
        return -1;
    }

    ret = jsonparse_addpeople(name,NULL,NULL,PCREATE);
    if(ret < 0){
        LOG("create people error!\n")
        return -1;
    }
    curl_easy_cleanup(curl);

    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"person_name",\
                 CURLFORM_COPYCONTENTS,name,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"group_name",\
                 CURLFORM_COPYCONTENTS,V_global.group,CURLFORM_END);

    curl = curl_easy_init();
    if(!curl){
        LOG("curl init error!\n")
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/group/add_person");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

    code = curl_easy_perform(curl);
    if(code != CURLE_OK){
        LOG("curl perform error!\n")
        return -1;
    }

    ret = jsonparse_addpeople(name,V_global.group,NULL,GROUPADD);
    if(ret < 0){
        LOG("add people error!\n")
        return -1;
    }

    curl_easy_cleanup(curl);

    int i = 0;
    for(i = 0;i<3;i++)
    {
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                     CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                     CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
     //   curl_formadd(&post,&last,CURLFORM_COPYNAME,"img",
     //                CURLFORM_FILECONTENT,V_global.VideoBuf,CURLFORM_END);
         curl_formadd(&post,&last,CURLFORM_COPYNAME,"img",\
                      CURLFORM_FILE,"text.jpg",CURLFORM_END);
         curl_formadd(&post,&last,CURLFORM_COPYNAME,"mode",\
                      CURLFORM_COPYCONTENTS,"oneface",CURLFORM_END);
        curl = curl_easy_init();
        if(!curl){
            LOG("curl init error!\n")
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/detection/detect");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

        code = curl_easy_perform(curl);
        if(code != CURLE_OK){
            LOG("curl perform error!\n")
            return -1;
        }
        ret = jsonparse_addpeople(NULL,NULL,faceid[i],FACEGET);
        if(ret < 0){
            LOG("identify face img error!\n")
            return -1;
        }
//        printf("111 : %d\n",i);
//        sleep(1);
        printf("faceid : %s\n",faceid[i]);
    }

    curl_easy_cleanup(curl);

    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"person_name",\
                 CURLFORM_COPYCONTENTS,name,CURLFORM_END);
    char faceids[200] = {0};
    sprintf(faceids,"%s,%s,%s",faceid[0],faceid[1],faceid[2]);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"face_id",\
                 CURLFORM_COPYCONTENTS,faceids,CURLFORM_END);

    curl = curl_easy_init();
    if(!curl){
        LOG("curl init error!\n")
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/person/add_face");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

    code = curl_easy_perform(curl);
    if(code != CURLE_OK){
        LOG("curl perform error!\n")
        return -1;
    }

    ret = jsonparse_addpeople(name,V_global.group,NULL,FACEADD);
    if(ret < 0){
        LOG("add face to people error!\n")
        return -1;
    }

    curl_easy_cleanup(curl);


    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"group_name",\
                 CURLFORM_COPYCONTENTS,V_global.group,CURLFORM_END);


    curl = curl_easy_init();
    if(!curl){
        LOG("curl init error!\n")
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/train/identify");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值

    code = curl_easy_perform(curl);
    if(code != CURLE_OK){
        LOG("curl perform error!\n")
        return -1;
    }

    ret = jsonparse_addpeople(NULL,NULL,NULL,TRAIN);
    if(ret < 0){
        LOG("add face to people error!\n")
        return -1;
    }

    curl_easy_cleanup(curl);



    curl_formfree(post);


    return 0;
}

