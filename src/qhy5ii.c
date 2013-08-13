#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#include "qhyccd.h"
#include "qhy5ii.h"
#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern QHYCCD QCam;
bool qhy5iiDeNoise;
bool longExpMode;

int GainTable[73] = 
{
	0x004, 0x005, 0x006, 0x007, 0x008, 0x009, 0x00A, 0x00B, 0x00C, 0x00D, 0x00E,
	0x00F, 0x010, 0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019,
	0x01A, 0x01B, 0x01C, 0x01D, 0x01E, 0x01F, 0x051, 0x052, 0x053, 0x054, 0x055,
	0x056, 0x057, 0x058, 0x059, 0x05A, 0x05B, 0x05C, 0x05D, 0x05E, 0x05F, 0x6CE,
	0x6CF, 0x6D0, 0x6D1, 0x6D2, 0x6D3, 0x6D4, 0x6D5, 0x6D6, 0x6D7, 0x6D8, 0x6D9,
	0x6DA, 0x6DB, 0x6DC, 0x6DD, 0x6DE, 0x6DF, 0x6E0, 0x6E1, 0x6E2, 0x6E3, 0x6E4,
	0x6E5, 0x6E6, 0x6E7, 0x6FC, 0x6FD, 0x6FE, 0x6FF
};


void  QHY5II_DeNoise(bool on)
{
    qhy5iiDeNoise = on;
}

void SetQHY5IIGain(unsigned short gain)
{
    int i = 0;
    int Gain_Min = 0;
    int Gain_Max = 72;
    i = (Gain_Max - Gain_Min) * gain / 1000;
    I2CTwoWrite(0x35, GainTable[i]);
}

void QHY5IISetResolution(int x, int y)
{
    if (QCam.CAMERA == DEVICETYPE_QHY5II) 
    {
	I2CTwoWrite(0x09, 200);
	I2CTwoWrite(0x01, 8 + (1024 - y) / 2); // y start
	I2CTwoWrite(0x02, 16 + (1280 - x) / 2); // x start
	I2CTwoWrite(0x03, (unsigned short)(y - 1)); // y size
	I2CTwoWrite(0x04, (unsigned short)(x - 1)); // x size
	I2CTwoWrite(0x22, 0x00); // normal bin
	I2CTwoWrite(0x23, 0x00); // normal bin
    }
}

void  CorrectQHY5IIWH(int *w,int *h)
{
    if(*w <= 320 && *h <=240)
	{
		*w = 320;
		*h = 240;
		initQHY5II_QVGA();
	}
	else if(*w <= 400 && *h <= 400)
	{
		*w = 400;
		*h = 400;
		initQHY5II_R400();
	}
	else if(*w <= 640 && *h <= 480)
	{
		*w = 640;
		*h = 480;
		initQHY5II_VGA();
	}
	else if(*w <= 800 && *h <= 600)
	{
		*w = 800;
		*h = 600;
		initQHY5II_SVGA();
	}
	else if(*w <= 800 && *h <= 800)
	{
		*w = 800;
		*h = 800;
		initQHY5II_R800();
	}
	else if(*w < 960 && *h <= 720)
	{
		*w = 960;
		*h = 720;
		initQHY5II_960X720();
	}
	else if(*w <= 1024 && *h <= 768)
	{
		*w = 1024;
		*h = 768;
		initQHY5II_XGA();
	}
	else if(*w <= 1280 && *h <= 720)
	{
		*w = 1280;
		*h = 720;
		initQHY5II_1280X720();
	}
	else if(*w <= 1280 && *h <= 1024)
	{
		*w = 1280;
		*h = 1024;
		initQHY5II_SXGA();
	}
}

