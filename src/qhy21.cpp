#include "qhy21.h"
#include "qhycam.h"
#include <stdio.h>


extern QUsb *qhyusb;
QHY21 *qhy21;

void  QHY21::CorrectQHY21WH(int *w,int *h)
{
    if(*w <= 512 && *h <= 375)
    {
        *w = 512;
	*h = 375;
	initQHY21_1024x375();
    }
    else if(*w <= 1024 && *h <= 750)
    {
        *w = 1536;
	*h = 1120;
	initQHY21_1024x750();
    }
    else
    {
	*w = 2048;
	*h = 1500;
	initQHY21_2048x1500();
    }
}

void QHY21::initQHY21_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"QHY21-0";
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
    qhyusb->ccdreg.TopSkipPix = 0;
    qhyusb->ccdreg.MechanicalShutterMode = 0;
    qhyusb->ccdreg.DownloadCloseTEC = 0;
    qhyusb->ccdreg.SDRAM_MAXSIZE = 100;
    qhyusb->ccdreg.ClockADJ = 0x0000;
    qhyusb->ccdreg.CLAMP = 0;
    qhyusb->ccdreg.MotorHeating = 1;
}

void QHY21::initQHY21_1024x375(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 1024;
    qhyusb->ccdreg.VerticalSize = 375;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 512;
    qhyusb->QCam.cameraH = 375;
    
}

void QHY21::initQHY21_1024x750(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 1024;
    qhyusb->ccdreg.VerticalSize = 750;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 1024;
    qhyusb->QCam.cameraH = 750;
}
void QHY21::initQHY21_2048x1500(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 2048;
    qhyusb->ccdreg.VerticalSize = 1500;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 2048;
    qhyusb->QCam.cameraH = 1500;

}

