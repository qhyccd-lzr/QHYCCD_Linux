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
#include "qhy6.h"
#include "qhy9.h"
#include "ic8300.h"
#include "qhy22.h"
#include "qhy21.h"
#include "qhy23.h"
#include "qhy11.h"
#include "qhy16000.h"
#include "qhycam.h"


extern QUsb     *qhyusb;
extern QHY5II   *q5ii;
extern QHY5LII  *q5lii;
extern QHY6     *qhy6;
extern QHY9     *qhy9;
extern IC8300   *ic8300;
extern QHY22    *qhy22;
extern QHY21    *qhy21;
extern QHY23    *qhy23;
extern QHY11    *qhy11;
extern QHY16000 *qhy16000;

extern int GainTable[73];

int OpenCameraByID(int camid)
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

            if(buf[0] == 6 && camid == DEVICETYPE_QHY5LII)
            {
                qhyusb->QCam.CAMERA = DEVICETYPE_QHY5LII;   
                q5lii = new QHY5LII();
                InitCamera();
                return DEVICETYPE_QHY5LII;
            }
            else if(buf[0] == 1 && camid == DEVICETYPE_QHY5II)
            {
                qhyusb->QCam.CAMERA = DEVICETYPE_QHY5II;
                q5ii = new QHY5II();
                InitCamera();
                return DEVICETYPE_QHY5II;
            }
        }
        else if(model->model_id == QHYCCD_QHY6 && camid == DEVICETYPE_QHY6)
        {
            #ifdef QHYCCD_DEBUG
            printf("QHY6 Found\n");
            #endif
            qhy6 = new QHY6();
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY6;
            InitCamera();
            return DEVICETYPE_QHY6;
        }
        else if(model->model_id == QHYCCD_QHY9 && camid == DEVICETYPE_QHY9)
        {
            #ifdef QHYCCD_DEBUG
            printf("QHY9 Found\n");
            #endif
            qhy9 = new QHY9();
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY9;
            InitCamera();
            return DEVICETYPE_QHY9;
        }
        else if(model->model_id == QHYCCD_IC8300 && camid == DEVICETYPE_IC8300)
        {
            ic8300 = new IC8300();
            qhyusb->QCam.CAMERA = DEVICETYPE_IC8300;
            InitCamera();
            ic8300->send2oled("USB CAM");
            return DEVICETYPE_IC8300;
        }
        else if(model->model_id == QHYCCD_QHY11 && camid == DEVICETYPE_QHY11)
        {
            qhy11 = new QHY11();
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY11;
            InitCamera();
            return DEVICETYPE_QHY11;
        }
        else if(model->model_id == QHYCCD_QHY22 && camid == DEVICETYPE_QHY22)
        {
            qhy22 = new QHY22();
            #ifdef QHYCCD_DEBUG
            printf("QHY22 Found\n");
            #endif
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY22;
            InitCamera();
            return DEVICETYPE_QHY22;
        }
        else if(model->model_id == QHYCCD_QHY21 && camid == DEVICETYPE_QHY21)
        {
            qhy21 = new QHY21();
            #ifdef QHYCCD_DEBUG
            printf("QHY21 Found\n");
            #endif
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY21;
            InitCamera();
            return DEVICETYPE_QHY21;
        }  
        else if(model->model_id == QHYCCD_QHY23 && camid == DEVICETYPE_QHY23)
        {
            qhy23 = new QHY23();
            #ifdef QHYCCD_DEBUG
            printf("QHY23 Found\n");
            #endif
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY23;
            InitCamera();
            return DEVICETYPE_QHY23;
        }      
        else if(model->model_id == QHYCCD_QHY16000 && camid == DEVICETYPE_QHY16000)
        {
            qhy16000 = new QHY16000();
            #ifdef QHYCCD_DEBUG
            printf("QHY16000 Found\n");
            #endif
            qhyusb->QCam.CAMERA = DEVICETYPE_QHY16000;
            InitCamera();
            return DEVICETYPE_QHY16000;
        } 
    }
    #ifdef QHYCCD_DEBUG
    printf("please check USB link or camid\n");
    #endif
    return DEVICETYPE_UNKOWN;   
}