void  QHY5IIDeNoise(unsigned char *ImgData, int x, int y) 
{
	unsigned short ob;
	unsigned char cutOff;

	if (QCam.camGain > 800)
		cutOff = 200;
	else
		cutOff = 245;

	long s;
	short temp;

	s = 5;

	if (x == 1312) 
	{
                int j;
		for (j = 0; j < y; j++) 
		{
			ob = 0;
                        int i;
			for (i = 0; i < 14; i++) 
			{
				ob = ImgData[s] + ob;
				s++;
			}
			for (i = 0; i < 6; i++) 
			{
				ob = ImgData[s + 1291] + ob;
				s++;
			}

			s = s - 6;
			ob = ob / 20;
			for (i = 14; i < 1312; i++) 
			{
				temp = ImgData[s];
				temp = (temp + 10 - ob);

				if (temp > cutOff)
					temp = 255;
				if (temp < 0)
					temp = 0;
				ImgData[s] = (unsigned char)temp;

				s++;
			}
		}
	}
	else 
	{
                int j;
		for (j = 0; j < y; j++)
		{
			ob = 0;
                        int i;
			for (i = 0; i < 7; i++) 
			{
				ob = ImgData[s] + ob;
				s++;
			}

			for (i = 0; i < 3; i++) 
			{
				ob = ImgData[s + 645] + ob;
				s++;
			}

			s = s - 3;
			ob = ob / 10;
			for (i = 7; i < 1312 / 2; i++) 
			{
				temp = ImgData[s];
				temp = (temp + 10 - ob);
				if (temp > cutOff)
					temp = 255;
				if (temp < 0)
					temp = 0;
				ImgData[s] = (unsigned char)temp;

				s++;
			}
		}
	}
}

void  initQHY5II_SXGA(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 1024;
		QCam.ShowImgX = 1280;
		QCam.ShowImgY = 1024;
		QCam.ShowImgX_Start = 14;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(1312, 1024);
	}
	else 
	{
		QCam.ImgX = 1280;
		QCam.ImgY = 1024;
		QCam.ShowImgX = 1280;
		QCam.ShowImgY = 1024;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(1280, 1024);
	}

}

void  initQHY5II_XGA(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 768;
		QCam.ShowImgX = 1024;
		QCam.ShowImgY = 768;
		QCam.ShowImgX_Start = (1312 - 1024) / 2;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(1312, 768);
	}
	else 
	{
		QCam.ImgX = 1024;
		QCam.ImgY = 768;
		QCam.ShowImgX = 1024;
		QCam.ShowImgY = 768;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;	
	    QHY5IISetResolution(1024, 768);
	}
}

void  initQHY5II_SVGA(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 600;
		QCam.ShowImgX = 800;
		QCam.ShowImgY = 600;
		QCam.ShowImgX_Start = (1312 - 800) / 2;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(1312, 600);
	}
	else 
	{
		QCam.ImgX = 800;
		QCam.ImgY = 600;
		QCam.ShowImgX = 800;
		QCam.ShowImgY = 600;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(800, 600);
	}
}

void  initQHY5II_VGA(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 480;
		QCam.ShowImgX = 640;
		QCam.ShowImgY = 480;
		QCam.ShowImgX_Start = (1312 - 640) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 480);
	}
	else 
	{
		QCam.ImgX = 640;
		QCam.ImgY = 480;
		QCam.ShowImgX = 640;
		QCam.ShowImgY = 480;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(640, 480);
	}
}

void  initQHY5II_QVGA(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 240;
		QCam.ShowImgX = 320;
		QCam.ShowImgY = 240;
		QCam.ShowImgX_Start = (1312 - 320) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 240);
	}
	else 
	{
		QCam.ImgX = 320;
		QCam.ImgY = 240;
		QCam.ShowImgX = 320;
		QCam.ShowImgY = 240;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;	
		QHY5IISetResolution(320, 240);
	}
}

void  initQHY5II_R1024(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 1024;
		QCam.ShowImgX = 1024;
		QCam.ShowImgY = 1024;
		QCam.ShowImgX_Start = (1312 - 1024) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 1024);
	}
	else 
	{
		QCam.ImgX = 1024;
		QCam.ImgY = 1024;
		QCam.ShowImgX = 1024;
		QCam.ShowImgY = 1024;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1024, 1024);
	}

}

void  initQHY5II_R800(void) 
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 800;
		QCam.ShowImgX = 800;
		QCam.ShowImgY = 800;
		QCam.ShowImgX_Start = (1312 - 800) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 800);
	}
	else 
	{
		QCam.ImgX = 800;
		QCam.ImgY = 800;
		QCam.ShowImgX = 800;
		QCam.ShowImgY = 800;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(800, 800);
	}
}

