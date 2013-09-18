#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#include "qhyccd.h"
#include "qhy5lii.h"
#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern QHYCCD QCam;
QHY5LII QCam5LII;

void SetQHY5LIIGain(unsigned short gain)
{
     if(gain == 1000)
          I2CTwoWrite(0x3100,0x0004);
     else
     {
         I2CTwoWrite(0x3100,0);

         if(QCam.isColor)
         {
             double RG,BG;
             RG = (double)QCam.wbred / 100;
             BG = (double)QCam.wbblue / 100;
             SetGainColorQHY5LII(gain,RG,BG);
         }
         else
         {
             SetGainMonoQHY5LII(gain);
         }
    }
}

void SetSpeedQHY5LII(int i)
{
    // i=0,1,2    0=12M  1=24M  2=48M
    unsigned char buf[2];
    buf[0] = i;

    libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc8, 0x00, 0x00, buf, 1,0);
}

void SWIFT_MSBLSBQHY5LII(unsigned char *ImgData)
{
	int i = 0;
	while(i<QCam.cameraW*QCam.cameraH*2)
	{
		ImgData[i] = ImgData[i+1];
		ImgData[i+1] = ImgData[i] << 4;
		i += 2;
	}
}

double setQHY5LREG_PLL(unsigned char clk)
{
	double i = 0;

	if (clk == 0) 
	{

		W_I2C_MICRON_Address16_OneRegister(0x302A, 14); // DIV           14
		W_I2C_MICRON_Address16_OneRegister(0x302C, 1); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x302E, 3); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x3030, 42); // MULTI          44

		W_I2C_MICRON_Address16_OneRegister(0x3082, 0x0029);
		// OPERATION_MODE_CTRL

		if (QCam5LII.CheckBoxQHY5LIILoneExpMode) {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x5330);
			// DIGITAL_TEST    5370: PLL BYPASS   1370  USE PLL
			i = 1.0;
		}
		else {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x1330);
			i = 1.0;
		}
		// 5330
		W_I2C_MICRON_Address16_OneRegister(0x305e, 0x00ff); // gain
		W_I2C_MICRON_Address16_OneRegister(0x3012, 0x0020);
		// coarse integration time

		W_I2C_MICRON_Address16_OneRegister(0x3064, 0x1802);
		// EMBEDDED_DATA_CTRL

		return i;

	}

	if (clk == 1) {

		W_I2C_MICRON_Address16_OneRegister(0x302A, 14); // DIV           14
		W_I2C_MICRON_Address16_OneRegister(0x302C, 1); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x302E, 3); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x3030, 65); // MULTI          44

		W_I2C_MICRON_Address16_OneRegister(0x3082, 0x0029);
		// OPERATION_MODE_CTRL

		if (QCam5LII.CheckBoxQHY5LIILoneExpMode) {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x5330);
			// DIGITAL_TEST    5370: PLL BYPASS   1370  USE PLL
			i = 1.0;
		}
		else {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x1330);
			i = 65 / 14 / 3;
		}
		W_I2C_MICRON_Address16_OneRegister(0x305e, 0x00ff); // gain
		W_I2C_MICRON_Address16_OneRegister(0x3012, 0x0020);
		// coarse integration time

		W_I2C_MICRON_Address16_OneRegister(0x3064, 0x1802);
		// EMBEDDED_DATA_CTRL

		return i;

	}

	if (clk == 2) {

		W_I2C_MICRON_Address16_OneRegister(0x302A, 14); // DIV           14
		W_I2C_MICRON_Address16_OneRegister(0x302C, 1); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x302E, 3); // DIV
		W_I2C_MICRON_Address16_OneRegister(0x3030, 57); // MULTI          44

		W_I2C_MICRON_Address16_OneRegister(0x3082, 0x0029);
		// OPERATION_MODE_CTRL

		if (QCam5LII.CheckBoxQHY5LIILoneExpMode) {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x5330);
			// DIGITAL_TEST    5370: PLL BYPASS   1370  USE PLL
			i = 1.0;
		}
		else {
			W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x1330);
			i = 57 / 14 / 3;
		}

		W_I2C_MICRON_Address16_OneRegister(0x305e, 0x00ff); // gain
		W_I2C_MICRON_Address16_OneRegister(0x3012, 0x0020);
		// coarse integration time

		W_I2C_MICRON_Address16_OneRegister(0x3064, 0x1802);
		// EMBEDDED_DATA_CTRL

		return i;

	}
	return i;
}