void CloseCamera(void)
{
    if(qhyusb)
        delete(qhyusb);
    else if(q5lii)
        delete(q5lii);
    else if(q5ii)
        delete(q5ii);
    else if(qhy6)
        delete(qhy6);
    else if(qhy9)
        delete(qhy9);
    else if(qhy11)
        delete(qhy11);
    else if(qhy21)
        delete(qhy21);
    else if(qhy22)
        delete(qhy22);
    else if(qhy23)
        delete(qhy23);
    else if(qhy16000)
        delete(qhy16000);
    qhyusb->qhyccd_free_device_list(qhyusb->QCam.device_list);
    qhyusb->qhyccd_close(qhyusb->QCam.ccd_handle);
}

void InitCamera(void)
{
    unsigned char buf[4];

    switch(qhyusb->QCam.CAMERA)
    {
        case DEVICETYPE_QHY5II:
	case DEVICETYPE_QHY5LII:
    	{
            qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xc1,buf,4);
            SetUSBTraffic(30);
            SetSpeed(false);
            if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
            {
                SetResolution(1280,960);
                q5lii->SetExposureTime_QHY5LII(1000);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
            {
                SetResolution(1280,1024);
	        q5ii->SetExposureTime_QHY5II(1000);
            }
	    break;
    	}
        case DEVICETYPE_QHY6:
    	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
    	case DEVICETYPE_QHY22:
    	case DEVICETYPE_QHY21:
    	case DEVICETYPE_QHY23:
        {
    	    SetExposeTime(1000.0);
    	    SetGain(0);
    	    SetOffset(130);
    	    SetSpeed(false);
    	    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY6)
            {
                SetResolution(800,298);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9 || qhyusb->QCam.CAMERA == DEVICETYPE_IC8300)
            {
                SetResolution(3584,2574);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY11)
            {
                SetResolution(4096,2720);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
            {
    	        SetResolution(3072,2240);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY21)
            {
    	        SetResolution(2048,1500);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY23)
            {
    	        SetResolution(3468,2728);
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY16000)
            {
    	        SetResolution(4960,3328);
            }
	    break;
        }
        
    }
    InitOthers();
}


void InitOthers(void)
{
    switch(qhyusb->QCam.CAMERA)
    {
        case DEVICETYPE_QHY16000:
        case DEVICETYPE_QHY5II:
        case DEVICETYPE_QHY5LII:
        {
            qhyusb->QCam.transferBit = 8;//8:位传输模式 16:16位传输模式
            qhyusb->QCam.bin = 11;//软件上BIN模式，11:原始模式 22:2x2合并模式
            break;
        }
        case DEVICETYPE_QHY6:
        case DEVICETYPE_QHY9:
        case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
        case DEVICETYPE_QHY22:
        case DEVICETYPE_QHY23:
        {
            qhyusb->QCam.transferBit = 16; 
            break;
        }
    }
    qhyusb->QCam.isColor = false;//相机是否彩色 true:彩色 false:黑白
    qhyusb->QCam.transferspeed = 0;//传输速度0:低速 1:高速
    qhyusb->liveabort = 0;
    qhyusb->QCam.expModeChanged = false;
    qhyusb->QCam.usbtraffic = 30;
}

void SetBin(int w,int h)
{
    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY5LII:
	case DEVICETYPE_QHY5II:
	{
	    if(w == 1 && h == 1)
		qhyusb->QCam.bin = 11;
	    else if(w == 2 && h == 2)
		qhyusb->QCam.bin = 22;
	    break;
	}
        default:
            qhyusb->QCam.bin = 11;
    }
}

void SetTransferBit(int Bit)
{
    switch(qhyusb->QCam.CAMERA)
    {	
	case DEVICETYPE_QHY5II:
		break;
	case DEVICETYPE_QHY5LII:
	{
		qhyusb->QCam.transferBit = Bit;
		if(Bit == 16)
		{
		    q5lii->Set14Bit(1);
		}
		else 
		{
		    q5lii->Set14Bit(0);
		}
		if(qhyusb->QCam.transferspeed == 1)
		{
		    SetSpeed(true);
		}
		else
		{
		    SetSpeed(false);
		}
		break;
	}
        case DEVICETYPE_QHY6:
	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
	case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
	case DEVICETYPE_QHY22:
	case DEVICETYPE_QHY23:
	{
		qhyusb->QCam.transferBit = 16;
		break;
	}
	default:
		qhyusb->QCam.transferBit = 8;
    }
}

