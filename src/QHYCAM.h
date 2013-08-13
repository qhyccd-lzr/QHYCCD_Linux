#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include "common.h"

typedef struct ccdreg 
{
    char* devname;
    unsigned char Gain;
    unsigned char Offset;
    unsigned long Exptime;
    unsigned char HBIN;
    unsigned char VBIN;
    unsigned short LineSize;
    unsigned short VerticalSize;
    unsigned short SKIP_TOP;
    unsigned short SKIP_BOTTOM;
    unsigned short LiveVideo_BeginLine;
    unsigned short AnitInterlace;
    unsigned char MultiFieldBIN;
    unsigned char AMPVOLTAGE;
    unsigned char DownloadSpeed;
    unsigned char TgateMode;
    unsigned char ShortExposure;
    unsigned char VSUB;
    unsigned char CLAMP;
    unsigned char TransferBIT;
    unsigned char TopSkipNull;
    unsigned short TopSkipPix;
    unsigned char MechanicalShutterMode;
    unsigned char DownloadCloseTEC;
    unsigned char SDRAM_MAXSIZE;
    unsigned short ClockADJ;
    unsigned char Trig;
    unsigned char MotorHeating;   //0,1,2
    unsigned char WindowHeater;   //0-15
    unsigned char ADCSEL;
}CCDREG;

void sendRegisterQHYCCDOld(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber);

void sendRegisterQHYCCDNew(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber);
                  
void setDC201FromInterrupt(qhyccd_device_handle *handle,unsigned char PWM,unsigned char FAN);

signed short getDC201FromInterrupt(qhyccd_device_handle *handle);

unsigned char sendInterrupt(qhyccd_device_handle *handle,unsigned char length,unsigned char *data);

unsigned char getFromInterrupt(qhyccd_device_handle *handle,unsigned char length,unsigned char *data);

double GetCCDTemp(qhyccd_device_handle *handle);

double RToDegree(double R);

double mVToDegree(double V);
