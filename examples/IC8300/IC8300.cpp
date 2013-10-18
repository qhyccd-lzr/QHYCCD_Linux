#include <stdio.h>
#include <libqhyccd/common.h>
#include <libqhyccd/interguider.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


unsigned char ImgData[3584*2574*2];
int w = 800,h = 298,bpp = 16,channels = 1;
int main(void)
{
    IplImage *img = cvCreateImage(cvSize(w,h),bpp,channels);
    img->imageData = (char *)ImgData;

    int ret = OpenCameraByID(DEVICETYPE_QHY6);
    cvNamedWindow("IC8300",CV_WINDOW_AUTOSIZE);
    if(ret > 0)
    {
        printf("ret:%d\n",ret);
 
        //SetSpeed(false);   
        //SetExposeTime(100000.0);
        //SetUSBTraffic(100);
        //SetGain(30);
        //SetOffset(135);
        //SetResolution(w,h);
 
        BeginLive();

        //GetImageData(w,h,bpp,channels,ImgData); 
        cvShowImage("IC8300",img);
        cvWaitKey(0);
       
        cvDestroyWindow("IC8300");

    }
    cvReleaseImage(&img);
    return 0;
}
