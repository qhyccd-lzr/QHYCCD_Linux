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
#include "qhycam.h"

#if 0
#ifdef __cplusplus
extern "C" 
{
#endif
#endif

extern QUsb *qhyusb;

extern int GainTable[73];

int OpenCamera(void)
{   
    qhyusb = new QUsb();

    qhyccd_device *dev;
    qhyccd_device_model *model;

    qhyusb->qhyccd_init();
    
    ssize_t n_device;
    n_device = qhyusb->qhyccd_get_device_list(&(qhyusb->QCam.device_list));
    #ifdef QHYCCD_DEBUG
    printf("Total devices %d\n",n_device);
    #endif
    
    int i;
    for(i = 0;i < n_device;i++)
    {
        model = qhyusb->qhyccd_get_device_model(qhyusb->QCam.device_list[i]);
        
        #ifdef QHYCCD_DEBUG
        printf("model is %d\n",model->model_id);
        #endif
        dev = qhyusb->QCam.device_list[i];
        qhyusb->qhyccd_open(dev,&(qhyusb->QCam.ccd_handle));

        if(model->model_id == QHYCCD_QHY5II)
        {            
            unsigned char buf[16];
            EepromRead(0x10,buf,16);
            if(buf[1] == 1)
                qhyusb->QCam.isColor = true;
            else 
                qhyusb->QCam.isColor = false;
            //(buf[1] == 1)? qhyusb->QCam.isColor = true:qhyusb->QCam.isColor = false;
            if(buf[0] == 6)
            {
                qhyusb->QCam.CAMERA = DEVICETYPE_QHY5LII;   
                InitCamera();
                return DEVICETYPE_QHY5LII;
            }
            else if(buf[0] == 1)
            {
                qhyusb->QCam.CAMERA = DEVICETYPE_QHY5II;
                InitCamera();
                return DEVICETYPE_QHY5II;
            }
        }
        else if(model->model_id == QHYCCD_QHY9)
        {
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY9;
            InitCamera();
            return DEVICETYPE_QHY9;
        }
        else if(model->model_id == QHYCCD_QHY9L)
        {
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY9L;
            InitCamera();
            return DEVICETYPE_QHY9L;
        }
        else if(model->model_id == QHYCCD_QHY22)
        {
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY22;
            InitCamera();
            return DEVICETYPE_QHY22;
        }
    }
   
    return DEVICETYPE_UNKOWN;   
}

void CloseCamera(void)
{
    qhyusb->qhyccd_free_device_list(qhyusb->QCam.device_list);
    qhyusb->qhyccd_close(qhyusb->QCam.ccd_handle);
}

void InitCamera(void)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        unsigned char buf[4];
        libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xc1,0,0,buf,4, 0);
        SetResolution(1280,960);
        SetExposureTime_QHY5LII(1000);
        SetUSBTraffic(30);
        SetSpeed(false);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetResolution(1280,1024);
	SetExposureTime_QHY5II(1000);
	SetUSBTraffic(30);
	SetSpeed(false);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        SetExposeTime(10.0);
        SetGain(30);
        SetOffset(120);
	SetSpeed(false);
        SetResolution(3584,2574);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
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
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        qhyusb->QCam.transferBit = 8;//8:位传输模式 16:16位传输模式
        qhyusb->QCam.bin = 11;//软件上BIN模式，11:原始模式 22:2x2合并模式
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
        qhyusb->QCam.transferBit = 16; 
    }
    qhyusb->QCam.isColor = false;//相机是否彩色 true:彩色 false:黑白
    qhyusb->QCam.transferspeed = 0;//传输速度0:低速 1:高速
    qhyusb->liveabort = 0;
}

void SetBin(int w,int h)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II )
    {
	if(w == 1 && h == 1)
	{
	    qhyusb->QCam.bin = 11;
	}
	else if(w == 2 && h == 2)
	{
	    qhyusb->QCam.bin = 22;
	}
    }
    qhyusb->QCam.bin = 11;
}

void SetTransferBit(int Bit)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        qhyusb->QCam.transferBit = Bit;
	if(Bit == 16)
	{
	    Set14Bit(1);
	}
	else 
	{
	    Set14Bit(0);
	}
	if(qhyusb->QCam.transferspeed == 1)
	{
	    SetSpeed(true);
	}
	else
	{
	    SetSpeed(false);
	}
	SetResolution(qhyusb->QCam.cameraW,qhyusb->QCam.cameraH);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
        qhyusb->QCam.transferBit = 16;
    }
    else
    {
        qhyusb->QCam.transferBit = 8;
    }
}