void SetExposeTime(double exptime)
{
    switch(qhyusb->QCam.CAMERA)
    {	
	case DEVICETYPE_QHY5II:
	{
		q5ii->SetExposureTime_QHY5II(exptime);
		break;
	}
	case DEVICETYPE_QHY5LII:
	{
		//expMode changed, long mode change to short mode  or short mode to long mode
		if((qhyusb->QCam.camTime < 4000 && exptime >= 4000) || (qhyusb->QCam.camTime >= 4000 && exptime < 4000))
			qhyusb->QCam.expModeChanged = true;

		if(exptime >= 1200000)
		{
			q5lii->CheckBoxQHY5LIILoneExpMode = true;
		}
		else 
		{
			q5lii->CheckBoxQHY5LIILoneExpMode = false;
		}

		q5lii->SetExposureTime_QHY5LII(exptime);
		break;
	}
        case DEVICETYPE_QHY16000:
        {
                qhy16000->setExpseTime16000(exptime);
                break;
        }
    }
    qhyusb->QCam.camTime = (unsigned long)exptime/10;
}

void SetGain(unsigned short gain)
{
    switch(qhyusb->QCam.CAMERA)
    {	
	case DEVICETYPE_QHY5II:
	{
		q5ii->SetQHY5IIGain(gain);
		break;
	}
	case DEVICETYPE_QHY5LII:
    	{
        	q5lii->SetQHY5LIIGain(gain);
		break;
    	}
        case DEVICETYPE_QHY16000:
        {
                qhy16000->setGain16000(gain);
                break;
        }
    }
    qhyusb->QCam.camGain = gain;
}

void SetOffset(unsigned char offset)
{
    qhyusb->QCam.camOffset = offset;
}

