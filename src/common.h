#include <stdbool.h>
#include "qhyccd.h"

//#define QHYCCD_DEBUG

#define DEVICETYPE_QHY5II     51
#define DEVICETYPE_QHY5LII    56
#define DEVICETYPE_QHY6       60
#define DEVICETYPE_QHY9      390
#define DEVICETYPE_IC8300    391
#define DEVICETYPE_QHY11    1101
#define DEVICETYPE_QHY21     674
#define DEVICETYPE_QHY22     694
#define DEVICETYPE_QHY23     814
#define DEVICETYPE_QHY16000 1611
#define DEVICETYPE_UNKOWN      0

enum CONTROL_ID
{
    CONTROL_WBR=0,
    CONTROL_WBB,
    CONTROL_WBG,
    CONTROL_GAIN,
    CONTROL_OFFSET,
    CONTROL_EXPOSURE,
    CONTROL_SPEED,
    CONTROL_TRANSFERBIT,
    CONTROL_USBTRAFFIC,
    CONTROL_ROWNOISERE,
    CONTROL_HDRMODE,
    CONTROL_COOLER
};

int OpenCameraByID(int camid);

void InitCamera(void);

void CloseCamera(void);

void InitOthers(void);

void SetBin(int w,int h);

void SetTransferBit(int Bit);

void SetResolution(int x,int y);

void SetExposeTime(double exptime);

void SetGain(unsigned short gain);

void SetOffset(unsigned char offset);

void SetSpeed(bool isHigh);

void SetUSBTraffic(int i);

void SetWBBlue(int blue);

void SetWBGreen(int green);

void SetWBRed(int red);

void BeginLive(void);

void Bin2x2(unsigned char *ImgData,int w,int h);

void GetFrame(void *data,int *pW,int *pH,int *pBpp,int *lvlStat,int *lvlstatR,int *lvlstatG,int *lvlstatB);

void GetImageData(int w,int h,int bpp,int channels,unsigned char *rawArray);

void GetROIImageData(int w,int h,int bpp,int channels,unsigned char *rawArray);

int GetMaxFrameLength(void);

bool IsAstroCcd(int m_devType);

bool IsHighSpeed(void);

void GetImageFormat(int *w,int *h,int *bpp,int *channels);

void EepromRead(unsigned char addr, unsigned char* data, unsigned short len);

void GuideControl(unsigned char Direction,long PulseTime);

void BeginCooler(unsigned char PWM);

double GetTemp(void);

void StopCooler(void);

void StopLive(void);