void SetExposeTime(double exptime)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetExposureTime_QHY5LII(exptime);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetExposureTime_QHY5II(exptime);
    }
    qhyusb->ccdreg.Exptime = exptime;       //unit: ms
    qhyusb->QCam.camTime = exptime;
}

void SetGain(unsigned short gain)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetQHY5LIIGain(gain);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        SetQHY5IIGain(gain);
    }
    qhyusb->ccdreg.Gain = gain;
    qhyusb->QCam.camGain = gain;
    
}

void SetOffset(unsigned char offset)
{
	  qhyusb->ccdreg.Offset = offset;
    qhyusb->QCam.camOffset = offset;
}

void SetResolution(int x,int y)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        CorrectQHY5LIIWH(&x,&y);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        CorrectQHY5IIWH(&x,&y);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9)
    {
        CorrectQHY9WH(&x,&y);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        CorrectQHY9LWH(&x,&y);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
        CorrectQHY22WH(&x,&y);
    }
 
    qhyusb->QCam.cameraW = x;
    qhyusb->QCam.cameraH = y;
    SetExposeTime(qhyusb->QCam.camTime);
    SetGain(qhyusb->QCam.camGain);
    if(qhyusb->QCam.isColor)
    {
        SetWBGreen(qhyusb->QCam.wbgreen);
        SetWBRed(qhyusb->QCam.wbred);
        SetWBBlue(qhyusb->QCam.wbblue);
    }
}


void SetSpeed(bool isHigh)
{
    //isHigh?qhyusb->QCam.transferspeed = 1:qhyusb->QCam.transferspeed = 0;
    if(isHigh)
        qhyusb->QCam.transferspeed = 1;
    else
        qhyusb->QCam.transferspeed = 0;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        if(isHigh)
	{
	    if(qhyusb->QCam.transferBit == 16)
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
	    if(qhyusb->QCam.transferBit == 16)
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
    if (qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II) 
    {
        if (qhyusb->QCam.cameraW == 1280)
	    qhyusb->I2CTwoWrite(0x300c, 1650 + i*50);
	else
	    qhyusb->I2CTwoWrite(0x300c, 1388 + i*50);
    }
}

void SetWBBlue(int blue)
{
    qhyusb->QCam.wbred = blue;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(qhyusb->QCam.camGain,qhyusb->QCam.wbred,qhyusb->QCam.wbblue);
    }
}

void SetWBGreen(int green)
{
    qhyusb->QCam.wbgreen = green;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(qhyusb->QCam.camGain,qhyusb->QCam.wbred,qhyusb->QCam.wbblue);
    }
}

void SetWBRed(int red)
{
    qhyusb->QCam.wbred = red;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        SetGainColorQHY5LII(qhyusb->QCam.camGain,qhyusb->QCam.wbred,qhyusb->QCam.wbblue);
    }
}


void BeginLive(void)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II ||
       qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L ||
       qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
        int Total_P,PatchNumber;
        sendRegisterQHYCCDOld(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*2,&Total_P,&PatchNumber);
	qhyusb->beginVideo(qhyusb->QCam.ccd_handle);
    }
    qhyusb->liveabort = 0;
}

