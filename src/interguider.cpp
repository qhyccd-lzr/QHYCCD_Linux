#include   <stdio.h> 
#include   <stdlib.h> 
#include   <opencv/cv.h>
#include   <opencv/highgui.h>

#include   "common.h"
#include   "cssl.h"
#include   "interguider.h"

//ttyS0 control handle
cssl_t *serial;

//interGuider oled temprory memory
unsigned char data8[533];

//From interGuider to ARM command
char ch;
//flag for new coming command
int flagch = 0;

//callback for receive data from ttyS0
static void callback(int id,uint8_t *buf,int length);

int openOled() 
{ 

    cssl_start();
    
    serial=cssl_open("/dev/ttyS0",callback,0,
		     9600,8,0,1);

    if (!serial) 
    {
        #ifdef QHYCCD_DEBUG
	printf("%s\n",cssl_geterrormsg());
        #endif
	return -1;
    }
    return 0;
} 
void closeOled()
{
    cssl_close(serial);
    cssl_stop();
}

void send2Speaker()
{
    unsigned char buf[7];
    buf[0] = '#';
    buf[1] = '#';
    buf[2] = 'A';
    buf[5] = '&';
    buf[6] = '&';
    cssl_putdata(serial,buf,7);
}

void interGuiderOled(int width,int height,int bpp,int channels,unsigned char *showdata)
{
    unsigned char data[132 * 32];
    memset(data,0,132 * 32);

    //this is final image to oled
    IplImage *img = cvCreateImage(cvSize(132,32),8,1);

    //this is a temp image to load source image
    IplImage *srcimg = cvCreateImage(cvSize(width,height),bpp,1);

    //ensure image is a mono data
    if(channels == 3)
    {
        IplImage *colorimg = cvCreateImage(cvSize(width,height),bpp,3);
        colorimg->imageData = (char *)showdata;
        cvCvtColor(colorimg,srcimg,CV_BGR2GRAY); 
        cvReleaseImage(&colorimg);       
    }
    else if(channels == 1)
    {
        srcimg->imageData = (char *)showdata;
    }
  
    //first to resize the source image to 132*32
    cvResize(srcimg,img,CV_INTER_NN);
    
    //just flip the image to fit oled
    cvFlip(img,0,-1);

    int headlen = 3;
    int e = 0;
    int s = 0;
    int k = 0;
 
	CvScalar pix;
    memset(data8,0,533);

    data8[0] = '#';
    data8[1] = '#';
    data8[2] = 'B';
    e = 0;
    s = headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 8)
        {
            pix = cvGet2D(img,e,k);

            if(pix.val[0] > 128)
            {
                data8[s] = data8[s] | (1 << e);
            }
            e++;
         }
         e = 0;
         s++;
    }

    e = 8;
    s = 132 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 16)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%8));
	    }
            e++;
         }
	 e = 8;
         s++;
    }

    e = 16;
    s = 264 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 24)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%16));
	    }
	    e++;
        }
        e = 16;
        s++;
    }
    e = 24;
    s = 396 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 32)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e % 24));
	    }
	    e++;
        }
        e = 24;
        s++;
    }
    data8[531] = '&';
    data8[532] = '&';
    cssl_putdata(serial,data8,533);
    cvReleaseImage(&srcimg);
    cvReleaseImage(&img);
}

void sendStr2Oled(char message[])
{
    unsigned char data[132 * 32];
    memset(data,0,132 * 32);

    IplImage *img = cvCreateImage(cvSize(132,32),8,1);
    img->imageData = (char *)data;
    CvFont font;
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,1,1,0,0,1);
    cvPutText(img,message,cvPoint(0,31),&font,CV_RGB(255,255,255));
    //just flip the image to fit oled
    cvFlip(img,0,-1);

    int headlen = 3;
    int e = 0;
    int s = 0;
    int k = 0;
 
    CvScalar pix;
    memset(data8,0,533);

    data8[0] = '#';
    data8[1] = '#';
    data8[2] = 'B';
    e = 0;
    s = headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 8)
        {
            pix = cvGet2D(img,e,k);

            if(pix.val[0] > 128)
            {
                data8[s] = data8[s] | (1 << e);
            }
            e++;
         }
         e = 0;
         s++;
    }

    e = 8;
    s = 132 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 16)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%8));
	    }
            e++;
         }
	 e = 8;
         s++;
    }

    e = 16;
    s = 264 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 24)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e%16));
	    }
	    e++;
        }
        e = 16;
        s++;
    }
    e = 24;
    s = 396 + headlen;
    for(k = 0;k < 132;k++)
    {
        while(e < 32)
	{
            pix = cvGet2D(img,e,k);

	    if(pix.val[0] > 128)
	    {
                data8[s] = data8[s] | (1 << (e % 24));
	    }
	    e++;
        }
        e = 24;
        s++;
    }
    data8[531] = '&';
    data8[532] = '&';
    cssl_putdata(serial,data8,533);
    cvReleaseImage(&img);
} 


char readOled()
{
    if(flagch == 1)
    {
        flagch = 0;
        return ch;
    }
    else
    {
        return '\0';
    } 
}

static void callback(int id,uint8_t *buf,int length)
{
    ch = buf[0];
    #ifdef QHYCCD_DEBUG
    printf("%c\n",buf[0]);
    #endif
    flagch = 1;

}

