#include <stdio.h>
#include <libqhyccd/common.h>
#include <libqhyccd/interguider.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


unsigned char ImgData[3584*2574*2];
int w = 640,h = 480,bpp = 8,channels = 1;
int main(void)
{
    IplImage *img = cvCreateImage(cvSize(w,h),bpp,channels);
    img->imageData = (char *)ImgData;

    int ret = OpenCamera(DEVICETYPE_QHY9);
    openOled();
    cvNamedWindow("IC8300",CV_WINDOW_AUTOSIZE);
    
    if(ret > 0)
    {
        printf("found\n");
        
        SetExposeTime(100000.0);
        SetUSBTraffic(100);
        SetGain(300);
        SetResolution(w,h);
        BeginLive();

        GetImageData(w,h,bpp,channels,ImgData); 
        cvShowImage("IC8300",img);
        cvWaitKey(0);
       
        cvDestroyWindow("IC8300");
    }
    cvReleaseImage(&img);
    return 0;
}