void SetQHY5LIIHDR(bool on) 
{
	if (on)
		I2CTwoWrite(0x3082, 0x0028);
	else
		I2CTwoWrite(0x3082, 0x0001);
}

void SetGainMonoQHY5LII(double gain)
{
	// gain input range 0-1000  输入范围已经归一化到0-1000

	int Gain_Min, Gain_Max;

	Gain_Min = 0;
	Gain_Max = 796;

	gain = (Gain_Max - Gain_Min) * gain / 1000;

	gain = gain / 10; // range:0-39.8

	unsigned short REG30B0;

	if (QCam5LII.CheckBoxQHY5LIILoneExpMode)
		REG30B0 = 0X5330;
	else
		REG30B0 = 0X1330;

	unsigned short baseDGain;

	double C[8] = {10, 8, 5, 4, 2.5, 2, 1.25, 1};
	double S[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int A[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int B[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	double Error[8];

        int i;
	for (i = 0; i < 8; i++) 
        {
		S[i] = gain / C[i];
		A[i] = (int)(S[i]);
		B[i] = (int)((S[i] - A[i]) / 0.03125);
		if (A[i] > 7)
			A[i] = 10000; // 限制A的范围在1-3
		if (A[i] == 0)
			A[i] = 10000; // 限制A的范围在1-3
		Error[i] = fabs(((double)(A[i])+(double)(B[i]) * 0.03125) * C[i] - gain);
	}

	double minValue;
	int minValuePosition;

	minValue = Error[0];
	minValuePosition = 0;

	for (i = 0; i < 8; i++) {

		if (minValue > Error[i]) {
			minValue = Error[i];
			minValuePosition = i;
		}
	}
	// Form1->Edit6->Text=Form1->Edit6->Text+"minPosition="+AnsiString(minValuePosition)+"minValue="+minValue;
	int AA, BB, CC;
	double DD;
	double EE;

	AA = A[minValuePosition];
	BB = B[minValuePosition];
	if (minValuePosition == 0) {
		CC = 8;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x30);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 1) {
		CC = 8;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x30);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 2) {
		CC = 4;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x20);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 3) {
		CC = 4;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x20);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 4) {
		CC = 2;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x10);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 5) {
		CC = 2;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x10);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 6) {
		CC = 1;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x00);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 7) {
		CC = 1;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x00);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}

	EE = fabs(((double)(AA)+(double)(BB) * 0.03125) * CC * DD - gain);

	baseDGain = BB + AA * 32;
	W_I2C_MICRON_Address16_OneRegister(0x305E, baseDGain);

}

