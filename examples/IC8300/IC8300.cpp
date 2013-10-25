#include <stdio.h>
#include <libqhyccd/common.h>
#include <libqhyccd/interguider.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


unsigned char ImgData[3584*2574*3];
int w = 3584,h = 2574,bpp = 16,channels = 1;
int main(void)
{
    IplImage *img = cvCreateImage(cvSize(w,h),bpp,channels);
    img->imageData = (char *)ImgData;

    int ret = OpenCameraByID(DEVICETYPE_QHY16000);
    cvNamedWindow("IC8300",CV_WINDOW_AUTOSIZE);
    if(ret > 0)
    {
        printf("ret:%d\n",ret);
 
        SetSpeed(false);   
        SetExposeTime(1000.0);
        SetUSBTraffic(100);
        SetGain(200);
        SetOffset(135);
        SetResolution(w,h);
 
        BeginLive();
        while(1)
        {
            GetImageData(w,h,bpp,channels,ImgData); 
            cvShowImage("IC8300",img);
            cvWaitKey(30);
        }
       
        cvDestroyWindow("IC8300");

    }
    cvReleaseImage(&img);
    return 0;
}
