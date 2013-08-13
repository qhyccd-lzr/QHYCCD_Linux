#include <stdio.h>
#include <common.h>
#include <oledcontrol.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


unsigned char ImgData[3584*2574*2];
int w = 132,h = 32,bpp = 8,channels = 1;
int main(void)
{
    IplImage *img = cvCreateImage(cvSize(w,h),bpp,channels);
    img->imageData = (char *)ImgData;

    int ret = OpenCamera();
    if(ret > 0)
    {
        SetExposeTime(1000000.0);
        //SetResolution(w,h);
        BeginLive();
        GetImageData(w,h,bpp,channels,ImgData); 
        openoled();
        interGuiderOled(img->width,img->height,img->depth,img->nChannels,ImgData);
        getchar();
        send2speaker();
        getchar();
        printf("%c\n",readoled());
        closeoled();
        cvSaveImage("test.bmp",img,0);
        //cvNamedWindow("IC8300",CV_WINDOW_AUTOSIZE);
        //cvShowImage("IC8300",img);
        //cvWaitKey(0);
        //cvDestroyWindow("IC8300");
    }
    cvReleaseImage(&img);
    return 0;
}
