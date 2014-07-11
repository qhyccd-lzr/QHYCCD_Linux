/*
 QHYCCD SDK
 
 Copyright (c) 2014 QHYCCD.
 All Rights Reserved.
 
 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.
 
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.
 
 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59
 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 The full GNU General Public License is included in this distribution in the
 file called LICENSE.
 */

#include "qhybase.h"
#include <opencv/cv.h>

#ifndef QHY12DEF
#define QHY12DEF

class QHY12:public QHYBASE
{
public:
    QHY12();
    ~QHY12();
    int ConnectCamera(libusb_device *d,qhyccd_handle **h);
    int DisConnectCamera(qhyccd_handle *h);
    int InitChipRegs(qhyccd_handle *h);
    int IsChipHasFunction(CONTROL_ID id);
    int IsColorCam();
    int IsCoolCam();
    int ReSetParams2cam(qhyccd_handle *h);
    int SetChipGain(qhyccd_handle *h,double gain);
    int SetChipExposeTime(qhyccd_handle *h,double i);
    int SetChipSpeed(qhyccd_handle *h,int i);
    int SetChipOffset(qhyccd_handle *h,double offset);
    int SetChipBinMode(qhyccd_handle *h,int wbin,int hbin);
    int Send2CFWPort(qhyccd_handle *h,int pos);
    int GetControlMinMaxStepValue(CONTROL_ID controlId,double *min,double *max,double *step);
    int GetChipMemoryLength();
    double GetChipExposeTime();
    double GetChipGain();
    double GetChipSpeed();
    double GetChipOffset();
    double GetChipBitsMode();
    double GetChipCoolTemp(qhyccd_handle *h);
    double GetChipCoolPWM();
    int CorrectWH(int *w,int *h);
    int InitBIN11Mode();
    int InitBIN22Mode();
    int InitBIN44Mode();
    int SetChipResolution(qhyccd_handle *h,int x,int y);
    int BeginSingleExposure(qhyccd_handle *h);
    int StopSingleExposure(qhyccd_handle *h);
    int GetSingleFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData);
    int BeginLiveExposure(qhyccd_handle *h);
    int StopLiveExposure(qhyccd_handle *h);
    int GetLiveFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData);
    int AutoTempControl(qhyccd_handle *h,double ttemp);
    int SetChipCoolPWM(qhyccd_handle *h,double PWM);
    void ConvertDataBIN11(unsigned char * Data,int x, int y, unsigned short PixShift);
    void ConvertDataBIN22(unsigned char * Data,int x, int y, unsigned short PixShift);
    void ConvertDataBIN44(unsigned char * Data,int x, int y, unsigned short PixShift);
};
#endif