void SetResolution(int x,int y)
{
    switch(qhyusb->QCam.CAMERA)
    {	
	case DEVICETYPE_QHY5II:
	{
	    q5ii->CorrectQHY5IIWH(&x,&y);
	    break;
	}
	case DEVICETYPE_QHY5LII:
	{
       	    q5lii->CorrectQHY5LIIWH(&x,&y);
	    break;
    	}
        case DEVICETYPE_QHY6:
        {
            qhy6->CorrectQHY6WH(&x,&y);
            break;
        }
    	case DEVICETYPE_QHY9:
    	{
	    qhy9->CorrectQHY9WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_IC8300:
    	{
	    ic8300->CorrectIC8300WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_QHY11:
    	{
	    qhy11->CorrectQHY11WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_QHY21:
    	{
	    qhy21->CorrectQHY21WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_QHY22:
    	{
	    qhy22->CorrectQHY22WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_QHY23:
    	{
	    qhy23->CorrectQHY23WH(&x,&y);
	    break;
    	}
    	case DEVICETYPE_QHY16000:
    	{
	    qhy16000->CorrectQHY16000WH(&x,&y);
	    break;
    	}
    }
 
    qhyusb->QCam.cameraW = x;
    qhyusb->QCam.cameraH = y;
    SetExposeTime(qhyusb->QCam.camTime);
    SetGain(qhyusb->QCam.camGain);
    SetUSBTraffic(qhyusb->QCam.usbtraffic);
    if(qhyusb->QCam.isColor)
    {
        SetWBGreen(qhyusb->QCam.wbgreen);
        SetWBRed(qhyusb->QCam.wbred);
        SetWBBlue(qhyusb->QCam.wbblue);
    }

}
void SetSpeed(bool isHigh)
{
    if(isHigh)
        qhyusb->QCam.transferspeed = 1;
    else
        qhyusb->QCam.transferspeed = 0;

    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY5LII:
	case DEVICETYPE_QHY5II:
    	{
            if(isHigh)
	    {
	    	if(qhyusb->QCam.transferBit == 16)
		    q5lii->SetSpeedQHY5LII(1);
	    	else
		    q5lii->SetSpeedQHY5LII(2);
            }
            else
            {
	    	if(qhyusb->QCam.transferBit == 16)
	            q5lii->SetSpeedQHY5LII(0);
	    	else
		    q5lii->SetSpeedQHY5LII(1);
	    }
	    break;
	}
    }

    SetExposeTime(qhyusb->QCam.camTime);
    SetGain(qhyusb->QCam.camGain);
    if(qhyusb->QCam.isColor)
    {
	SetWBRed(qhyusb->QCam.wbred);
	SetWBBlue(qhyusb->QCam.wbblue);
	SetWBGreen(qhyusb->QCam.wbgreen);
    }
}


void SetUSBTraffic(int i)
{
    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
    {
	qhyusb->I2CTwoWrite(0x05, 0x0009 + i*50);
    }
    else if (qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII) 
    {
        if (qhyusb->QCam.cameraW == 1280)
	    qhyusb->I2CTwoWrite(0x300c, 1650 + i*50);
	else
	    qhyusb->I2CTwoWrite(0x300c, 1388 + i*50);
    }
    qhyusb->QCam.usbtraffic = i;
}

void SetWBBlue(int blue)
{
    qhyusb->QCam.wbred = blue;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	double R2G, B2G;
	R2G = double(qhyusb->QCam.wbred) / 100;
	B2G = double(qhyusb->QCam.wbblue) / 100;
        q5lii->SetGainColorQHY5LII(qhyusb->QCam.camGain,R2G,B2G);
    }
}

void SetWBGreen(int green)
{
    qhyusb->QCam.wbgreen = green;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        double R2G, B2G;
	R2G = double(qhyusb->QCam.wbred) / 100;
	B2G = double(qhyusb->QCam.wbblue) / 100;
        q5lii->SetGainColorQHY5LII(qhyusb->QCam.camGain,R2G,B2G);
    }
}

void SetWBRed(int red)
{
    qhyusb->QCam.wbred = red;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
	double R2G, B2G;
	R2G = double(qhyusb->QCam.wbred) / 100;
	B2G = double(qhyusb->QCam.wbblue) / 100;
        q5lii->SetGainColorQHY5LII(qhyusb->QCam.camGain,R2G,B2G);
    }
}


void BeginLive(void)
{
    #ifdef QHYCCD_DEBUG
    printf("BeginExpose\n");
    #endif
    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY5LII:
	case DEVICETYPE_QHY5II:
        {
	    qhyusb->beginVideo(qhyusb->QCam.ccd_handle);
            break;
        }
        case DEVICETYPE_QHY6:
	case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY21:
	case DEVICETYPE_QHY22:
        case DEVICETYPE_QHY23:
    	{ 
            if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY6)
            {
                qhy6->initQHY6_regs();
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_IC8300)
            {
                ic8300->initIC8300_regs();
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY21)
            {
                qhy21->initQHY21_regs();
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY22)
            {
                qhy22->initQHY22_regs();
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY23)
            {
                qhy23->initQHY23_regs();
            }
            sendRegisterQHYCCDOld(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.P_Size,&qhyusb->QCam.Total_P,&qhyusb->QCam.PatchNumber);
	    qhyusb->beginVideo(qhyusb->QCam.ccd_handle);
	    break;
    	}
        case DEVICETYPE_QHY9:
        case DEVICETYPE_QHY11:
        {
            if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY9)
            {
                qhy9->initQHY9_regs();
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY11)
            {
                qhy11->initQHY11_regs();
            }
            sendRegisterQHYCCDNew(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.P_Size,&qhyusb->QCam.Total_P,&qhyusb->QCam.PatchNumber);
            qhyusb->beginVideo(qhyusb->QCam.ccd_handle);
	    break;           
        }
        case DEVICETYPE_QHY16000:
        {
            qhyusb->beginVideo(qhyusb->QCam.ccd_handle);
            break;
        }
    }
}

