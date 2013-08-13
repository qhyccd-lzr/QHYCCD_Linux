#include "qhy9.h"
#include "QHYCAM.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern QHYCCD QCam;
extern CCDREG ccdreg;

void  CorrectQHY9WH(int *w,int *h)
{
    if(*w <= 896 && *h <= 644)
	{
		*w = 896;
		*h = 644;
		initQHY9_896x644();
	}
	else if(*w <= 1196 && *h <= 858)
	{
		*w = 1196;
		*h = 858;
		initQHY9_1196x858();
	}
	else if(*w <= 1792 && *h <= 1287)
	{
		*w = 1792;
		*h = 1287;
		initQHY9_1792x1287();
	}
	else
	{
		*w = 3584;
		*h = 2574;
		initQHY9_3584x2574();
	}
}

void initQHY9_regs(void)
{
    ccdreg.devname = "QHY9-0";
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

void initQHY9_896x644(void)
{
    ccdreg.HBIN = 4;
    ccdreg.VBIN = 4;
    ccdreg.LineSize = 896;
    ccdreg.VerticalSize = 644;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 896;
    QCam.cameraH = 644;
    
    int Total_P,PatchNumber;
    sendRegisterQHYCCDNew(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}

void initQHY9_1196x858(void)
{
    ccdreg.HBIN = 3;
    ccdreg.VBIN = 3;
    ccdreg.LineSize = 1196;
    ccdreg.VerticalSize = 858;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 1196;
    QCam.cameraH = 858;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDNew(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}
void initQHY9_1792x1287(void)
{
    ccdreg.HBIN = 2;
    ccdreg.VBIN = 2;
    ccdreg.LineSize = 1792;
    ccdreg.VerticalSize = 1287;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 1792;
    QCam.cameraH = 1287;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDNew(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}
void initQHY9_3584x2574(void)
{
    ccdreg.HBIN = 1;
    ccdreg.VBIN = 1;
    ccdreg.LineSize = 3584;
    ccdreg.VerticalSize = 2574;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 3584;
    QCam.cameraH = 2574;
    int Total_P,PatchNumber;
    sendRegisterQHYCCDNew(QCam.ccd_handle,ccdreg,QCam.cameraW*QCam.cameraH*2,&Total_P,&PatchNumber);
    #ifdef QHYCCD_DEBUG
    printf("Total_P %d,PathNumber %d\n",Total_P,PatchNumber);
    #endif
}


#ifdef __cplusplus
}
#endif