void Bin2x2(unsigned char *ImgData,int w,int h)
{
    int i,j;
    unsigned long temp = 0;
    int s = 0;

    if(qhyusb->QCam.transferBit == 8)
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
    else if(qhyusb->QCam.transferBit == 16)
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
    
    *pW = qhyusb->QCam.cameraW;
    *pH = qhyusb->QCam.cameraH;
    *pBpp = qhyusb->QCam.transferBit;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    { 	
        while((ret != (qhyusb->QCam.cameraW * qhyusb->QCam.cameraH + 5)) && (qhyusb->liveabort == 0))
        {
            ret = qhyusb->qhyccd_readUSB2B(qhyusb->QCam.ccd_handle,(unsigned char *)data,qhyusb->QCam.cameraW * qhyusb->QCam.cameraH + 5,1,&qhyusb->QCam.pos);
            #ifdef QHYCCD_DEBUG
            printf("%d\n",ret);
            #endif
        }

	if(qhyusb->QCam.transferBit == 16 && qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
	{
            SWIFT_MSBLSBQHY5LII((unsigned char *)data);
	}
		
	IplImage *cvImg, *cropImg;
	cvImg = cvCreateImage(cvSize(qhyusb->QCam.ImgX, qhyusb->QCam.ImgY), qhyusb->QCam.transferBit, 1);
	cropImg = cvCreateImage(cvSize(qhyusb->QCam.ShowImgX, qhyusb->QCam.ShowImgY), qhyusb->QCam.transferBit, 1);
	cvImg->imageData = (char *)data + 5;
	cvSetImageROI(cvImg, cvRect(qhyusb->QCam.ShowImgX_Start, qhyusb->QCam.ShowImgY_Start, qhyusb->QCam.ShowImgX,qhyusb->QCam.ShowImgY));
	cvCopy(cvImg, cropImg, NULL);
	cvResetImageROI(cvImg);
	memcpy(data,cropImg->imageData,cropImg->imageSize);
	cvReleaseImage(&cvImg);
	cvReleaseImage(&cropImg);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
        while(ret != (qhyusb->ccdreg.LineSize * qhyusb->ccdreg.VerticalSize *2))
        {
            ret = qhyusb->qhyccd_readUSB2B(qhyusb->QCam.ccd_handle,(unsigned char *)data,qhyusb->ccdreg.LineSize * qhyusb->ccdreg.VerticalSize * 2,1,&qhyusb->QCam.pos);
            #ifdef QHYCCD_DEBUG
            printf("W %d H %d\n",qhyusb->QCam.cameraW,qhyusb->QCam.cameraH);
            printf("%d\n",ret);
            #endif
        }
        if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
        {
            if(qhyusb->ccdreg.VBIN == 1)
            {
                ConvertQHY9LDataBIN11((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,0);
            }
            else if(qhyusb->ccdreg.VBIN == 2)
            {
                ConvertQHY9LDataBIN22((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,0);
            }
            else if(qhyusb->ccdreg.VBIN == 4)
            {
                ConvertQHY9LDataBIN44((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,0);
            }
        }
        
    }
	
    if(qhyusb->QCam.bin == 22)
    {
	Bin2x2((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH);
	*pW /= 2;
	*pH /= 2;
    }
}

void GetImageData(int w,int h,int bpp,int channels,unsigned char *rawArray)
{
    int nWidth,nHeight,nBpp;

    unsigned char *ImgData = (unsigned char *)malloc(qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*3*bpp/2);
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
         
	if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
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

	unsigned char *ImgData = (unsigned char *)malloc(qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*3*bpp/2);
	memset(ImgData,0,qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*3*bpp/2);
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


		if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
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
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
        return 1280*1024*3;
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        return 35000000;
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	return 35000000;
    }
    return 0;
}

void BeginCooler(unsigned char PWM)
{
        if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
	{
		setDC201FromInterrupt(qhyusb->QCam.ccd_handle,PWM,255);
	}
}

double GetTemp()
{
	if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII){
		return GetQHY5LIITemp();
	}
	if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
	{
		return GetCCDTemp(qhyusb->QCam.ccd_handle);
	}
	return 0.0;
}

void StopLive(void)
{
        qhyusb->liveabort = 1;
}

void StopCooler()
{
	if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
	{
		setDC201FromInterrupt(qhyusb->QCam.ccd_handle,0,255);
	}
}


bool IsAstroCcd(int m_devType)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L
       || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
    {
    	return true;
    }
    return false;
}

bool IsHighSpeed(void)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II || 
       qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L)
    {
        return true;
    }
    return false;
}

void GetImageFormat(int *w,int *h,int *bpp,int *channels)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	*w = 1280;
	*h = 960;
	*bpp = 8;
	*channels = 1;
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	*w = 1280;
	*h = 1024;
	*bpp = 8;
	*channels = 1;
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY9L)
    {
	*w = 3584;
	*h = 2574;
	*bpp = 16;
	*channels = 1;
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
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
    libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_READ, 0xCA,0,addr,data,len, 0);
}

void GuideControl(unsigned char Direction,long PulseTime) 
{
    // pulseTime unit is 1ms
    unsigned char Buffer[2];

    if (Direction == 0) 
    {
        libusb_control_transfer(qhyusb->QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc0, 0x0001, 0x0080, Buffer, 2,0);
        usleep(PulseTime*1000);
        libusb_control_transfer(qhyusb->QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc0, 0x0001, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 1) 
    {
    	libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0040, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 2) 
    {
        libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0020, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0002, 0x0000, Buffer, 2,0);
    }
    else if (Direction == 3) 
    {
    	libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0001, 0x0010, Buffer, 2,0);
	usleep(PulseTime*1000);
	libusb_control_transfer(qhyusb->QCam.ccd_handle, QHYCCD_REQUEST_WRITE,0xc0, 0x0001, 0x0000, Buffer, 2,0);
    }
}
#if 0
#ifdef __cplusplus
}
#endif

#endif