void StopLive(void)
{
	qhyusb->liveabort = 1;//quit the getImage loop
	switch(qhyusb->QCam.CAMERA)
	{
		case DEVICETYPE_QHY5II:
		case DEVICETYPE_QHY5LII:
		{
			unsigned char buf[4];
			qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xc1,buf,4);
			break;
		}
	}

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

    switch(qhyusb->QCam.CAMERA)
    {
        case DEVICETYPE_QHY5LII:
        case DEVICETYPE_QHY5II:
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
                q5lii->SWIFT_MSBLSBQHY5LII((unsigned char *)data);
	    }
		
	    IplImage *cvImg, *cropImg;
	    cvImg = cvCreateImage(cvSize(qhyusb->QCam.ImgX, qhyusb->QCam.ImgY), qhyusb->QCam.transferBit, 1);
	    cropImg = cvCreateImage(cvSize(qhyusb->QCam.ShowImgX, qhyusb->QCam.ShowImgY), qhyusb->QCam.transferBit, 1);
	    cvImg->imageData = (char *)data;
	    cvSetImageROI(cvImg, cvRect(qhyusb->QCam.ShowImgX_Start, qhyusb->QCam.ShowImgY_Start, qhyusb->QCam.ShowImgX,qhyusb->QCam.ShowImgY));
	    cvCopy(cvImg, cropImg, NULL);
	    cvResetImageROI(cvImg);
	    memcpy(data,cropImg->imageData,cropImg->imageSize);
	    cvReleaseImage(&cvImg);
	    cvReleaseImage(&cropImg);
            break;
        }
        case DEVICETYPE_QHY16000:
        {
            qhyusb->qhyccd_readUSB2B(qhyusb->QCam.ccd_handle,(unsigned char *)data,qhyusb->QCam.cameraW * qhyusb->QCam.cameraH,1,&qhyusb->QCam.pos);
            break;
        }
        case DEVICETYPE_QHY9:
        case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
        case DEVICETYPE_QHY22:
        case DEVICETYPE_QHY23:
        case DEVICETYPE_QHY6:
        {
            qhyusb->qhyccd_readUSB2B(qhyusb->QCam.ccd_handle,(unsigned char *)data,qhyusb->QCam.P_Size,qhyusb->QCam.Total_P,&qhyusb->QCam.pos);
            
            if(qhyusb->QCam.CAMERA == DEVICETYPE_IC8300 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY22 || qhyusb->QCam.CAMERA == DEVICETYPE_QHY21 ||
               qhyusb->QCam.CAMERA == DEVICETYPE_QHY23)
            {
                if(qhyusb->ccdreg.VBIN == 1)
                {
                    ic8300->ConvertIC8300DataBIN11((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,qhyusb->ccdreg.TopSkipPix);
                }
                else if(qhyusb->ccdreg.VBIN == 2)
                {
                    ic8300->ConvertIC8300DataBIN22((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,qhyusb->ccdreg.TopSkipPix);
                }
                else if(qhyusb->ccdreg.VBIN == 4)
                {
                    ic8300->ConvertIC8300DataBIN44((unsigned char *)data,qhyusb->QCam.cameraW,qhyusb->QCam.cameraH,qhyusb->ccdreg.TopSkipPix);
                }
            }
            else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY6)
            {
                if(qhyusb->ccdreg.VBIN == 1)
                {
                    qhy6->ConvertQHY6PRODataBIN11((unsigned char *)data);
                }
                else if(qhyusb->ccdreg.VBIN == 2)
                {
                    qhy6->ConvertQHY6PRODataBIN22((unsigned char *)data);
                }
            }
            break;
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

    unsigned char *ImgData = (unsigned char *)malloc(qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*3*bpp/8);
    #ifdef QHYCCD_DEBUG
    printf("GetImageData:Malloc memory Size %d\n",qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*3*bpp/8);
    #endif

    GetFrame(ImgData, &nWidth, &nHeight, &nBpp, NULL, NULL, NULL, NULL);
    #ifdef QHYCCD_DEBUG
    printf("GetImageData:nWidth %d nHeight %d nBpp %d\n",nWidth,nHeight,nBpp);
    #endif

    memcpy(rawArray, ImgData,nWidth*nHeight*nBpp/8);
    
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
    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY5LII:
	case DEVICETYPE_QHY5II:
	    return 1280*1024*3*2;
        case DEVICETYPE_QHY6:
            return 800*298*3;
	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
            return 3584*2574*3;
        case DEVICETYPE_QHY11:
            return 4096*2720*3;
        case DEVICETYPE_QHY21:
            return 2048*1500*3;
	case DEVICETYPE_QHY22:
            return 3072*2240*3;
	case DEVICETYPE_QHY23:
            return 3468*2728*3;

    }
    return 0;
}