void  initQHY5II_R400(void)
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 400;
		QCam.ShowImgX = 400;
		QCam.ShowImgY = 400;
		QCam.ShowImgX_Start = (1312 - 400) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 400);
	}
	else 
	{
		QCam.ImgX = 400;
		QCam.ImgY = 400;
		QCam.ShowImgX = 400;
		QCam.ShowImgY = 400;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(400, 400);
	}
}

void  initQHY5II_1280X720(void)
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 720;
		QCam.ShowImgX = 1280;
		QCam.ShowImgY = 720;
		QCam.ShowImgX_Start = (1312 - 1280) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 720);
	}
	else 
	{
		QCam.ImgX = 1280;
		QCam.ImgY = 720;
		QCam.ShowImgX = 1280;
		QCam.ShowImgY = 720;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1280, 720);
	}
}


void  initQHY5II_960X720(void)
{
	if (qhy5iiDeNoise) 
	{
		QCam.ImgX = 1312;
		QCam.ImgY = 720;
		QCam.ShowImgX = 960;
		QCam.ShowImgY = 720;
		QCam.ShowImgX_Start = (1312 - 960) / 2;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(1312, 720);
	}
	else 
	{
		QCam.ImgX = 960;
		QCam.ImgY = 720;
		QCam.ShowImgX = 960;
		QCam.ShowImgY = 720;
		QCam.ShowImgX_Start = 0;
		QCam.ShowImgY_Start = 0;
		QHY5IISetResolution(960, 720);
	}
}

void  SetExposureTime_QHY5II(unsigned long i) 
{
	// 需要输入的参数: CMOSCLK  REG04  REG05 REG0C REG09

	double CMOSCLK;

	if (QCam.transferspeed == 1)
		CMOSCLK = 48;
	else
		CMOSCLK = 24;

	double pixelPeriod;
	pixelPeriod = 1 / CMOSCLK; // unit: us

	double A, Q;
	double P1, P2;
	double RowTime; // unit: us
	unsigned long ExpTime; // unit: us
	unsigned short REG04, REG05, REG0C, REG09;
	double MaxShortExpTime;

	REG04 = I2CTwoRead(0x04);
	REG05 = I2CTwoRead(0x05);
	REG09 = I2CTwoRead(0x09);
	REG0C = I2CTwoRead(0x0C);
	ExpTime = i;

	A = REG04 + 1;
	P1 = 242;
	P2 = 2 + REG05 - 19;
	Q = P1 + P2;
	RowTime = (A + Q) * pixelPeriod;

	MaxShortExpTime = 15000 * RowTime - 180 * pixelPeriod -
		4 * REG0C * pixelPeriod;

	unsigned char buf[4];

	if (ExpTime > MaxShortExpTime) {
		I2CTwoWrite(0x09, 15000);

		ExpTime = (unsigned long )(ExpTime - MaxShortExpTime);

		buf[0] = 0;
		buf[1] = (unsigned char) (((ExpTime / 1000)&~0xff00ffff) >> 16);
		buf[2] = ((ExpTime / 1000)&~0xffff00ff) >> 8;
		buf[3] = ((ExpTime / 1000)&~0xffffff00);
                libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc1, 0x00, 0x00, buf, 4,0);
		ExpTime = (unsigned long)(ExpTime + MaxShortExpTime);
		longExpMode = true;
	}

	else {
		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = 0;
		libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc1, 0x00, 0x00, buf, 4,0);
		usleep(100);
		REG09 = (unsigned short)((ExpTime + 180 * pixelPeriod + 4 * REG0C * pixelPeriod)/ RowTime);
		if (REG09 < 1)
			REG09 = 1;
		I2CTwoWrite(0x09, REG09);
		ExpTime = (unsigned long)(REG09 * RowTime - 180 * pixelPeriod - 4 * REG0C * pixelPeriod);
		longExpMode = false;
	}
}


#ifdef __cplusplus
}
#endif

