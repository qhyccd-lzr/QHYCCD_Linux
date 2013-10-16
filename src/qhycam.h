#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include "common.h"


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