void BeginCooler(unsigned char PWM)
{
    switch(qhyusb->QCam.CAMERA)
    {
        case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY9:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
        case DEVICETYPE_QHY22:
        case DEVICETYPE_QHY23:
	{
            setDC201FromInterrupt(qhyusb->QCam.ccd_handle,PWM,255);
            break;
	}
    }
}
double GetTemp()
{
    switch(qhyusb->QCam.CAMERA)
    {
        case DEVICETYPE_QHY5LII:
	    return q5lii->GetQHY5LIITemp();
	case DEVICETYPE_QHY9:
        case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
        case DEVICETYPE_QHY22:
        case DEVICETYPE_QHY23:
	{
	    return GetCCDTemp(qhyusb->QCam.ccd_handle);
	}
    }
	return 0.0;
}
void StopCooler()
{
     switch(qhyusb->QCam.CAMERA)
     {
         case DEVICETYPE_QHY9:
         case DEVICETYPE_IC8300:
         case DEVICETYPE_QHY11:
         case DEVICETYPE_QHY21:
         case DEVICETYPE_QHY22:
         case DEVICETYPE_QHY23:
         {
	    setDC201FromInterrupt(qhyusb->QCam.ccd_handle,0,255);
            break;
	 }
    }
}

bool IsAstroCcd(int m_devType)
{
    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
	case DEVICETYPE_QHY22:
	case DEVICETYPE_QHY23:
    	    return true;
    }
    return false;
}

bool IsHighSpeed(void)
{
    switch(qhyusb->QCam.CAMERA)
    {
	case DEVICETYPE_QHY5II:
	case DEVICETYPE_QHY5LII:
	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
        case DEVICETYPE_QHY11:
        case DEVICETYPE_QHY21:
	case DEVICETYPE_QHY22:
	case DEVICETYPE_QHY23:
	    return true;
    }
    return false;
}

void GetImageFormat(int *w,int *h,int *bpp,int *channels)
{
    switch(qhyusb->QCam.CAMERA)
    {
    	case DEVICETYPE_QHY5LII:
	{
		*w = 1280;
		*h = 960;
		*bpp = 8;
		*channels = 1;
		break;
	}
	case DEVICETYPE_QHY5II:
	{
		*w = 1280;
		*h = 1024;
		*bpp = 8;
		*channels = 1;
		break;
	}
        case DEVICETYPE_QHY6:
        {
                *w = 800;
                *h = 298;
                *channels = 1;
                break;
        }
	case DEVICETYPE_QHY9:
	case DEVICETYPE_IC8300:
	{
		*w = 3584;
		*h = 2574;
		*bpp = 16;
		*channels = 1;
		break;
	}
        case DEVICETYPE_QHY11:
        {   
               *w = 4096;
               *h = 2720;
               *bpp = 16;
               *channels = 1;
               break;
        }
	case DEVICETYPE_QHY22:
	{
		*w = 3072;
		*h = 2240;
		*bpp = 16;
		*channels = 1;
		break;
	}
        case DEVICETYPE_QHY21:
        {   
               *w = 2048;
               *h = 1500;
               *bpp = 16;
               *channels = 1;
               break;
        }
        case DEVICETYPE_QHY23:
        {   
               *w = 3468;
               *h = 2728;
               *bpp = 16;
               *channels = 1;
               break;
        }
	default:
	{
		*w = 1280;
		*h = 1024;
		*bpp = 8;
		*channels = 1;
		break;
	}
    }
}

void EepromRead(unsigned char addr, unsigned char* data, unsigned short len)
{
    qhyusb->qhyccd_vRXD_Ex(qhyusb->QCam.ccd_handle,0xCA,0,addr,data,len);
}

