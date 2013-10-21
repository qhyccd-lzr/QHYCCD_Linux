#include "qhy23.h"
#include "qhycam.h"
#include <stdio.h>


extern QUsb *qhyusb;
QHY23 *qhy23;

void  QHY23::CorrectQHY23WH(int *w,int *h)
{
    if(*w <= 868 && *h <= 682)
    {
        *w = 868;
	*h = 682;
	initQHY23_1736x682();
    }
    else if(*w <= 1736 && *h <= 1364)
    {
        *w = 1736;
	*h = 1364;
	initQHY23_1736x1364();
    }
    else
    {
	*w = 3468;
	*h = 2728;
	initQHY23_3468x2728();
    }
}

void QHY23::initQHY23_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"QHY23-0";
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

void QHY23::initQHY23_1736x682(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 1736;
    qhyusb->ccdreg.VerticalSize = 682;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*50;
    qhyusb->QCam.cameraW = 868;
    qhyusb->QCam.cameraH = 682;
    
}

void QHY23::initQHY23_1736x1364(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 1736;
    qhyusb->ccdreg.VerticalSize = 1364;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*50;
    qhyusb->QCam.cameraW = 1736;
    qhyusb->QCam.cameraH = 1364;
}
void QHY23::initQHY23_3468x2728(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 3468;
    qhyusb->ccdreg.VerticalSize = 2728;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.P_Size = 1024*100;
    qhyusb->QCam.cameraW = 3468;
    qhyusb->QCam.cameraH = 2728;

}

