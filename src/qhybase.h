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

#include "qhycam.h"
#include "shareparameters.h"
#include "qhyccdcamdef.h"
#include "qhyccderr.h"
#include <pthread.h>
#include <opencv/cv.h>


#ifndef QHYBASEDEF
#define QHYBASEDEF

enum CONTROL_ID
{
    CONTROL_BRIGHTNESS = 0,
    CONTROL_CONTRAST,
    CONTROL_WBR,
    CONTROL_WBB,
    CONTROL_WBG,
    CONTROL_GAMMA,
    CONTROL_GAIN,
    CONTROL_OFFSET,
    CONTROL_EXPOSURE,
    CONTROL_SPEED,
    CONTROL_TRANSFERBIT,
    CONTROL_CHANNELS,
    CONTROL_USBTRAFFIC,
    CONTROL_ROWNOISERE,
    CONTROL_CURTEMP,
    CONTROL_CURPWM,
    CONTROL_MANULPWM,
    CONTROL_CFWPORT,
    CAM_BIN1X1MODE,
    CAM_BIN2X2MODE,
    CAM_BIN3X3MODE,
    CAM_BIN4X4MODE
};

class QHYBASE:public QHYCAM
{
public:
    QHYBASE()
    {
    }
    ~QHYBASE(){};
    virtual qhyccd_handle *ConnectCamera(){return NULL;}
    virtual int ConnectCamera(qhyccd_device *d,qhyccd_handle **h){return 0;}
    virtual int DisConnectCamera(qhyccd_handle *h){return 0;}
    virtual int InitChipRegs(qhyccd_handle *h){return 0;}
    virtual int SetChipOffset(qhyccd_handle *h,unsigned char offset){return 0;}
    virtual int SetChipExposeTime(qhyccd_handle *h,double t){return 0;}
    virtual int SetChipGain(qhyccd_handle *h,double gain){return 0;}
    virtual int SetChipGainColor(qhyccd_handle *h,double gain, double RG, double BG){return 0;}
    virtual int SetChipWBRed(qhyccd_handle *h,double red){return 0;}
    virtual int SetChipWBGreen(qhyccd_handle *h,double green){return 0;}
    virtual int SetChipWBBlue(qhyccd_handle *h,double blue){return 0;}
    virtual double GetChipWBRed(){return 0.0;}
    virtual double GetChipWBBlue(){return 0.0;}
    virtual double GetChipWBGreen(){return 0.0;}
    virtual double GetChipExposeTime(){return 0.0;}
    virtual double GetChipGain(){return 0.0;}
    virtual double GetChipOffset(){return 0.0;}
    virtual double GetChipSpeed(){return 0.0;}
    virtual double GetChipUSBTraffic(){return 0.0;}
    virtual double GetChipBitsMode(){return 0.0;}
    virtual double GetChipChannels(){return 1.0;}
    virtual double GetChipCoolTemp(qhyccd_handle *h){return currentTEMP;}
    virtual double GetChipCoolPWM(){return currentPWM;}
    virtual int GetControlMinMaxStepValue(CONTROL_ID controlId,double *min,double *max,double *step){return 0;}
    virtual int CorrectWH(int *w,int *h){return 0;}
    virtual int SetChipResolution(qhyccd_handle *h,int x,int y){return 0;}
    virtual int BeginChipExposure(qhyccd_handle *h){return 0;}
    virtual int StopChipExposure(qhyccd_handle *h){return 0;}
    virtual int GetFullFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData){return 0;}

    virtual int BeginSingleExposure(qhyccd_handle *h){return 0;}
    virtual int StopSingleExposure(qhyccd_handle *h){return 0;}
    virtual int GetSingleFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData){return 0;}

    virtual int BeginLiveExposure(qhyccd_handle *h){return 0;}
    virtual int StopLiveExposure(qhyccd_handle *h){return 0;}
    virtual int GetLiveFrame(qhyccd_handle *h,int *pW,int *pH,int * pBpp,int *pChannels,unsigned char *ImgData){return 0;}

    virtual int SetChipUSBTraffic(qhyccd_handle *h,int i){return 0;}
    virtual int DeChipRowNoise(qhyccd_handle *h,bool value){return 0;}
    virtual int GetChipMemoryLength(){return 0;}
    virtual bool IsSupportHighSpeed(){return false;}
    virtual int IsTempControl(){return 0;}
    virtual int IsChipHasFunction(CONTROL_ID id){return 0;}
    virtual int IsColorCam(){return 0;}
    virtual int IsCoolCam(){return 0;}
    virtual int SetChipCoolPWM(qhyccd_handle *h,double PWM){return 0;}
    virtual int AutoTempControl(qhyccd_handle *h,double ttemp){return 0;}
    virtual double GetTempFromChip(qhyccd_handle *h){return 0.0;}
    virtual unsigned char GetPWMFromChip(qhyccd_handle *h){return 0;}
    virtual int SetChipSpeed(qhyccd_handle *h,int i){return 0;}
    virtual int SetChipBitsMode(qhyccd_handle *h,int bits){return 0;};    
    virtual int SetChipChannels(int channels){return 0;}
    virtual int SetChipBinMode(qhyccd_handle *h,int wbin,int hbin){return 0;};
    virtual struct BINRESOLUTION *IsSupportBIN(int bin){return NULL;};

    virtual void ConvertDataBIN11(unsigned char * ImgData,int x, int y, unsigned short TopSkipPix){};
    virtual void ConvertDataBIN22(unsigned char * ImgData,int x, int y, unsigned short TopSkipPix){};
    virtual void ConvertDataBIN33(unsigned char * ImgData,int x, int y, unsigned short TopSkipPix){};
    virtual void ConvertDataBIN44(unsigned char * ImgData,int x, int y, unsigned short TopSkipPix){};
    virtual void BlackLevelCalibration(int x, int y, int bpp,int channels,unsigned char *rawArray, unsigned char *dst){};
    virtual int Send2GuiderPort(qhyccd_handle *h,unsigned char Direction,unsigned short PulseTime){return 0;}
    virtual int Send2CFWPort(qhyccd_handle *h,int pos){return 0;} 
