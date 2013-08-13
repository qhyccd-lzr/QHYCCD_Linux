#include "QHYCAM.h"
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

CCDREG ccdreg;

void sendRegisterQHYCCDOld(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber)
{
        unsigned long T;  //total actual transfer data  (byte)
        unsigned char REG[64];
        unsigned char time_H,time_M,time_L;

        T = reg.LineSize * reg.VerticalSize * 2 + reg.TopSkipPix * 2;

        if (T % P_Size) {
                *Total_P = T / P_Size+1;
                *PatchNumber = *Total_P * P_Size - T;
        } else {
                *Total_P = T / P_Size;
                *PatchNumber = 0;
        }

        time_L=(reg.Exptime % 256);
        time_M=(reg.Exptime-time_L)/256;
        time_H=(reg.Exptime-time_L-time_M*256)/65536;
        
        
        REG[0]=reg.Gain ;
        
        REG[1]=reg.Offset ;
        
        REG[2]=time_H;
        REG[3]=time_M;
        REG[4]=time_L;
        
        REG[5]=reg.HBIN ;
        REG[6]=reg.VBIN ;
        
        REG[7]=MSB(reg.LineSize );
        REG[8]=LSB(reg.LineSize );
        
        REG[9]= MSB(reg.VerticalSize );
        REG[10]=LSB(reg.VerticalSize );
        
        REG[11]=MSB(reg.SKIP_TOP );
        REG[12]=LSB(reg.SKIP_TOP );
        
        REG[13]=MSB(reg.SKIP_BOTTOM );
        REG[14]=LSB(reg.SKIP_BOTTOM );
        
        REG[15]=MSB(reg.LiveVideo_BeginLine );
        REG[16]=LSB(reg.LiveVideo_BeginLine );
        
        REG[19]=MSB(reg.AnitInterlace );
        REG[20]=LSB(reg.AnitInterlace );
        
        REG[22]=reg.MultiFieldBIN ;
        
        REG[29]=MSB(reg.ClockADJ );
        REG[30]=LSB(reg.ClockADJ );
        
        REG[32]=reg.AMPVOLTAGE ;
        
        REG[33]=reg.DownloadSpeed ;
        
        REG[35]=reg.TgateMode ;
        REG[36]=reg.ShortExposure ;
        REG[37]=reg.VSUB ;
        REG[38]=reg.CLAMP;
        
        REG[42]=reg.TransferBIT ;
        
        REG[46]=reg.TopSkipNull ;
        
        REG[47]=MSB(reg.TopSkipPix );
        REG[48]=LSB(reg.TopSkipPix );
        
        REG[51]=reg.MechanicalShutterMode ;
        REG[52]=reg.DownloadCloseTEC ;
       
        REG[58]=reg.SDRAM_MAXSIZE ;
        
        REG[63]=reg.Trig ;
        
        REG[17]=MSB(*PatchNumber);
        REG[18]=LSB(*PatchNumber);
        
        REG[53]=(reg.WindowHeater&~0xf0)*16+(reg.MotorHeating&~0xf0);
        
        REG[57]=reg.ADCSEL ;
        
        qhyccd_vTXD(handle, 0xb5, REG, 64);

        qhyccd_vTXD(handle, 0xb5, REG, 64);
}

