#include "qhy22.h"
#include "QHYCAM.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern QHYCCD QCam;
extern CCDREG ccdreg;

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
    ccdreg.devname = "QHY22-0";
    ccdreg.Gain = QCam.camGain;
    ccdreg.Offset = QCam.camOffset;
    ccdreg.Exptime = QCam.camTime;       //unit: ms
    ccdreg.SKIP_TOP = 0;
    ccdreg.SKIP_BOTTOM = 0;
    ccdreg.AMPVOLTAGE = 1;
    ccdreg.LiveVideo_BeginLine = 0;
    ccdreg.AnitInterlace = 0;
    ccdreg.MultiFieldBIN = 0;
    ccdreg.TgateMode = 0;
    ccdreg.ShortExposure = 0;
    ccdreg.VSUB = 0;
    ccdreg.TransferBIT = 0;
    ccdreg.TopSkipNull = 30;
    ccdreg.TopSkipPix = 0;
    ccdreg.MechanicalShutterMode = 0;
    ccdreg.DownloadCloseTEC = 0;
    ccdreg.SDRAM_MAXSIZE = 100;
    ccdreg.ClockADJ = 0x0000;
    ccdreg.CLAMP = 0;
    ccdreg.MotorHeating = 1;
}

void initQHY22_768x560(void)
{
    ccdreg.HBIN = 2;
    ccdreg.VBIN = 4;
    ccdreg.LineSize = 1536;
    ccdreg.VerticalSize = 560;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    ccdreg.Exptime = QCam.camTime;
    ccdreg.Gain = QCam.camGain;
    ccdreg.Offset =  QCam.camOffset;
    QCam.cameraW = 768;
    QCam.cameraH = 560;
    
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}

void initQHY22_1536x1120(void)
{
    ccdreg.HBIN = 2;
    ccdreg.VBIN = 2;
    ccdreg.LineSize = 1536;
    ccdreg.VerticalSize = 1120;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    ccdreg.Exptime = QCam.camTime;
    ccdreg.Gain = QCam.camGain;
    ccdreg.Offset =  QCam.camOffset;
    QCam.cameraW = 1536;
    QCam.cameraH = 1120;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}
void initQHY22_3072x2240(void)
{
    ccdreg.HBIN = 1;
    ccdreg.VBIN = 1;
    ccdreg.LineSize = 3072;
    ccdreg.VerticalSize = 2240;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    ccdreg.Exptime = QCam.camTime;
    ccdreg.Gain = QCam.camGain;
    ccdreg.Offset =  QCam.camOffset;
    QCam.cameraW = 3072;
    QCam.cameraH = 2240;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDOld(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}


#ifdef __cplusplus
}
#endif
