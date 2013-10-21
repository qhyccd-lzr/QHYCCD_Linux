#include "qhy11.h"
#include "qhycam.h"
#include <stdio.h>


extern QUsb *qhyusb;
QHY11 *qhy11;

void  QHY11::CorrectQHY11WH(int *w,int *h)
{
    if(*w <= 1024 && *h <= 680)
    {
        *w = 1024;
	*h = 680;
	initQHY11_1024x680();
    }
    else if(*w <= 1368 && *h <= 906)
    {
        *w = 1368;
	*h = 906;
	initQHY11_1368x906();
    }
    else if(*w <= 2048 && *h <= 1360)
    {
        *w = 2048;
	*h = 1360;
	initQHY11_2048x1360();
    }
    else
    {
	*w = 4096;
	*h = 2720;
	initQHY11_4096x2720();
    }
}

void QHY11::initQHY11_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"QHY11-0";
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
    qhyusb->ccdreg.TopSkipNull = 10;
    qhyusb->ccdreg.TopSkipPix = 0;
    qhyusb->ccdreg.MechanicalShutterMode = 0;
    qhyusb->ccdreg.DownloadCloseTEC = 0;
    qhyusb->ccdreg.SDRAM_MAXSIZE = 100;
    qhyusb->ccdreg.ClockADJ = 0x0000;
    qhyusb->ccdreg.CLAMP = 0;
    qhyusb->ccdreg.MotorHeating = 1;
}

void QHY11::initQHY11_1024x680(void)
{
    qhyusb->ccdreg.HBIN = 4;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 1024;
    qhyusb->ccdreg.VerticalSize = 680;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*50;
    qhyusb->QCam.cameraW = 1024;
    qhyusb->QCam.cameraH = 680;
    
}

void QHY11::initQHY11_1368x906(void)
{
    qhyusb->ccdreg.HBIN = 3;
    qhyusb->ccdreg.VBIN = 3;
    qhyusb->ccdreg.LineSize = 1368;
    qhyusb->ccdreg.VerticalSize = 906;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*50;
    qhyusb->QCam.cameraW = 1368;
    qhyusb->QCam.cameraH = 906;
    
}

void QHY11::initQHY11_2048x1360(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 2048;
    qhyusb->ccdreg.VerticalSize = 1360;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*50;
    qhyusb->QCam.cameraW = 2048;
    qhyusb->QCam.cameraH = 1360;
}
void QHY11::initQHY11_4096x2720(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 4096;
    qhyusb->ccdreg.VerticalSize = 2720;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*100;
    qhyusb->QCam.cameraW = 3468;
    qhyusb->QCam.cameraH = 2728;

}