public:
    void ControlCamTemp(qhyccd_handle *h,double MAXPWM);
    void Bit16To8_Stretch(unsigned char *InputData16,unsigned char *OutputData8,int imageX,int imageY,unsigned short B,unsigned short W);
    void HistInfo(int x,int y,unsigned char *InBuf,unsigned char *OutBuf);
    
    /* current camera width */
    int camx;
    /* current camera height */
    int camy;
    /* current camera width bin */
    int camxbin;
    /* current camera height bin */
    int camybin;
    /* current camera bits mode */
    int cambits;    
    /* current camera channels */
    int camchannels;
    /* current usbtraffic */
    int usbtraffic;
    /* current usb speed mode */
    int usbspeed;
    /* current cam expose time */
    double camtime;
    /* current cam gain */
    double camgain;
    /* current cam offset */
    int camoffset;
    /* current white blance red value */
    double camred;
    /* current white blance blue value */
    double camblue;
    /* current white blance green value */
    double camgreen; 
    /* tmp buffer pointer for usb transfer */
    unsigned char *rawarray;
    
    /* for image roi */
    int roixstart;
    int roiystart;    
    int roixsize;
    int roiysize;
    IplImage *monoimg;
    IplImage *roiimg;
    unsigned char *roiarray;

    /* for debyer */
    IplImage *colorimg;

    //for temprature control
    double targetTEMP;
    double currentTEMP;
    double currentPWM;
    int nowVoltage;
    bool flag_timer;
    bool flag_timer_2;//控制温控每4秒调节一次

    //pid算法调控 参数
    double NowError;//第k步差值
    double PrevError;//第k-2步，初始值
    double LastError;//第k-1步，初始值
    double Proportion;//比例系数
    double Integral;//积分系数
    double Derivative;//微分系数


    pthread_t tempid;
    bool thrdrun;
    bool flagtemp;
    bool flagquit;
/*
        void Bin2x2(unsigned char *ImgData,int w,int h);
        void CorrectGamma(unsigned char *Src,long totalP);
        void BuildGammaTable();
	void SetGamma(double gamma);
	void SetBrightness(int b);
	void SetContrast(double c);
*/
        
};


#endif