void GuideControl(unsigned char Direction,long PulseTime) 
{
    unsigned char Buffer[4];
    unsigned short value = 0;
    unsigned short index = 0;

    if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
    {
        switch(Direction)
	{
            case 0:
	    {
	        index = 0x80;
		value = 0x01;
		break;
            }
	    case 1:
	    {
		index = 0x40;
		value = 0x02;
		break;
            }
	    case 2:
	    {
		index = 0x20;
		value = 0x02;
		break;
	    }
	    case 3:
	    {
		index = 0x10;
		value = 0x01;
		break;
            }
	}
        qhyusb->qhyccd_vTXD_Ex(qhyusb->QCam.ccd_handle,0xc0,value,index,Buffer,2);
    // pulseTime unit is 1ms
        usleep(PulseTime*1000);
        qhyusb->qhyccd_vTXD_Ex(qhyusb->QCam.ccd_handle,0xc0,value,0x0,Buffer,2);
    }
    else if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY6)
    {
        switch(Direction)
	{
            case 0:
	    {
	        index = 0x02;
		break;
            }
	    case 1:
	    {
		index = 0x04;
		break;
            }
	    case 2:
	    {
		index = 0x01;
		break;
	    }
	    case 3:
	    {
		index = 0x08;
		break;
            }
	}
        Buffer[0] = 0x09;
        Buffer[1] = index;
        Buffer[2] = qhyusb->MSB(PulseTime);
        Buffer[3] = qhyusb->LSB(PulseTime);
        sendInterrupt(qhyusb->QCam.ccd_handle,4,Buffer);
        usleep(PulseTime*1000);
    }
}


bool IsControlAvailable(CONTROL_ID id)
{
    switch(id)
    {
	case CONTROL_GAIN:
	case CONTROL_EXPOSURE:
	case CONTROL_SPEED:
	case CONTROL_TRANSFERBIT:
	    return true;
	case CONTROL_OFFSET:
	case CONTROL_COOLER:
	{
	    switch(qhyusb->QCam.CAMERA)
	    {
		case DEVICETYPE_QHY9:
		case DEVICETYPE_IC8300:
		case DEVICETYPE_QHY11:
                case DEVICETYPE_QHY21:
		case DEVICETYPE_QHY22:
                case DEVICETYPE_QHY23:
		    return true;
	    }
	    return false;
	}	
	case CONTROL_WBR:
	case CONTROL_WBB:
	{
	    switch(qhyusb->QCam.CAMERA)
	    {
		case DEVICETYPE_QHY5LII:
		{
		    if(qhyusb->QCam.isColor)
			return true;
		    else 
			return false;
		}
	    }
	    return false;
	}
	case CONTROL_WBG:
	{
	    switch(qhyusb->QCam.CAMERA)
	    {
		case DEVICETYPE_QHY5LII:
		    return false;
	    }
	    return false;
	}
	case CONTROL_USBTRAFFIC:
	{
	   switch(qhyusb->QCam.CAMERA)
	   {
		case DEVICETYPE_QHY5II:
		case DEVICETYPE_QHY5LII:
		    return true;
	   }
	   return false;
	}
	case CONTROL_ROWNOISERE:
	{
	   if(qhyusb->QCam.CAMERA==DEVICETYPE_QHY5II)
		return true;
	   else
		return false;
	}
	case CONTROL_HDRMODE:
	{
	   if(qhyusb->QCam.CAMERA==DEVICETYPE_QHY5LII)
		return true;
	   else
		return false;
	}
    }
    return false;
}

double GetMinValue(CONTROL_ID id)
{
    switch(id)
    {
	case CONTROL_WBR:
	case CONTROL_WBB:
	case CONTROL_WBG:
	{
	    switch(qhyusb->QCam.CAMERA)
	    {
		case DEVICETYPE_QHY5LII: 
			return 50;
	    }
	    break;
	}
	case CONTROL_GAIN:
	    return 0;
	case CONTROL_OFFSET:
	    return 0;
	case CONTROL_EXPOSURE:
	    return 0;
	case CONTROL_SPEED:
	    return 0;
	case CONTROL_USBTRAFFIC:
	    return 0;
	case CONTROL_TRANSFERBIT:
	    return 8;
	case CONTROL_ROWNOISERE:
	    return 0;
	case CONTROL_HDRMODE:
	    return 0;
	case CONTROL_COOLER:
	    return 0;
    }
    return 0.0;

}