void SetGainColorQHY5LII(double gain, double RG, double BG)
{
	// gain input range 0-1000  输入范围已经归一化到0-1000
	int Gain_Min, Gain_Max;

	Gain_Min = 0;
	Gain_Max = 398;

	if (gain < 26)
		gain = 26;

	gain = (Gain_Max - Gain_Min) * gain / 1000;

	gain = gain / 10; // range:0-39.8

	unsigned short REG30B0;

	if (QCam5LII.CheckBoxQHY5LIILoneExpMode)
		REG30B0 = 0x5330;
	else
		REG30B0 = 0x1330;

	unsigned short baseDGain;

	double C[8] = {10, 8, 5, 4, 2.5, 2, 1.25, 1};
	double S[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int A[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int B[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	double Error[8];

        int i;
	for (i = 0; i < 8; i++) 
	{
		S[i] = gain / C[i];
		A[i] = (int)(S[i]);
		B[i] = (int)((S[i] - A[i]) / 0.03125);
		if (A[i] > 3)
			A[i] = 10000; // 限制A的范围在1-3
		if (A[i] == 0)
			A[i] = 10000; // 限制A的范围在1-3
		Error[i] = fabs(((double)(A[i])+(double)(B[i]) * 0.03125) * C[i] - gain);
	}

	double minValue;
	int minValuePosition;

	minValue = Error[0];
	minValuePosition = 0;

	for (i = 0; i < 8; i++) 
	{
		if (minValue > Error[i]) 
		{
			minValue = Error[i];
			minValuePosition = i;
		}
	}

	int AA, BB, CC;
	double DD;
	double EE;

	AA = A[minValuePosition];
	BB = B[minValuePosition];
	if (minValuePosition == 0) {
		CC = 8;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x30);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 1) {
		CC = 8;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x30);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 2) {
		CC = 4;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x20);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 3) {
		CC = 4;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x20);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 4) {
		CC = 2;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x10);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 5) {
		CC = 2;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x10);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}
	if (minValuePosition == 6) {
		CC = 1;
		DD = 1.25;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x00);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD308);
	}
	if (minValuePosition == 7) {
		CC = 1;
		DD = 1;
		W_I2C_MICRON_Address16_OneRegister(0x30B0, (REG30B0 &~0x0030) + 0x00);
		W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0XD208);
	}

	EE = fabs(((double)(AA)+(double)(BB) * 0.03125) * CC * DD - gain);

	baseDGain = BB + AA * 32;

	W_I2C_MICRON_Address16_OneRegister(0x3058, (unsigned short)(baseDGain*BG));
	W_I2C_MICRON_Address16_OneRegister(0x305a, (unsigned short)(baseDGain*RG));
	W_I2C_MICRON_Address16_OneRegister(0x305c, baseDGain);
	W_I2C_MICRON_Address16_OneRegister(0x3056, baseDGain);
}

void Set14Bit(int i)
{
	unsigned char buf[2];
	buf[0] = i;

        libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xcd, 0x00, 0x00, buf, 1,0);
}

void initQHY5LII_XGA(void) 
{
	int x = 1024;
	int y = 768;
	QCam.ImgX = 1024;
	QCam.ImgY = 768;
	QCam.ShowImgX = 1024;
	QCam.ShowImgY = 768;
	QCam.ShowImgX_Start = 0;
	QCam.ShowImgY_Start = 0;

	InitQHY5LIIRegs();
	QCam5LII.QHY5L_PLL_Ratio = (int)setQHY5LREG_PLL(0);

	int xstart = 4 + (1280 - 1024) / 2;
	int ystart = 4 + (960 - 768) / 2;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 795); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1388); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER

}

void initQHY5LII_1280X960(void)
{
	int x = 1280;
	int y = 960;
	QCam.ImgX = 1280;
	QCam.ImgY = 960;
	QCam.ShowImgX = 1280;
	QCam.ShowImgY = 960;
	QCam.ShowImgX_Start = 0;
	QCam.ShowImgY_Start = 0;

	InitQHY5LIIRegs();
	QCam5LII.QHY5L_PLL_Ratio = (int)setQHY5LREG_PLL(0);

	int xstart = 4;
	int ystart = 4;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 990); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1650); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER

}

void initQHY5LII_SVGA(void)
{
	int x = 800;
	int y = 600;
	QCam.ImgX = 800;
	QCam.ImgY = 600;
	QCam.ShowImgX = 800;
	QCam.ShowImgY = 600;
	QCam.ShowImgX_Start = 0;
	QCam.ShowImgY_Start = 0;

	InitQHY5LIIRegs();
        QCam5LII.QHY5L_PLL_Ratio = (int)setQHY5LREG_PLL(2);

	int xstart = 4 + (1280 - 800) / 2; ;
	int ystart = 4 + (960 - 600) / 2; ;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 626); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1388); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER
}


void initQHY5LII_VGA(void)
{
	int x = 640;
	int y = 480;
	
	QCam.ImgX = 640;
	QCam.ImgY = 480;
	QCam.ShowImgX = 640;
	QCam.ShowImgY = 480;
	QCam.ShowImgX_Start = 0;
	QCam.ShowImgY_Start = 0;

        InitQHY5LIIRegs();
	QCam5LII.QHY5L_PLL_Ratio = (int)setQHY5LREG_PLL(1);

	int xstart = 4 + (1280 - 640) / 2; ;
	int ystart = 4 + (960 - 480) / 2; ;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 506); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1388); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER
}

