#include <stdio.h>
#include <libqhyccd/common.h>
#include <libqhyccd/interguider.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


unsigned char ImgData[3584*2574*2];
int w = 1280,h = 1024,bpp = 8,channels = 1;
int main(void)
{
    IplImage *img = cvCreateImage(cvSize(w,h),bpp,channels);
    img->imageData = (char *)ImgData;

    int ret = OpenCamera();
    openOled();
    
    if(ret > 0)
    {
        printf("found\n");
        SetExposeTime(10.0);
        SetOffset(130);
        SetGain(0);
        SetResolution(w,h);
        BeginLive();
        GetImageData(w,h,bpp,channels,ImgData); 
        cvNamedWindow("IC8300",CV_WINDOW_AUTOSIZE);
        cvShowImage("IC8300",img);
        cvWaitKey(0);
        cvDestroyWindow("IC8300");
    }
    cvReleaseImage(&img);
    return 0;
}
