#ifndef     __PICDEAL_H
#define     __PICDEAL_H

void *getvideo_thread(void *);
void *sendvideo_thread(void *);
void *dealvideo_thread(void *);
int add_people(char *);
size_t read_data(void* buffer,size_t size,size_t nmemb,void *stream);

enum{PCREATE,GROUPADD,FACEADD,FACEGET,TRAIN};

#define APIKEY      "45ffa2a9f3f88066f27e727914804e9f"
#define APISCRT     "4K7T2MgAVl3vC72Gqv9X3QdqGw13dzmi"
#define IMGSRC
#define FACEURL     "apicn.faceplusplus.com/v2"
//#define GROUPNAME   "vedio99"

#define JSONBUFSIZE 1024*8

extern char JsonBuf[JSONBUFSIZE];


#endif
