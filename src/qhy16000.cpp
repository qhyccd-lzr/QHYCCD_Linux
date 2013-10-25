#include "qhy16000.h"
#include "qhycam.h"
#include <stdio.h>


extern QUsb *qhyusb;
QHY16000 *qhy16000;

void  QHY16000::CorrectQHY16000WH(int *w,int *h)
{
    if(*w <= 4960 && *h <= 560)
    {
        *w = 4960;
	*h = 560;
	initQHY16000_4960x560();
    }
    else
    {
	*w = 4960;
	*h = 3328;
	initQHY16000_4960x3328();
    }
}

void QHY16000::initQHY16000_regs(void)
{
    unsigned char data[2];
    data[0] = 0x00;
    data[1] = 0xff;

    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb5,data,2);
    usleep(100000);
}

void QHY16000::sendByte2Regs(unsigned char index,unsigned char value)
{
    unsigned char data[2];
    data[0] = index;
    data[1] = value;

    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb5,data,2);
}

void QHY16000::sendWord2Regs(unsigned char index,unsigned short value)
{
    unsigned char data[3];
    data[0] = index;
    data[1] = qhyusb->MSB(value);
    data[2] = qhyusb->LSB(value);
    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb5,data,2);
}

void QHY16000::initQHY16000_4960x560(void)
{
    initQHY16000_regs();

    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 4960;
    qhyusb->ccdreg.VerticalSize = 560;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 4960;
    qhyusb->QCam.cameraH = 560;

    sendByte2Regs(0x13,0x00);
    sendByte2Regs(0x14,0x09);
    sendByte2Regs(0x15,0x18);
    sendByte2Regs(0x16,0x00);
    sendByte2Regs(0x17,0x00);
    sendByte2Regs(0x18,0x00);
    sendByte2Regs(0x19,0x00);
    sendByte2Regs(0x1a,0x00);
    sendByte2Regs(0x1b,0x00);
    sendWord2Regs(0x1c,0x00);
    sendWord2Regs(0x1e,0x400);
    sendByte2Regs(0x00,0x01);   
}


void QHY16000::initQHY16000_4960x3328(void)
{
    initQHY16000_regs();

    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 4960;
    qhyusb->ccdreg.VerticalSize = 3328;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 4960;
    qhyusb->QCam.cameraH = 3328;

    sendByte2Regs(0x13,0x00);
    sendByte2Regs(0x14,0x0c);
    sendByte2Regs(0x15,0xff);
    sendByte2Regs(0x16,0x00);
    sendByte2Regs(0x17,0x00);
    sendByte2Regs(0x18,0x00);
    sendByte2Regs(0x19,0x01);
    sendByte2Regs(0x1a,0x00);
    sendByte2Regs(0x1b,0x00);
    sendWord2Regs(0x1c,0x00);
    sendWord2Regs(0x1e,0x400);
    sendByte2Regs(0x00,0x00);  
}

void QHY16000::setExpseTime16000(unsigned long time)
{
    unsigned char data[2];

    data[1] = qhyusb->LSB(time);
    data[0] = qhyusb->MSB(time);
    sendByte2Regs(0x19,data[0]);
    sendByte2Regs(0x18,data[0]);
}

void QHY16000::setGain16000(unsigned long gain)
{
    unsigned char data[2];
    data[0] = 4;
    data[1] = (unsigned char)((gain &~0x00ffffff) >> 24);
    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb4,data,2);  

    data[0] = 5;
    data[1] = (unsigned char)((gain &~0xff00ffff) >> 16);
    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb4,data,2);  


    data[0] = 6;
    data[1] = (unsigned char)((gain &~0xffff00ff) >> 8);
    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb4,data,2);

    data[0] = 7;
    data[1] = (unsigned char)((gain &~0xffffff00));
    qhyusb->qhyccd_vTXD(qhyusb->QCam.ccd_handle,0xb4,data,2);
}
