#include "qhy22.h"
#include "qhycam.h"
#include <stdio.h>

#if 0
#ifdef __cplusplus
extern "C"
{
#endif
#endif
extern QUsb *qhyusb;

void  CorrectQHY22WH(int *w,int *h)
{
    if(*w <= 768 && *h <= 560)
	{
		*w = 768;
		*h = 560;
		initQHY22_768x560();
	}
	else if(*w <= 1536 && *h <= 1120)
	{
		*w = 1536;
		*h = 1120;
		initQHY22_1536x1120();
	}
	else
	{
		*w = 3072;
		*h = 2240;
		initQHY22_3072x2240();
	}
}

void initQHY22_regs(void)
{
    qhyusb->ccdreg.devname = (char *)"QHY22-0";
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

void initQHY22_768x560(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 4;
    qhyusb->ccdreg.LineSize = 1536;
    qhyusb->ccdreg.VerticalSize = 560;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 768;
    qhyusb->QCam.cameraH = 560;
    
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}

void initQHY22_1536x1120(void)
{
    qhyusb->ccdreg.HBIN = 2;
    qhyusb->ccdreg.VBIN = 2;
    qhyusb->ccdreg.LineSize = 1536;
    qhyusb->ccdreg.VerticalSize = 1120;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 1536;
    qhyusb->QCam.cameraH = 1120;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}
void initQHY22_3072x2240(void)
{
    qhyusb->ccdreg.HBIN = 1;
    qhyusb->ccdreg.VBIN = 1;
    qhyusb->ccdreg.LineSize = 3072;
    qhyusb->ccdreg.VerticalSize = 2240;
    qhyusb->ccdreg.DownloadSpeed = qhyusb->QCam.transferspeed;
    qhyusb->ccdreg.Exptime = qhyusb->QCam.camTime;
    qhyusb->ccdreg.Gain = qhyusb->QCam.camGain;
    qhyusb->ccdreg.Offset =  qhyusb->QCam.camOffset;
    qhyusb->QCam.cameraW = 3072;
    qhyusb->QCam.cameraH = 2240;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(qhyusb->QCam.ccd_handle,qhyusb->ccdreg,qhyusb->QCam.cameraW*qhyusb->QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}

#if 0
#ifdef __cplusplus
}
#endif
#endif