double GetMaxValue(CONTROL_ID id)
{
    switch(id)
    {
	case CONTROL_WBR:
	case CONTROL_WBB:
	case CONTROL_WBG:
	{
	    switch(qhyusb->QCam.CAMERA)
	    {
		case DEVICETYPE_QHY5LII:
		    return 200;
	    }
	    break;
	}
	case CONTROL_GAIN:
		return 1000;
	case CONTROL_OFFSET:
		return 255;
	case CONTROL_SPEED:
		return 1;
	case CONTROL_USBTRAFFIC:
		return 30;
	case CONTROL_TRANSFERBIT:
		return 16;
	case CONTROL_ROWNOISERE:
		return 1;
	case CONTROL_HDRMODE:
		return 1;
	case CONTROL_COOLER:
		return 100;
    }
    return 0.0;
}

void SetValue(CONTROL_ID id, double value)
{
    switch(id)
    {
	case CONTROL_WBR:
	{
		SetWBRed((int)value);
		break;
	}
	case CONTROL_WBG:
	{
		SetWBGreen((int)value);
		break;
	}
	case CONTROL_WBB:
	{
		SetWBBlue((int)value);
		break;
	}
	case CONTROL_EXPOSURE:
	{
		SetExposeTime(value);
		break;
	}
	case CONTROL_GAIN:
	{
		SetGain((unsigned short)value);
		break;
	}
	case CONTROL_OFFSET:
	{
		SetOffset((unsigned char)value);
		break;
	}
	case CONTROL_SPEED:
	{
		if((int)value)
			SetSpeed(true);
		else
			SetSpeed(false);
		break;
	}
	case CONTROL_USBTRAFFIC:
	{
		if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II || qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
		{
		    SetUSBTraffic(value);
			
		    SetExposeTime(qhyusb->QCam.camTime);
		    SetGain(qhyusb->QCam.camGain);
		    if(qhyusb->QCam.isColor)
		    {
			SetWBRed(qhyusb->QCam.wbred);
			SetWBBlue(qhyusb->QCam.wbblue);
			SetWBGreen(qhyusb->QCam.wbgreen);
		    }
		}
		break;
	}
	case CONTROL_TRANSFERBIT:
	{
		SetTransferBit((int)value);
		break;
	}
	case CONTROL_ROWNOISERE:
	{
    		if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5II)
		{
		    if((int)value)
        	    	q5ii->QHY5II_DeNoise(true);
		    else
			q5ii->QHY5II_DeNoise(false);
		    SetResolution(qhyusb->QCam.cameraW,qhyusb->QCam.cameraH);
		}
		break;
	}
	case CONTROL_HDRMODE:
	{
		if((int)value)
		    q5lii->SetQHY5LIIHDR(true);
		else
		    q5lii->SetQHY5LIIHDR(false);
		if(qhyusb->QCam.CAMERA == DEVICETYPE_QHY5LII)
		{
		    if((int)value)
  			q5lii->SetQHY5LIIHDR(true);
		    else
		    {
			q5lii->SetQHY5LIIHDR(false);
			usleep(100000);
		   	q5lii->SetQHY5LIIHDR(true);
			usleep(100000);
		   	q5lii->SetQHY5LIIHDR(false);
			usleep(100000);
		    }
		}
		break;
	}
	case CONTROL_COOLER:
	{
		value = (value/100)*255;
		BeginCooler((unsigned char)value);
		break;
	}
    }	 
}

double GetValue(CONTROL_ID id)
{
    switch(id)
    {
	case CONTROL_WBR:
	    return qhyusb->QCam.wbred;
	case CONTROL_WBG:
	    return qhyusb->QCam.wbgreen;
	case CONTROL_WBB:
	    return qhyusb->QCam.wbblue;
	case CONTROL_EXPOSURE:
	    return qhyusb->QCam.camTime;
	case CONTROL_GAIN:
	    return qhyusb->QCam.camGain;
	case CONTROL_OFFSET:
 	    return qhyusb->QCam.camOffset;
    }
    return 0;
}
