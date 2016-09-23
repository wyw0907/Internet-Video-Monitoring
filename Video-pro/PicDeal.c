
#include "include/config.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include "include/curl/curl.h"


char JsonBuf[JSONBUFSIZE] = {0};

void *getvideo_thread(void *arg)
{
    char *filename = AVINAME;
    IplImage* frame = NULL;
    CvCapture* capture = NULL;

    capture = cvCaptureFromCAM(-1);//open video capture

    if(!capture)
    {
        LOG("Could not initialize capturing...\n");
        exit(1);
    }
    CvVideoWriter *writer =cvCreateVideoWriter(filename, CV_FOURCC('D','I','V','X'), 25, cvSize(frame->width,frame->height),1);//create writer
    if(!writer)
    {
        LOG("cvCreateVideoWriter error\n");
        exit(1);
    }

    while(1)
    {
        pthread_mutex_lock(&V_global.mutex);


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
        while(V_global.videoReq != REQUEST );
        pthread_mutex_lock(&V_global.mutex);
        pthread_cond_wait(&V_global.cond,&V_global.mutex);

        ret = send(V_global.TcpFd,V_global.VideoBuf,PICBUFSIZE,0);
        if(ret < 0){
            LOG("send vedio to service error!\n")
        }


        pthread_mutex_unlock(&V_global.mutex);
    }
    pthread_exit((void *)0);
}


size_t read_data(void* buffer,size_t size,size_t nmemb,void *stream)
{
#ifdef __DEBUG
    printf("%s\n",buffer);
#endif
    memcpy(JsonBuf,buffer,size*nmemb);
    return size*nmemb;
}

static int jsonparse_identify(char *name,double *confidence,char *faceid)
{
    if(JsonBuf[0] == '\0')
        goto __err;
    cJSON *cJson = cJSON_Parse(JsonBuf);
    if(!cJson){
        LOG("cJSON_Parse error!\n")
        return -1;
    }
    cJSON *c1 = NULL,*c2 = NULL,*c3 = NULL;
    c1 = cJSON_GetObjectItem(cJson,"face");
    if(!c1)
        goto __err;
    if(c1->type != cJSON_Array)
        goto __err;
    int c1_size = cJSON_GetArraySize(c1);
    if(c1_size <= 0)
        goto __err;
    int i = 0;
 //   for(i=0;i<c1_size;i++)  这里错了，不需要遍历，只需要第一个，置信度最高的
    {
        c2 = cJSON_GetArrayItem(c1,i);
        if(!c2)
            goto __err;
        if(c2->type != cJSON_Object)
            goto __err;
        c3 = cJSON_GetObjectItem(c2,"candidate");
        if(!c3){
            goto __err;
        }
        if(c3->type != cJSON_Array){
            goto __err;
        }
        cJSON *c4 = NULL,*c5=NULL;
        int c3_size = cJSON_GetArraySize(c3);
        if(c3_size == 0){
            LOG("identify : nomatched person!\n")
            cJSON_Delete(cJson);
            return 1;
        }
        int j = 0;
        for(j=0;j<c3_size;j++)
        {
            c4 = cJSON_GetArrayItem(c3,i);
            if(!c4)
                goto __err;
            if(c4->type != cJSON_Object)
                goto __err;
            c5 = cJSON_GetObjectItem(c4,"confidence");
            if(!c5){
                goto __err;
            }
            if(c5->type != cJSON_Number){
               goto __err;
            }
    //        printf("confidencd %d : %f\n",j,c5->valuedouble);
            *confidence = c5->valuedouble;

            c5 = cJSON_GetObjectItem(c4,"person_name");
            if(!c5){
                goto __err;
            }
            if(c5->type != cJSON_String){
                goto __err;
            }
  //          printf("person_name %d : %s\n",j,c5->valuestring);
            strcpy(name,c5->valuestring);
        }

        c3 = cJSON_GetObjectItem(c2,"face_id");
        if(!c3)
            goto __err;
        if(c3->type != cJSON_String)
            goto __err;
  //      printf("face_id : %s\n",c3->valuestring);
        strcpy(faceid,c3->valuestring);
    }
    return 0;
__err:
    LOG("face identufy error!\n")
    cJSON_Delete(cJson);
    return -1;
}

void *dealvideo_thread(void *arg)
{
    CURL *curl = NULL;
    CURLcode code;
 //   CURLFORMcode formcode;
    int timeout = 10;
    int ret;
    struct curl_httppost *post = NULL;
    struct curl_httppost *last = NULL;
    char name[32] = {0};
    char faceid[40] = {0};
    double confidence = 0;
    while(1)
    {
 //       sleep(10);
        pthread_mutex_lock(&V_global.mutex);
  //      pthread_cond_wait(&V_global.cond,&V_global.mutex);
        /**......****/
        if(0)
        {
            pthread_mutex_unlock(&V_global.mutex);
            continue;
        }
        curl_easy_setopt(curl,CURLOPT_NOSIGNAL,0);

        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                     CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                     CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
     //   curl_formadd(&post,&last,CURLFORM_COPYNAME,"img",
       //              CURLFORM_FILECONTENT,V_global.VideoBuf,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"img",\
                     CURLFORM_FILE,"text.jpg",CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"group_name",\
                     CURLFORM_COPYCONTENTS,V_global.group,CURLFORM_END);
        curl_formadd(&post,&last,CURLFORM_COPYNAME,"mode",\
                     CURLFORM_COPYCONTENTS,"oneface",CURLFORM_END);

        curl = curl_easy_init();
        if(!curl){
            LOG("curl init error!\n")
            continue;
        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL,FACEURL"/recognition/identify");
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,read_data); //对返回的数据进行操作的函数地址
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,NULL); //这是write_data的第四个参数值


        pthread_mutex_unlock(&V_global.mutex);   //接下来是和face++的交互，所以直接把线程锁释放

        code = curl_easy_perform(curl);  //这里会阻塞至回调函数执行完毕或者timeout
        if(code != CURLE_OK){
            LOG("curl easy perform error!\n")
            continue;
        }
        ret = jsonparse_identify(name,&confidence,faceid);
        if(ret < 0){
            LOG("identify error!\n")
        }
        else if(ret == 1)
            LOG("indetify : nomatched person!\n")
        else if(ret == 0){
            LOG("identified !\n")
            printf("%s matched %s : %f\n",faceid,name,confidence);
        }

        curl_easy_cleanup(curl);
    }
    curl_formfree(post);
    pthread_exit((void *)0);
}

static int jsonparse_addpeople(char *name,char *groupname,char *faceid,int flag)
{
    if(JsonBuf[0] == '\0')
        return -1;
    cJSON *cJson = cJSON_Parse(JsonBuf);
    if(!cJson){
        LOG("cJSON_parse error!\n")
        return -1;
    }
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
            c1 = cJSON_GetObjectItem(cJson,"error");
            if(c1){
                if(c1->type == cJSON_String){
                    if(strcmp(c1->valuestring,"NAME_EXIT")==0)
                        LOG("the person is cerated !\n")
                        return 0;
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
    char faceid[3][40] = {{0},{0},{0}};
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
#ifdef __DEBUG
        printf("faceid : %s\n",faceid[i]);
#endif
    }

    curl_easy_cleanup(curl);

    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_key",\
                 CURLFORM_COPYCONTENTS,APIKEY,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"api_secret",\
                 CURLFORM_COPYCONTENTS,APISCRT,CURLFORM_END);
    curl_formadd(&post,&last,CURLFORM_COPYNAME,"person_name",\
                 CURLFORM_COPYCONTENTS,name,CURLFORM_END);
    char faceids[143] = {0};
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

