#include "ic8300.h"
#include "qhycam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv/cv.h>


extern QUsb *qhyusb;
IC8300 *ic8300;

void  IC8300::CorrectIC8300WH(int *w,int *h)
{
    if(*w <= 896 && *h <= 644)
    {
	*w = 896;
	*h = 644;
	initIC8300_896x644();
    }
    else if(*w <= 1792 && *h <= 1287)
    {
	*w = 1792;
	*h = 1287;
	initIC8300_1792x1287();
    }
    else 
    {
	*w = 3584;
	*h = 2574;
	initIC8300_3584x2574();
    }
}

void IC8300::initIC8300_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"IC8300-0";
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset = qhyusb->QCam.camOffset;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;       //unit: ms
    qhyusb->ccdreg.SKIP_TOP = 0;
    qhyusb->ccdreg.SKIP_BOTTOM = 0;
    qhyusb->ccdreg.AMPVOLTAGE = 1;
    qhyusb->ccdreg.LiveVideo_BeginLine = 0;
    qhyusb->ccdreg.AnitInterlace = 0;
    qhyusb->ccdreg.MultiFieldBIN = 0;
    qhyusb->ccdreg.TgateMode = 0;
    qhyusb->ccdreg.ShortExposure = 0;
    qhyusb->ccdreg.VSUB = 0;
    qhyusb->ccdreg.TransferBIT = 0;
    qhyusb->ccdreg.TopSkipNull = 30;
    qhyusb->ccdreg.MechanicalShutterMode = 0;
    qhyusb->ccdreg.DownloadCloseTEC = 0;
    qhyusb->ccdreg.SDRAM_MAXSIZE = 100;
    qhyusb->ccdreg.ClockADJ = 0x0000;
    qhyusb->ccdreg.CLAMP = 0;
    qhyusb->ccdreg.MotorHeating = 1;
}

void IC8300::initIC8300_896x644(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 1792;
    qhyusb->ccdreg.VerticalSize = 644;
    qhyusb->ccdreg.TopSkipPix = 0;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->QCam.cameraW = 896;
    qhyusb->QCam.cameraH = 644;
    qhyusb->QCam.P_Size = 1024;
   
}

void IC8300::initIC8300_1792x1287(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 1792;
    qhyusb->ccdreg.VerticalSize = 1287;
    qhyusb->ccdreg.TopSkipPix = 1100;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->QCam.cameraW = 1792;
    qhyusb->QCam.cameraH = 1287;
    qhyusb->QCam.P_Size = 3584*2;
}
void IC8300::initIC8300_3584x2574(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 3584;
    qhyusb->ccdreg.VerticalSize = 2574;
    qhyusb->ccdreg.TopSkipPix = 1150;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->QCam.cameraW = 3584;
    qhyusb->QCam.cameraH = 2574;
    qhyusb->QCam.P_Size = 3584*14;
}

void IC8300::ConvertIC8300DataBIN11(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
     unsigned char *Buf = NULL;

     qhyusb->SWIFT_MSBLSB(ImgData,x,y);

     Buf=(unsigned char *)malloc(x*y*2);
     
     memcpy(Buf,ImgData+TopSkipPix*2,x*y*2);
     memcpy(ImgData,Buf,x*y*2);
     free(Buf);
}



void IC8300::ConvertIC8300DataBIN22(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
    unsigned char *Buf = NULL;

    qhyusb->SWIFT_MSBLSB(ImgData,x,y );

    Buf=(unsigned char *) malloc(x*y*2);
    memcpy(Buf,ImgData+TopSkipPix*2,x*y*2);
    memcpy(ImgData,Buf,x*y*2);
    free(Buf);
}

void IC8300::ConvertIC8300DataBIN44(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
     unsigned char * Buf = NULL;
     unsigned int pix;

     qhyusb->SWIFT_MSBLSB(ImgData,x*2 ,y);

     Buf=(unsigned char *) malloc(x*y*2);

     unsigned long k=0;
     unsigned long s=TopSkipPix*2;

     while((int)k < x*y*2)
     {
         pix=(ImgData[s]+ImgData[s+1]*256+ImgData[s+2]+ImgData[s+3]*256)/2;
         if (pix>65535) pix=65535;

         Buf[k]  =qhyusb->LSB(pix);
         Buf[k+1]=qhyusb->MSB(pix);
         s=s+4;
         k=k+2;
     }

    memcpy(ImgData,Buf,x*y*2);
    free(Buf);
}

void IC8300::writec(unsigned char value)
{
	unsigned char data[1];
	data[0]=value;
        libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xbb,0x78,0,data,1, 0);
	//vendTXD_Ex("IC8300-0",0XBB,1,0x00,0x78,data);
}

void IC8300::writed(unsigned char value)
{
	unsigned char data[1];
	data[0]=value;
	 libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xbb,0x78,0x40,data,1, 0);
       //vendTXD_Ex("IC8300-0",0XBB,1,0x40,0x78,data);
}
void IC8300::oled(unsigned char buffer[])
{
	unsigned char i;
	unsigned char byte;

	writec(0xAF);
	writec(0x00);    	//set lower column address
	writec(0x10);    	//set higher column address
	writec(0xb0);
	writec(0xB0+6);    	//set page address

	for(i=0;i<128;i++)
	{
		byte=buffer[i];
		writed(byte);
	};
	writec(0x00);    	//set lower column address
	writec(0x10);    	//set higher column address
	writec(0xB0+7);    	//set page address
	for(i=128;i<255;i++)
	{
		byte=buffer[i];
		writed(byte);
	};
}

void IC8300::send2oled(char message[])
{
	unsigned char data[128*16];
	memset(data,0,128*16);

	IplImage *img = cvCreateImage(cvSize(128,16),8,1);
  img->imageData = (char *)data;
  CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,0.4,0.4,0,0,1);
	cvPutText(img,message,cvPoint(0,15),&font,CV_RGB(255,255,255));

  unsigned char data8[256];

  int e = 0;
	int s = 0;
  int k = 0;
	CvScalar pix;
  memset(data8,0,256);

  s = 0;
  e = 0;
  for(k = 0;k < 128;k++)
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
  s = 128;
  for(k = 0;k < 128;k++)
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

	oled(data8);
}
