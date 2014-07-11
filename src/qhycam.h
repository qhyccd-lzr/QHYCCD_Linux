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

#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <libusb-1.0/libusb.h>

#ifndef QHYCAMODEF
#define QHYCAMODEF

typedef libusb_device                qhyccd_device;
typedef libusb_device_handle         qhyccd_handle;


#define QHYCCD_REQUEST_READ  0xC0
#define QHYCCD_REQUEST_WRITE 0x40

#define QHYCCD_INTERRUPT_READ_ENDPOINT  0x81
#define QHYCCD_INTERRUPT_WRITE_ENDPOINT 0x01
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

class QHYCAM
{
public:
	QHYCAM(){};
	~QHYCAM(){};
        int openCamera(libusb_device *d,libusb_device_handle **h);

        void closeCamera(libusb_device_handle *h);

        void sendForceStop(qhyccd_handle *handle){};

        int  sendInterrupt(qhyccd_handle *handle,unsigned char length,unsigned char *data);      

	int  vendTXD(qhyccd_handle *dev_handle, uint8_t req, unsigned char* data, uint16_t length);

	int  vendRXD(qhyccd_handle *dev_handle, uint8_t req, unsigned char* data, uint16_t length);

        int  iTXD(qhyccd_handle *dev_handle,unsigned char *data, int length);

        int  iRXD(qhyccd_handle *dev_handle,unsigned char *data, int length);
	
        int  vendTXD_Ex(qhyccd_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length);
	
        int  vendRXD_Ex(qhyccd_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length);
        
        int  readUSB2B(qhyccd_handle *dev_handle, unsigned char *data,int p_size, int p_num, int* pos);

        int  readUSB2BForQHY5IISeries(qhyccd_handle *dev_handle, unsigned char *data,int sizetoread,int exptime);

	int  readUSB2(qhyccd_handle *dev_handle, unsigned char *data, int p_size, int p_num);

	int  readUSB2_OnePackage3(qhyccd_handle *dev_handle, unsigned char *data, int length);

	int beginVideo(qhyccd_handle *handle);        

        int sendRegisterQHYCCDOld(qhyccd_handle *handle,
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber);

        int sendRegisterQHYCCDNew(qhyccd_handle *handle, 
                  CCDREG reg, int P_Size, int *Total_P, int *PatchNumber);
                  
        int setDC201FromInterrupt(qhyccd_handle *handle,unsigned char PWM,unsigned char FAN);

        signed short getDC201FromInterrupt(qhyccd_handle *handle);

        unsigned char getFromInterrupt(qhyccd_handle *handle,unsigned char length,unsigned char *data);

        double GetCCDTemp(qhyccd_handle *handle);

        double RToDegree(double R);

        double mVToDegree(double V);

        double DegreeTomV(double degree);

        double DegreeToR(double degree);

	int I2CTwoWrite(qhyccd_handle *handle,uint16_t addr,unsigned short value);

	unsigned short I2CTwoRead(qhyccd_handle *handle,uint16_t addr);

	unsigned char MSB(unsigned short i);

	unsigned char LSB(unsigned short i);

	void SWIFT_MSBLSB(unsigned char * Data, int x, int y);

        CCDREG ccdreg;

        /* usb transfer endpoint */
        int usbep;
        /* usb transfer package size at onece */
        int psize;
        /* the number of usb transfer packages */
        int totalp;
        /* patch for image*/
        int patchnumber;

};

#endif