void sendRegisterQHYCCDNew(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber)
{
        unsigned long T;  //total actual transfer data  (byte)
        unsigned char REG[64];
        unsigned char time_H,time_M,time_L;

        T = reg.LineSize * reg.VerticalSize * 2 + reg.TopSkipPix * 2;

        if (T % P_Size) {
                *Total_P = T / P_Size+1;
                *PatchNumber = (*Total_P * P_Size - T) / 2 + 16;
        } else {
                *Total_P = T / P_Size;
                *PatchNumber = 16;
        }

        time_L=(reg.Exptime % 256);
        time_M=(reg.Exptime-time_L)/256;
        time_H=(reg.Exptime-time_L-time_M*256)/65536;
        
        
        REG[0]=reg.Gain ;
        
        REG[1]=reg.Offset ;
        
        REG[2]=time_H;
        REG[3]=time_M;
        REG[4]=time_L;
        
        REG[5]=reg.HBIN ;
        REG[6]=reg.VBIN ;
        
        REG[7]=MSB(reg.LineSize );
        REG[8]=LSB(reg.LineSize );
        
        REG[9]= MSB(reg.VerticalSize );
        REG[10]=LSB(reg.VerticalSize );
        
        REG[11]=MSB(reg.SKIP_TOP );
        REG[12]=LSB(reg.SKIP_TOP );
        
        REG[13]=MSB(reg.SKIP_BOTTOM );
        REG[14]=LSB(reg.SKIP_BOTTOM );
        
        REG[15]=MSB(reg.LiveVideo_BeginLine );
        REG[16]=LSB(reg.LiveVideo_BeginLine );
        
        REG[19]=MSB(reg.AnitInterlace );
        REG[20]=LSB(reg.AnitInterlace );
        
        REG[22]=reg.MultiFieldBIN ;
        
        REG[29]=MSB(reg.ClockADJ );
        REG[30]=LSB(reg.ClockADJ );
        
        REG[32]=reg.AMPVOLTAGE ;
        
        REG[33]=reg.DownloadSpeed ;
        
        REG[35]=reg.TgateMode ;
        REG[36]=reg.ShortExposure ;
        REG[37]=reg.VSUB ;
        REG[38]=reg.CLAMP;
        
        REG[42]=reg.TransferBIT ;
        
        REG[46]=reg.TopSkipNull ;
        
        REG[47]=MSB(reg.TopSkipPix );
        REG[48]=LSB(reg.TopSkipPix );
        
        REG[51]=reg.MechanicalShutterMode ;
        REG[52]=reg.DownloadCloseTEC ;
       
        REG[58]=reg.SDRAM_MAXSIZE ;
        
        REG[63]=reg.Trig ;
        
        REG[17]=MSB(*PatchNumber);
        REG[18]=LSB(*PatchNumber);
        
        REG[53]=(reg.WindowHeater&~0xf0)*16+(reg.MotorHeating&~0xf0);
        
        REG[57]=reg.ADCSEL ;
        
        qhyccd_vTXD(handle, 0xb5, REG, 64);

        qhyccd_vTXD(handle, 0xb5, REG, 64);
}

void setDC201FromInterrupt(qhyccd_device_handle *handle,unsigned char PWM,unsigned char FAN)
{
	unsigned char Buffer[3];
	Buffer[0]=0x01;
	
	if (PWM==0)
	{
		Buffer[2]=Buffer[2] &~ 0x80;
		Buffer[1]=0;
	}
	else
	{
		Buffer[1]=PWM;
		Buffer[2]=Buffer[2] | 0x80;
	}
	
	if (FAN==0) 	
	{
		Buffer[2]=Buffer[2] &~ 0x01;
	}
        else            
        {
        	Buffer[2]=Buffer[2] | 0x01;
        }
        sendInterrupt(handle,3, Buffer);
}
signed short getDC201FromInterrupt(qhyccd_device_handle *handle)
{
	unsigned char Buffer[4];
	signed short x;
	
	getFromInterrupt(handle,4,Buffer);
	x=Buffer[1]*256+Buffer[2];
	return x;
}
unsigned char sendInterrupt(qhyccd_device_handle *handle,unsigned char length,unsigned char *data)
{
	unsigned char i;
	i = qhyccd_iTXD(handle,data,length);
	return i;
}

unsigned char getFromInterrupt(qhyccd_device_handle *handle,unsigned char length,unsigned char *data)
{
	unsigned char i;
	
	i = qhyccd_iRXD(handle,data,length);
	return i;
}

double GetCCDTemp(qhyccd_device_handle *handle)
{
	signed short v;
	v = (signed short)(1.024*(float)getDC201FromInterrupt(handle));
	return mVToDegree((double)v);
}

double RToDegree(double R)
{
	double 	T;
	double LNR;
	
	if (R>400) R=400;
	if (R<1) R=1;
	
	LNR=log(R);
	T=1/( 0.002679+0.000291*LNR+	LNR*LNR*LNR*4.28e-7  );
	T=T-273.15;
	return T;
}


double mVToDegree(double V)
{
	double R;
	double T;

	R=33/(V/1000+1.625)-10;
	T=RToDegree(R);

	return T;
}

#ifdef __cplusplus
}
#endif
