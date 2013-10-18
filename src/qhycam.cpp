#include "qhycam.h"
#include <math.h>
#include <stdio.h>

#if 0
#ifdef __cplusplus
extern "C"
{
#endif
#endif
extern QUsb *qhyusb;

void sendRegisterQHYCCDOld(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber)
{
        unsigned long T;  //total actual transfer data  (byte)
        unsigned char REG[64];
        unsigned char time_H,time_M,time_L;

        T = reg.LineSize * reg.VerticalSize * 2 + reg.TopSkipPix * 2;
        #ifdef QHYCCD_DEBUG
        printf("sendRegisterOld,LineSize %ld\n",reg.LineSize);
        printf("sendRegisterOld,VSize %ld\n",reg.VerticalSize);
        printf("sendRegisterOld,TopSkipPix, %ld\n",reg.TopSkipPix);
        printf("sendRegisterOld,T %ld\n",T);
        printf("sendRegisterOld,P_Size %ld\n",P_Size);
        #endif
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
         #ifdef QHYCCD_DEBUG
        printf("sendRegisterOld,time_L, %d\n",time_L);
        printf("sendRegisterOld,time_M, %d\n",time_M);
        printf("sendRegisterOld,time_H, %d\n",time_H);
        #endif        
        
        REG[0]=reg.Gain ;
        
        REG[1]=reg.Offset ;
        
        REG[2]=time_H;
        REG[3]=time_M;
        REG[4]=time_L;
        
        REG[5]=reg.HBIN ;
        REG[6]=reg.VBIN ;
        
        REG[7]=qhyusb->MSB(reg.LineSize );
        REG[8]=qhyusb->LSB(reg.LineSize );
        
        REG[9]= qhyusb->MSB(reg.VerticalSize );
        REG[10]=qhyusb->LSB(reg.VerticalSize );
        
        REG[11]=qhyusb->MSB(reg.SKIP_TOP );
        REG[12]=qhyusb->LSB(reg.SKIP_TOP );
        
        REG[13]=qhyusb->MSB(reg.SKIP_BOTTOM );
        REG[14]=qhyusb->LSB(reg.SKIP_BOTTOM );
        
        REG[15]=qhyusb->MSB(reg.LiveVideo_BeginLine );
        REG[16]=qhyusb->LSB(reg.LiveVideo_BeginLine );
        
        REG[19]=qhyusb->MSB(reg.AnitInterlace );
        REG[20]=qhyusb->LSB(reg.AnitInterlace );
        
        REG[22]=reg.MultiFieldBIN ;
        
        REG[29]=qhyusb->MSB(reg.ClockADJ );
        REG[30]=qhyusb->LSB(reg.ClockADJ );
        
        REG[32]=reg.AMPVOLTAGE ;
        
        REG[33]=reg.DownloadSpeed ;
        
        REG[35]=reg.TgateMode ;
        REG[36]=reg.ShortExposure ;
        REG[37]=reg.VSUB ;
        REG[38]=reg.CLAMP;
        
        REG[42]=reg.TransferBIT ;
        
        REG[46]=reg.TopSkipNull ;
        
        REG[47]=qhyusb->MSB(reg.TopSkipPix );
        REG[48]=qhyusb->LSB(reg.TopSkipPix );
        
        REG[51]=reg.MechanicalShutterMode ;
        REG[52]=reg.DownloadCloseTEC ;
       
        REG[58]=reg.SDRAM_MAXSIZE ;
        
        REG[63]=reg.Trig ;
        
        REG[17]=qhyusb->MSB(*PatchNumber);
        REG[18]=qhyusb->LSB(*PatchNumber);
        
        REG[53]=(reg.WindowHeater&~0xf0)*16+(reg.MotorHeating&~0xf0);
        
        REG[57]=reg.ADCSEL ;
        
        qhyusb->qhyccd_vTXD(handle, 0xb5, REG, 64);

        qhyusb->qhyccd_vTXD(handle, 0xb5, REG, 64);
}

void sendRegisterQHYCCDNew(qhyccd_device_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber)
{
        unsigned long T;  //total actual transfer data  (byte)
        unsigned char REG[64];
        unsigned char time_H,time_M,time_L;

        T = reg.LineSize * reg.VerticalSize * 2 + reg.TopSkipPix * 2;

        #ifdef QHYCCD_DEBUG
        printf("sendRegisterNew,LineSize %ld\n",reg.LineSize);
        printf("sendRegisterNew,VSize %ld\n",reg.VerticalSize);
        printf("sendRegisterNew,TopSkipPix, %ld\n",reg.TopSkipPix);
        printf("sendRegisterNew,T %ld\n",T);
        printf("sendRegisterNew,P_Size %ld\n",P_Size);
        #endif

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
        #ifdef QHYCCD_DEBUG
        printf("sendRegisterNew,time_L, %d\n",time_L);
        printf("sendRegisterNew,time_M, %d\n",time_M);
        printf("sendRegisterNew,time_H, %d\n",time_H);
        #endif        
        
        REG[0]=reg.Gain ;
        
        REG[1]=reg.Offset ;
        
        REG[2]=time_H;
        REG[3]=time_M;
        REG[4]=time_L;
        
        REG[5]=reg.HBIN ;
        REG[6]=reg.VBIN ;
        
        REG[7]=qhyusb->MSB(reg.LineSize );
        REG[8]=qhyusb->LSB(reg.LineSize );
        
        REG[9]= qhyusb->MSB(reg.VerticalSize );
        REG[10]=qhyusb->LSB(reg.VerticalSize );
        
        REG[11]=qhyusb->MSB(reg.SKIP_TOP );
        REG[12]=qhyusb->LSB(reg.SKIP_TOP );
        
        REG[13]=qhyusb->MSB(reg.SKIP_BOTTOM );
        REG[14]=qhyusb->LSB(reg.SKIP_BOTTOM );
        
        REG[15]=qhyusb->MSB(reg.LiveVideo_BeginLine );
        REG[16]=qhyusb->LSB(reg.LiveVideo_BeginLine );
        
        REG[19]=qhyusb->MSB(reg.AnitInterlace );
        REG[20]=qhyusb->LSB(reg.AnitInterlace );
        
        REG[22]=reg.MultiFieldBIN ;
        
        REG[29]=qhyusb->MSB(reg.ClockADJ );
        REG[30]=qhyusb->LSB(reg.ClockADJ );
        
        REG[32]=reg.AMPVOLTAGE ;
        
        REG[33]=reg.DownloadSpeed ;
        
        REG[35]=reg.TgateMode ;
        REG[36]=reg.ShortExposure ;
        REG[37]=reg.VSUB ;
        REG[38]=reg.CLAMP;
        
        REG[42]=reg.TransferBIT ;
        
        REG[46]=reg.TopSkipNull ;
        
        REG[47]=qhyusb->MSB(reg.TopSkipPix );
        REG[48]=qhyusb->LSB(reg.TopSkipPix );
        
        REG[51]=reg.MechanicalShutterMode ;
        REG[52]=reg.DownloadCloseTEC ;
       
        REG[58]=reg.SDRAM_MAXSIZE ;
        
        REG[63]=reg.Trig ;
        
        REG[17]=qhyusb->MSB(*PatchNumber);
        REG[18]=qhyusb->LSB(*PatchNumber);
        
        REG[53]=(reg.WindowHeater&~0xf0)*16+(reg.MotorHeating&~0xf0);
        
        REG[57]=reg.ADCSEL ;
        
        qhyusb->qhyccd_vTXD(handle, 0xb5, REG, 64);

        qhyusb->qhyccd_vTXD(handle, 0xb5, REG, 64);
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
	i = qhyusb->qhyccd_iTXD(handle,data,length);
	return i;
}

unsigned char getFromInterrupt(qhyccd_device_handle *handle,unsigned char length,unsigned char *data)
{
	unsigned char i;
	
	i = qhyusb->qhyccd_iRXD(handle,data,length);
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
#if 0
#ifdef __cplusplus
}
#endif
#endif
