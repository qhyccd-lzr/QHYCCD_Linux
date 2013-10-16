/*
 * common.c
 *
 * libqhyusb program
 * 
 * Copyright (C) 2013 QHYCCD Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <opencv/cv.h>

#include "qhy5lii.h"
#include "qhy5ii.h"
#include "qhy9.h"
#include "qhy9l.h"
#include "qhy22.h"
#include "QHYCAM.h"


#ifdef __cplusplus
extern "C" 
{
#endif


QHYCCD QCam;
extern CCDREG ccdreg;
extern int GainTable[73];
int liveabort = 0;

int OpenCamera(void)
{   
    qhyccd_device *dev;
    qhyccd_device_model *model;

    qhyccd_init();
    
    ssize_t n_device;
    n_device = qhyccd_get_device_list(&QCam.device_list);
    #ifdef QHYCCD_DEBUG
    printf("Total devices %d\n",n_device);
    #endif
    
    int i;
    for(i = 0;i < n_device;i++)
    {
        model = qhyccd_get_device_model(QCam.device_list[i]);
        
        #ifdef QHYCCD_DEBUG
        printf("model is %d\n",model->model_id);
        #endif
        dev = QCam.device_list[i];
        qhyccd_open(dev,&QCam.ccd_handle);

        if(model->model_id == QHYCCD_QHY5II)
        {            
            unsigned char buf[16];
            EepromRead(0x10,buf,16);
            if(buf[1] == 1)
                QCam.isColor = true;
            else 
                QCam.isColor = false;
            //(buf[1] == 1)? QCam.isColor = true:QCam.isColor = false;
            if(buf[0] == 6)
            {
                QCam.CAMERA = DEVICETYPE_QHY5LII;   
                InitCamera();
                return DEVICETYPE_QHY5LII;
            }
            else if(buf[0] == 1)
            {
                QCam.CAMERA = DEVICETYPE_QHY5II;
                InitCamera();
                return DEVICETYPE_QHY5II;
            }
        }
        else if(model->model_id == QHYCCD_QHY9)
        {
            QCam.CAMERA = DEVICETYPE_QHY9;
            InitCamera();
            return DEVICETYPE_QHY9;
        }
        else if(model->model_id == QHYCCD_QHY9L)
        {
            QCam.CAMERA = DEVICETYPE_QHY9L;
            InitCamera();
            return DEVICETYPE_QHY9L;
        }
        else if(model->model_id == QHYCCD_QHY22)
        {
            QCam.CAMERA = DEVICETYPE_QHY22;
            InitCamera();
            return DEVICETYPE_QHY22;
        }
    }
   
    return DEVICETYPE_UNKOWN;   
}

void CloseCamera(void)
{
    qhyccd_free_device_list(QCam.device_list);
    qhyccd_close(QCam.ccd_handle);
}

void InitCamera(void)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        unsigned char buf[4];
        libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xc1,0,0,buf,4, 0);
        SetResolution(1280,960);
        SetExposureTime_QHY5LII(1000);
        SetUSBTraffic(30);
        SetSpeed(false);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetResolution(1280,1024);
	SetExposureTime_QHY5II(1000);
	SetUSBTraffic(30);
	SetSpeed(false);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        SetExposeTime(10.0);
        SetGain(30);
        SetOffset(120);
	SetSpeed(false);
        SetResolution(3584,2574);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	SetExposeTime(10);
    	SetGain(0);
    	SetOffset(120);
    	SetSpeed(false);
    	SetResolution(3072,2240);
    }

    InitOthers();
}

void InitOthers(void)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5II || QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        QCam.transferBit = 8;//8:位传输模式 16:16位传输模式
        QCam.bin = 11;//软件上BIN模式，11:原始模式 22:2x2合并模式
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
    {
        QCam.transferBit = 16; 
    }
    QCam.isColor = false;//相机是否彩色 true:彩色 false:黑白
    QCam.transferspeed = 0;//传输速度0:低速 1:高速
    liveabort = 0;
}

void SetBin(int w,int h)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II )
    {
	if(w == 1 && h == 1)
	{
	    QCam.bin = 11;
	}
	else if(w == 2 && h == 2)
	{
	    QCam.bin = 22;
	}
    }
    QCam.bin = 11;
}

void SetTransferBit(int Bit)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        QCam.transferBit = Bit;
	if(Bit == 16)
	{
	    Set14Bit(1);
	}
	else 
	{
	    Set14Bit(0);
	}
	if(QCam.transferspeed == 1)
	{
	    SetSpeed(true);
	}
	else
	{
	    SetSpeed(false);
	}
	SetResolution(QCam.cameraW,QCam.cameraH);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
    {
        QCam.transferBit = 16;
    }
    else
    {
        QCam.transferBit = 8;
    }
}

void SetExposeTime(double exptime)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetExposureTime_QHY5LII(exptime);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetExposureTime_QHY5II(exptime);
    }
    ccdreg.Exptime = exptime;       //unit: ms
    QCam.camTime = exptime;
}

void SetGain(unsigned short gain)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetQHY5LIIGain(gain);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetQHY5IIGain(gain);
    }
    ccdreg.Gain = gain;
    QCam.camGain = gain;
    
}

void SetOffset(unsigned char offset)
{
	  ccdreg.Offset = offset;
    QCam.camOffset = offset;
}

void SetResolution(int x,int y)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        CorrectQHY5LIIWH(&x,&y);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        CorrectQHY5IIWH(&x,&y);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9)
    {
        CorrectQHY9WH(&x,&y);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        CorrectQHY9LWH(&x,&y);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY22)
    {
        CorrectQHY22WH(&x,&y);
    }
 
    QCam.cameraW = x;
    QCam.cameraH = y;
    SetExposeTime(QCam.camTime);
    SetGain(QCam.camGain);
    if(QCam.isColor)
    {
        SetWBGreen(QCam.wbgreen);
        SetWBRed(QCam.wbred);
        SetWBBlue(QCam.wbblue);
    }
}


void SetSpeed(bool isHigh)
{
    //isHigh?QCam.transferspeed = 1:QCam.transferspeed = 0;
    if(isHigh)
        QCam.transferspeed = 1;
    else
        QCam.transferspeed = 0;

    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        if(isHigh)
	{
	    if(QCam.transferBit == 16)
	    {
		SetSpeedQHY5LII(1);
	    }
	    else                             
	    {
		SetSpeedQHY5LII(2);
	    }
        }
        else 
        {
	    if(QCam.transferBit == 16)
	    {
	        SetSpeedQHY5LII(0);
	    }
	    else           
	    {
		SetSpeedQHY5LII(1);
	    }
	}
    }
}


void SetUSBTraffic(int i)
{
    if (QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II) 
    {
        if (QCam.cameraW == 1280)
	    I2CTwoWrite(0x300c, 1650 + i*50);
	else
	    I2CTwoWrite(0x300c, 1388 + i*50);
    }
}

void SetWBBlue(int blue)
{
    QCam.wbred = blue;

    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(QCam.camGain,QCam.wbred,QCam.wbblue);
    }
}

void SetWBGreen(int green)
{
    QCam.wbgreen = green;

    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(QCam.camGain,QCam.wbred,QCam.wbblue);
    }
}

void SetWBRed(int red)
{
    QCam.wbred = red;

    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(QCam.camGain,QCam.wbred,QCam.wbblue);
    }
}


void BeginLive(void)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II ||
       QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L ||
       QCam.CAMERA == DEVICETYPE_QHY22)
    {
        int Total_P,PatchNumber;
        sendRegisterQHYCCDOld(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
	beginVideo(QCam.ccd_handle);
    }
    liveabort = 0;
}

void Bin2x2(unsigned char *ImgData,int w,int h)
{
    int i,j;
    unsigned long temp = 0;
    int s = 0;

    if(QCam.transferBit == 8)
    {
        for(j = 0;j < h;j+=2)
	{
	    for(i = 0;i < w;i+=2)
	    {
	        temp = ImgData[i+(j)*w] + ImgData[i+1+(j)*w] + ImgData[i+(j+1)*w] + ImgData[i+1+(j+1)*w];
		if(temp>255)
		{
		    ImgData[s++] = 255;
		}
	        else
		{
		    ImgData[s++] = (unsigned char)temp;
		}
		temp = 0;
	    }
	}
    }
    else if(QCam.transferBit == 16)
    {
        for(j = 0;j < h;j+=2)
	{
	    for(i = 0;i < w*2;i+=4)
	    {
                temp = ImgData[i + (j) * w * 2] + ImgData[ i + 1 + (j) * w * 2] * 256
				+ ImgData[i + 2 + (j) * w * 2] + ImgData[i+3+(j)*w*2] * 256
				+ ImgData[i + (j + 1) * w * 2] + ImgData[i + 1 + (j + 1) * w * 2] * 256
				+ ImgData[i+ 2 + (j + 1) * w * 2] + ImgData[i + 3 + (j + 1) * w * 2] * 256;
		if(temp>65535)
		{
		    ImgData[s] = 0xff;
		    ImgData[s+1] = 0xff;
		    s += 2;
		}
		else
		{
		    ImgData[s] = (unsigned char)(temp % 256);
		    ImgData[s+1] = (unsigned char)(temp / 256);
		    s += 2;
		}
		temp = 0;
	    }
        }
    }
}

void GetFrame(void *data,int *pW,int *pH,int *pBpp,int *lvlStat,
int *lvlstatR,int *lvlstatG,int *lvlstatB)
{
    int ret = 0;
    
    *pW = QCam.cameraW;
    *pH = QCam.cameraH;
    *pBpp = QCam.transferBit;

    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II)
    { 	
        while((ret != (QCam.cameraW * QCam.cameraH + 5)) && (liveabort == 0))
        {
            ret = qhyccd_readUSB2B(QCam.ccd_handle,data,QCam.cameraW * QCam.cameraH + 5,1,&QCam.pos);
            #ifdef QHYCCD_DEBUG
            printf("%d\n",ret);
            #endif
        }

	if(QCam.transferBit == 16 && QCam.CAMERA == DEVICETYPE_QHY5LII)
	{
            SWIFT_MSBLSBQHY5LII((unsigned char *)data);
	}
		
	IplImage *cvImg, *cropImg;
	cvImg = cvCreateImage(cvSize(QCam.ImgX, QCam.ImgY), QCam.transferBit, 1);
	cropImg = cvCreateImage(cvSize(QCam.ShowImgX, QCam.ShowImgY), QCam.transferBit, 1);
	cvImg->imageData = (char *)data + 5;
	cvSetImageROI(cvImg, cvRect(QCam.ShowImgX_Start, QCam.ShowImgY_Start, QCam.ShowImgX,QCam.ShowImgY));
	cvCopy(cvImg, cropImg, NULL);
	cvResetImageROI(cvImg);
	memcpy(data,cropImg->imageData,cropImg->imageSize);
	cvReleaseImage(&cvImg);
	cvReleaseImage(&cropImg);
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
    {
        while(ret != (ccdreg.LineSize * ccdreg.VerticalSize *2))
        {
            ret = qhyccd_readUSB2B(QCam.ccd_handle,data,ccdreg.LineSize * ccdreg.VerticalSize * 2,1,&QCam.pos);
            #ifdef QHYCCD_DEBUG
            printf("W %d H %d\n",QCam.cameraW,QCam.cameraH);
            printf("%d\n",ret);
            #endif
        }
        if(QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
        {
            if(ccdreg.VBIN == 1)
            {
                ConvertQHY9LDataBIN11(data,QCam.cameraW,QCam.cameraH,0);
            }
            else if(ccdreg.VBIN == 2)
            {
                ConvertQHY9LDataBIN22(data,QCam.cameraW,QCam.cameraH,0);
            }
            else if(ccdreg.VBIN == 4)
            {
                ConvertQHY9LDataBIN44(data,QCam.cameraW,QCam.cameraH,0);
            }
        }
        
    }
	
    if(QCam.bin == 22)
    {
	Bin2x2((unsigned char *)data,QCam.cameraW,QCam.cameraH);
	*pW /= 2;
	*pH /= 2;
    }
}

void GetImageData(int w,int h,int bpp,int channels,unsigned char *rawArray)
{
    int nWidth,nHeight,nBpp;

    unsigned char *ImgData = (unsigned char *)malloc(QCam.cameraW*QCam.cameraH*3*bpp/2);
    GetFrame(ImgData, &nWidth, &nHeight, &nBpp, NULL, NULL, NULL, NULL);

    memcpy(rawArray, ImgData, nWidth*nHeight*nBpp/8);

    
    if(bpp != nBpp)
    {
	if((bpp == 8) && (nBpp ==16))
	{
	    int i = 1;
	    int j = 0;
	    while(j < w*h)
	    {
                rawArray[j] = rawArray[i];
                j += 1;
		i += 2;
	    }
	}
	else if((bpp == 16) && (nBpp == 8))
	{
	    int i = 1;
	    int j = 0;

	    unsigned char *tempArray = (unsigned char *)malloc(w*h*2);

	    memcpy(tempArray,rawArray,w*h);
	    while(j < w*h)
	    {
                tempArray[i] = rawArray[j];
		tempArray[i-1] = 0;
	        j += 1;
		i += 2;
	    }
	    memcpy(rawArray,tempArray,w*h*2);
	    free(tempArray);
	}
    }

    if(channels == 3)
    {
	IplImage *img = cvCreateImage(cvSize(nWidth,nHeight),bpp,1);
	img->imageData = (char *)rawArray;
        IplImage *colorimg = cvCreateImage(cvSize(nWidth,nHeight),bpp,channels);
         
	if(QCam.CAMERA == DEVICETYPE_QHY5LII)
	{
            cvCvtColor(img,colorimg,CV_BayerGR2RGB);
	}
	memcpy(rawArray,colorimg->imageData,colorimg->imageSize);
	cvReleaseImage(&img);
	cvReleaseImage(&colorimg);
    }

    IplImage *img = cvCreateImage(cvSize(nWidth,nHeight),bpp,channels);
    img->imageData = (char *)rawArray;

    IplImage *rszimg = cvCreateImage(cvSize(w,h),bpp,channels);
    cvResize(img,rszimg,CV_INTER_NN);
    memcpy(rawArray,rszimg->imageData,rszimg->imageSize);
    cvReleaseImage(&rszimg);
    cvReleaseImage(&img);   
    free(ImgData);
}

void GetROIImageData(int w,int h,int bpp,int channels,unsigned char *rawArray)
{
	int nWidth,nHeight,nBpp;

	unsigned char *ImgData = (unsigned char *)malloc(QCam.cameraW*QCam.cameraH*3*bpp/2);
	memset(ImgData,0,QCam.cameraW*QCam.cameraH*3*bpp/2);
	GetFrame(ImgData, &nWidth, &nHeight, &nBpp, NULL, NULL, NULL, NULL);

	if (w == nWidth &&h == nHeight) 
	{
	    memcpy(rawArray, ImgData, nWidth*nHeight*nBpp/8);
	} 
	else if(w <= nWidth && h <= nHeight) 
	{
	    int count = 0;
            int i,j;
		if (nBpp > 8) 
		{
				
			for (i=0; i<h; i++)
			{
				for (j=0; j<w; j++) 
				{
					rawArray[count++] = ImgData[((nWidth*i)+j)*2]; 
					rawArray[count++] = ImgData[(((nWidth*i)+j)*2)+1]; 
				}
			}	
		} 
		else 
		{
				
			for (i=0; i<h; i++) 
			{
				for (j=0; j<w; j++)
				{
					rawArray[count] = ImgData[(nWidth*i)+j]; 
					count++;
				}
			}
				
		}
	}

	if(bpp != nBpp)
	{
		if((bpp == 8) && (nBpp ==16))
		{
			int i = 1;
			int j = 0;
			while(j < w*h)
			{
                rawArray[j] = rawArray[i];
                j += 1;
				i += 2;
			}
		}
		else if((bpp == 16) && (nBpp == 8))
		{
			int i = 1;
			int j = 0;

			unsigned char *tempArray = (unsigned char*)malloc(w*h*2);

			memcpy(tempArray,rawArray,w*h);
			while(j < w*h)
			{
                tempArray[i] = rawArray[j];
				tempArray[i-1] = 0;
			    j += 1;
			    i += 2;
			}
			memcpy(rawArray,tempArray,w*h*2);
			free(tempArray);
		}
	}

	if(channels == 3)
	{
		IplImage *img = cvCreateImage(cvSize(nWidth,nHeight),bpp,1);
		img->imageData = (char *)rawArray;
                IplImage *colorimg = cvCreateImage(cvSize(nWidth,nHeight),bpp,channels);


		if(QCam.CAMERA == DEVICETYPE_QHY5LII)
		{
                cvCvtColor(img,colorimg,CV_BayerGR2RGB);
		}
		memcpy(rawArray,colorimg->imageData,colorimg->imageSize);
		cvReleaseImage(&img);
		cvReleaseImage(&colorimg);
	}
	free(ImgData);

}

int GetMaxFrameLength(void)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        return 1280*1024*3;
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        return 35000000;
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	return 35000000;
    }
    return 0;
}

void BeginCooler(unsigned char PWM)
{
        if(QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY22)
	{
		setDC201FromInterrupt(QCam.ccd_handle,PWM,255);
	}
}

double GetTemp()
{
	if(QCam.CAMERA == DEVICETYPE_QHY5LII){
		return GetQHY5LIITemp();
	}
	if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
	{
		return GetCCDTemp(QCam.ccd_handle);
	}
	return 0.0;
}

void StopLive(void)
{
        liveabort = 1;
}

void StopCooler()
{
	if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L || QCam.CAMERA == DEVICETYPE_QHY22)
	{
		setDC201FromInterrupt(QCam.ccd_handle,0,255);
	}
}


bool IsAstroCcd(int m_devType)
{
    if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L
       || QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	return true;
    }
    return false;
}

bool IsHighSpeed(void)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII || QCam.CAMERA == DEVICETYPE_QHY5II || 
       QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        return true;
    }
    return false;
}

void GetImageFormat(int *w,int *h,int *bpp,int *channels)
{
    if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	*w = 1280;
	*h = 960;
	*bpp = 8;
	*channels = 1;
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	*w = 1280;
	*h = 1024;
	*bpp = 8;
	*channels = 1;
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY9 || QCam.CAMERA == DEVICETYPE_QHY9L)
    {
	*w = 3584;
	*h = 2574;
	*bpp = 16;
	*channels = 1;
    }
    else if(QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	*w = 3072;
    	*h = 2240;
    	*bpp = 16;
    	*channels = 1;
    }
    else
    {
	*w = 1280;
	*h = 1024;
	*bpp = 8;
	*channels = 1;
    }
}

void EepromRead(unsigned char addr, unsigned char* data, unsigned short len)
{
    libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_READ, 0xCA,0,addr,data,len, 0);
}

void GuideControl(unsigned char Direction,long PulseTime) 
{
    // pulseTime unit is 1ms
    unsigned char Buffer[2];

    if (Direction == 0) 
    {
        libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc0, 0x0001, 0x0080, Buffer, 2,0);
        usleep(PulseTime*1000);
        libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc0, 0x0001, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 1) 
    {
    	libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0040, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 2) 
    {
        libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0020, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 3) 
    {
    	libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0001, 0x0010, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0001, 0x0000, Buffer, 2,0);
    }
}

#ifdef __cplusplus
}
#endif