void initQHY5LII_QVGA(void) 
{
	int x = 320;
	int y = 240;

	QCam.ImgX = 320;
	QCam.ImgY = 240;
	QCam.ShowImgX = 320;
	QCam.ShowImgY = 240;
	QCam.ShowImgX_Start = 0;
	QCam.ShowImgY_Start = 0;

	InitQHY5LIIRegs();
	QCam5LII.QHY5L_PLL_Ratio = (int)setQHY5LREG_PLL(1);

	int xstart = 4 + (1280 - 320) / 2; ;
	int ystart = 4 + (960 - 320) / 2; ;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 266); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1388); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER
}

void CorrectQHY5LIIWH(int *w,int *h)
{
    if(*w <= 320 && *h <=240)
	{
		initQHY5LII_QVGA();
		*w = 320;
		*h = 240;
	}
	else if(*w <= 640 && *h <= 480)
	{
		initQHY5LII_VGA();
		*w = 640;
		*h = 480;
	}
	else if(*w <= 800 && *h <= 600)
	{
		initQHY5LII_SVGA();
		*w = 800;
		*h = 600;
	}
	else if(*w <= 1024 && *h <= 768)
	{
		initQHY5LII_XGA();
		*w = 1024;
		*h = 768;
	}
	else
	{	
		initQHY5LII_1280X960();
		*w = 1280;
		*h = 960;
	}
}

void SetExposureTime_QHY5LII(unsigned long i) 
{

	if(i >= 1200000)
	{
		QCam5LII.CheckBoxQHY5LIILoneExpMode = true;
	}
	else 
	{
		QCam5LII.CheckBoxQHY5LIILoneExpMode = false;
	}
	// 需要输入的参数: CMOSCLK
	double CMOSCLK;

	if (QCam.transferspeed == 1)
		CMOSCLK = 48;
	else
		CMOSCLK = 24;

	double pixelPeriod;
	pixelPeriod = 1 / (CMOSCLK * QCam5LII.QHY5L_PLL_Ratio); // unit: us

	double RowTime;
	double ExpTime;
	unsigned short REG300C, REG3012;

	double MaxShortExpTime;

	if (!QCam5LII.CheckBoxQHY5LIILoneExpMode) 
	{
		if (i > 2 * 1000 * 1000)
			i = 2 * 1000 * 1000;
	}

	REG300C = I2CTwoRead(0x300C);

	RowTime = REG300C * pixelPeriod;

	MaxShortExpTime = 65000 * RowTime;

	ExpTime = i;

	unsigned char buf[4];

	if (ExpTime > MaxShortExpTime) 
	{
		I2CTwoWrite(0x3012, 65000);

		ExpTime = ExpTime - MaxShortExpTime;

		buf[0] = 0;
		buf[1] = (((long)(ExpTime / 1000)) & (~0xff00ffff)) >> 16;
		buf[2] = ((long)((ExpTime / 1000))&~0xffff00ff) >> 8;
		buf[3] = ((long)((ExpTime / 1000))&~0xffffff00);
		libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc1, 0x00, 0x00, buf, 4,0);
		ExpTime = ExpTime + MaxShortExpTime;
		REG3012 = 65000;
		QCam5LII.longExpMode = true;
	}
	else 
	{

		if (QCam5LII.longExpMode == true) 
		{
			QCam5LII.longExpMode = false;
			// 切换过来以后,应该多执行一次
			buf[0] = 0;
			buf[1] = 0;
			buf[2] = 0;
			buf[3] = 0;
			libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc1, 0x00, 0x00, buf, 4,0);
			usleep(100);
			REG3012 = (unsigned short)(ExpTime / RowTime);
			if (REG3012 < 1)
				REG3012 = 1;
			I2CTwoWrite(0x3012, REG3012);
			ExpTime = REG3012 * RowTime;
			QCam5LII.longExpMode = false;

		}

		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = 0;
		libusb_control_transfer(QCam.ccd_handle,QHYCCD_REQUEST_WRITE, 0xc1, 0x00, 0x00, buf, 4,0);
		usleep(100);
		REG3012 = (unsigned short)(ExpTime / RowTime);
		if (REG3012 < 1)
			REG3012 = 1;
		I2CTwoWrite(0x3012, REG3012);
		ExpTime = REG3012 * RowTime;
		QCam5LII.longExpMode = false;
	}
}



