#include "include/config.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>

int video_init(char *opt)
{
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
        fprintf(stderr,"Could not initialize capturing...\n");
        return;
    }

    cvNamedWindow("Video Capture", 1);//create show window

    frame = cvQueryFrame(capture);

    CvVideoWriter *writer =cvCreateVideoWriter(filename, -1, 25, cvSize(frame->width,frame->height),1);//create writer

    //  CvVideoWriter *writer = cvCreateVideoWriter("camera.avi",CV_FOURCC('D','I','V','X'),25,cvSize(frame->width,frame->height));直接指定视频格式时出错。

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
