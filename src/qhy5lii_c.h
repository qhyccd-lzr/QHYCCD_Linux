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

#ifndef QHY5LII_CDEF
#define QHY5LII_CDEF

class QHY5LII_C:public QHYBASE
{
public:
    QHY5LII_C();
    ~QHY5LII_C();
    void InitCmos(qhyccd_handle *h);
    int ConnectCamera(libusb_device *d,qhyccd_handle **h);
    int DisConnectCamera(qhyccd_handle *h);
    int InitChipRegs(qhyccd_handle *h);
    int Init1280x960(qhyccd_handle *h);
    int Init1024x768(qhyccd_handle *h);
    int Init800x600(qhyccd_handle *h);
    int Init640x480(qhyccd_handle *h);
    int Init320x240(qhyccd_handle *h);
    int IsChipHasFunction(CONTROL_ID id);
    int IsColorCam();
    int IsCoolCam();
    int ReSetParams2cam(qhyccd_handle *h);
    int SetChipGain(qhyccd_handle *h,double gain);
    int SetChipGainColor(qhyccd_handle *h,double gain, double RG, double BG);
    int SetChipWBRed(qhyccd_handle *h,double red);
    int SetChipWBGreen(qhyccd_handle *h,double green);
    int SetChipWBBlue(qhyccd_handle *h,double blue);
    int SetChipExposeTime(qhyccd_handle *h,double i);
    int SetChipSpeed(qhyccd_handle *h,int i);
    int SetChipBitsMode(qhyccd_handle *h,int bits);
    int SetChipChannels(int channels);
    int SetChipBinMode(qhyccd_handle *h,int wbin,int hbin);
    int SetChipUSBTraffic(qhyccd_handle *h,int i);
    int SetPll(qhyccd_handle *h,unsigned char clk);
    int GetControlMinMaxStepValue(CONTROL_ID controlId,double *min,double *max,double *step);
    int GetChipMemoryLength();
    double GetChipWBRed();
    double GetChipWBBlue();
    double GetChipWBGreen();
    double GetChipExposeTime();
    double GetChipGain();
    double GetChipSpeed();
    double GetChipUSBTraffic();
    double GetChipBitsMode();
    double GetChipChannels();
    int CorrectWH(int *w,int *h);
    int SetChipResolution(qhyccd_handle *h,int x,int y);
    int BeginSingleExposure(qhyccd_handle *h);
    int StopSingleExposure(qhyccd_handle *h);
    int GetSingleFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData);
    int BeginLiveExposure(qhyccd_handle *h);
    int StopLiveExposure(qhyccd_handle *h);
    int GetLiveFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData);
    int Send2GuiderPort(qhyccd_handle *h,unsigned char Direction,unsigned short PulseTime);
    void SWIFT_MSBLSB(unsigned char *ImgData);
private:
       int expmode;
       int pllratio;
};
#endif