void  W_I2C_MICRON_Address16_OneRegister(unsigned short adr, unsigned short dat)
{
    I2CTwoWrite(adr, dat);
}

unsigned short R_I2C_MICRON_Address16_OneRegister(unsigned short adr)
{
    return I2CTwoRead(adr);
}

double GetQHY5LIITemp(void){
	double slope;
	double T0;
	uint16_t sensed, calib1, calib2;
	W_I2C_MICRON_Address16_OneRegister(0x30B4, 0x0011);
	sensed = R_I2C_MICRON_Address16_OneRegister(0x30B2);
	calib1 = R_I2C_MICRON_Address16_OneRegister(0x30C6);
	calib2 = R_I2C_MICRON_Address16_OneRegister(0x30C8);
	W_I2C_MICRON_Address16_OneRegister(0x30B4, 0x0000);

	slope = (70.0 - 55.0)/(calib1 - calib2);
	T0 = (slope*calib1 - 70.0);
/*#ifdef QHY5L_DEBUG
	printf("calib1 = 0x%x\n", calib1);
	printf("calib2 = 0x%x\n", calib2);
	printf("sensed = 0x%x\n", sensed);
	printf("slope %f\n", slope);
	printf("T0 %f\n", T0);
	printf("Temperatura %f\n", slope * sensed - T0);
#endif*/
	return slope * sensed - T0;
}


void InitQHY5LIIRegs(void)
{
	// [720p, 25fps input27Mhz,output50Mhz, ]
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x0001); // RESET_REGISTER
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10D8); // RESET_REGISTER
	usleep(100);
	/////Linear sequencer
	W_I2C_MICRON_Address16_OneRegister(0x3088, 0x8000); // SEQ_CTRL_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0025); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2D26); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0828); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0D17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0926); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0526); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA728); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0725); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x8080); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2925); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0040); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2702); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2706); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1F17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x3626); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA617); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0326); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA417); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1F28); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0526); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0425); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2700); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x171D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0519); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2706); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1741); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2660); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x175A); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2317); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1122); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1741); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x9027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0026); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1828); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x002E); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2A28); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x081C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7003); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7004); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7005); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7009); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x170C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0014); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0014); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0314); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0314); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0414); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0414); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0514); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2405); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5001); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2550); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x502D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2608); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x280D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1709); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2600); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A7); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2807); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2580); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x8029); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0216); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1627); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0620); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1736); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A6); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A4); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x171F); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2620); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2804); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2520); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1D25); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1710); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1A17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0327); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0617); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0317); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4126); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x6017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xAE25); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0090); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2700); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2618); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2800); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2E2A); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2808); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1D05); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7009); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1720); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2024); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5002); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2550); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x502D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2608); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x280D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1709); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2600); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A7); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2807); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2580); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x8029); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0216); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1627); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0617); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x3626); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA617); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0326); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA417); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1F28); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0526); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0425); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2700); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x171D); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2021); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1710); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1B17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0327); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0617); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0317); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4126); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x6017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xAE25); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0090); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2700); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2618); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2800); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2E2A); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2808); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1E17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0A05); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7009); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2024); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x502B); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x302C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2C2C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2C00); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0225); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2D26); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0828); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0D17); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0926); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0526); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0xA728); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0725); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x8080); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2917); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0525); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0040); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2702); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1616); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2706); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1736); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A6); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x26A4); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x171F); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2805); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2620); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2804); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2520); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1E25); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2117); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1028); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x051B); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2706); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1703); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1747); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2660); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x17AE); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2500); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x9027); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0026); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1828); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x002E); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2A28); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x081E); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0831); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1440); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4014); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1410); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1034); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1014); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x4013); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1802); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7004); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7003); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1470); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x7017); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2002); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2002); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5004); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2004); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x1400); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x5022); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0314); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0020); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0314); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x0050); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2C2C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x3086, 0x2C2C); // SEQ_DATA_PORT
	W_I2C_MICRON_Address16_OneRegister(0x309E, 0x018A); // RESERVED_MFR_309E
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10D8); // RESET_REGISTER
	W_I2C_MICRON_Address16_OneRegister(0x3082, 0x0029); // OPERATION_MODE_CTRL
	W_I2C_MICRON_Address16_OneRegister(0x301E, 0x00C8); // DATA_PEDESTAL
	W_I2C_MICRON_Address16_OneRegister(0x3EDA, 0x0F03); // RESERVED_MFR_3EDA
	W_I2C_MICRON_Address16_OneRegister(0x3EDE, 0xC007); // RESERVED_MFR_3EDE
	W_I2C_MICRON_Address16_OneRegister(0x3ED8, 0x01EF); // RESERVED_MFR_3ED8
	W_I2C_MICRON_Address16_OneRegister(0x3EE2, 0xA46B); // RESERVED_MFR_3EE2
	W_I2C_MICRON_Address16_OneRegister(0x3EE0, 0x067D); // RESERVED_MFR_3EE0
	W_I2C_MICRON_Address16_OneRegister(0x3EDC, 0x0070); // RESERVED_MFR_3EDC
	W_I2C_MICRON_Address16_OneRegister(0x3044, 0x0404); // DARK_CONTROL
	W_I2C_MICRON_Address16_OneRegister(0x3EE6, 0x4303); // RESERVED_MFR_3EE6
	W_I2C_MICRON_Address16_OneRegister(0x3EE4, 0xD208); // DAC_LD_24_25
	W_I2C_MICRON_Address16_OneRegister(0x3ED6, 0x00BD); // RESERVED_MFR_3ED6
	W_I2C_MICRON_Address16_OneRegister(0x3EE6, 0x8303); // RESERVED_MFR_3EE6
	W_I2C_MICRON_Address16_OneRegister(0x30E4, 0x6372); // RESERVED_MFR_30E4
	W_I2C_MICRON_Address16_OneRegister(0x30E2, 0x7253); // RESERVED_MFR_30E2
	W_I2C_MICRON_Address16_OneRegister(0x30E0, 0x5470); // RESERVED_MFR_30E0
	W_I2C_MICRON_Address16_OneRegister(0x30E6, 0xC4CC); // RESERVED_MFR_30E6
	W_I2C_MICRON_Address16_OneRegister(0x30E8, 0x8050); // RESERVED_MFR_30E8
	usleep(200);
	W_I2C_MICRON_Address16_OneRegister(0x302A, 14); // DIV           14
	W_I2C_MICRON_Address16_OneRegister(0x302C, 1); // DIV
	W_I2C_MICRON_Address16_OneRegister(0x302E, 3); // DIV
	W_I2C_MICRON_Address16_OneRegister(0x3030, 65); // MULTI          44
	W_I2C_MICRON_Address16_OneRegister(0x3082, 0x0029);
	// OPERATION_MODE_CTRL
	W_I2C_MICRON_Address16_OneRegister(0x30B0, 0x5330);
	W_I2C_MICRON_Address16_OneRegister(0x305e, 0x00ff); // gain
	W_I2C_MICRON_Address16_OneRegister(0x3012, 0x0020);
	// coarse integration time
	W_I2C_MICRON_Address16_OneRegister(0x3064, 0x1802);

	int x = 1280;
	int y = 960;
	int xstart = 4;
	int ystart = 4;
	int xsize = x - 1;
	int ysize = y - 1;

	W_I2C_MICRON_Address16_OneRegister(0x3002, ystart); // y start
	W_I2C_MICRON_Address16_OneRegister(0x3004, xstart); // x start
	W_I2C_MICRON_Address16_OneRegister(0x3006, ystart + ysize); // y end
	W_I2C_MICRON_Address16_OneRegister(0x3008, xstart + xsize); // x end
	W_I2C_MICRON_Address16_OneRegister(0x300a, 990); // frame length
	W_I2C_MICRON_Address16_OneRegister(0x300c, 1650); // line  length
	W_I2C_MICRON_Address16_OneRegister(0x301A, 0x10DC); // RESET_REGISTER
}

#ifdef __cplusplus
}
#endif

