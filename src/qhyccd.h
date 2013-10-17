/*
 * qhyccd.h
 *
 * libqhyccd header file
 * 
 * Copyright (C) 2013 QHYCCD Inc.
 */

#ifndef __QHYCCD_H__
#define __QHYCCD_H__

#include <libusb-1.0/libusb.h>
#include <stdbool.h>

#define LIB_QHYCCD_VER 1

#define QHYCCD_DEVICE_LIST_SIZE 4    //最多连接的相机数量 

#if 0
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif



typedef libusb_device                qhyccd_device;
typedef libusb_device_handle         qhyccd_device_handle;

enum qhyccd_request_type {
        QHYCCD_REQUEST_READ = 0xC0,
        QHYCCD_REQUEST_WRITE = 0x40
};

enum qhyccd_endpoint_type {
        QHYCCD_INTERRUPT_READ_ENDPOINT = 0x81,
        QHYCCD_INTERRUPT_WRITE_ENDPOINT = 0x01,
        QHYCCD_DATA_READ_ENDPOINT = 0x82
};

enum qhyccd_model_id 
{
        QHYCCD_IMG2S = 1,
        QHYCCD_IMG50 = 2,
        QHYCCD_QHY2E = 3,
        QHYCCD_QHY5II = 4,
        QHYCCD_QHY6   = 5,
        QHYCCD_QHY7   = 6,
        QHYCCD_QHY8M  = 7,
        QHYCCD_QHY9L  = 8,
        QHYCCD_QHY11  = 9,
        QHYCCD_QHY16  = 10,
        QHYCCD_QHY20  = 11,
        QHYCCD_QHY21  = 12,
        QHYCCD_QHY22  = 13,
        QHYCCD_QHY9   = 14
};

typedef struct qhyccd_device_model {
        int model_id;
        char *name;
        uint16_t vendor_id;
        uint16_t product_id;
} qhyccd_device_model;


typedef struct QHYCCDPara
{
    int CAMERA;//相机型号
    int transferBit;//8:位传输模式 16:16位传输模式
    int bin;//软件上BIN模式，11:原始模式 22:2x2合并模式
    int camGain;//增益
    int wbblue;//设置白平衡用到
    int wbgreen;//设置白平衡用到
    int wbred;//设置白平衡用到
    int ImgX;
    int ImgY;
    int ShowImgX;
    int ShowImgY;
    int ShowImgX_Start;
    int ShowImgY_Start;
    int cameraW;//宽
    int cameraH;//高
    int pos;
    int brightness;
    int curbrightness;
    double camTime;//曝光时间(us)
    bool isColor;//相机是否彩色 true:彩色 false:黑白
    bool expModeChanged;
    unsigned char transferspeed;//传输速度0:低速 1:高速
    unsigned char camOffset;//传输速度0:低速 1:高速
    double contrast;
    double gama;
    qhyccd_device_handle *ccd_handle;
    qhyccd_device **device_list;
}QStruct;

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

class QUsb
{
public:
	int qhyccd_init(void);
	void qhyccd_exit(void);
	ssize_t qhyccd_get_device_list(qhyccd_device ***list);
	void qhyccd_free_device_list(qhyccd_device **list);
        int is_supported_device(libusb_device *device);
	qhyccd_device_model* qhyccd_get_device_model(qhyccd_device *device);
	int qhyccd_open(qhyccd_device *device, qhyccd_device_handle **device_handle);
	void qhyccd_close(qhyccd_device_handle *dev_handle);
	int qhyccd_vTXD(qhyccd_device_handle *dev_handle, uint8_t req, unsigned char* data, uint16_t length);
	int qhyccd_vRXD(qhyccd_device_handle *dev_handle, uint8_t req, unsigned char* data, uint16_t length);
	int qhyccd_vTXD_Ex(qhyccd_device_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length);
	int qhyccd_vRXD_Ex(qhyccd_device_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length);
	int qhyccd_iTXD(qhyccd_device_handle *dev_handle, unsigned char *data, int length);
	int qhyccd_iRXD(qhyccd_device_handle *dev_handle, unsigned char *data, int length);
	int qhyccd_readUSB2B(qhyccd_device_handle *dev_handle, unsigned char *data, int p_size, int p_num, int* pos);
	int qhyccd_readUSB2(qhyccd_device_handle *dev_handle, unsigned char *data, int p_size, int p_num);
	int qhyccd_readUSB2_OnePackage3(qhyccd_device_handle *dev_handle, unsigned char *data, int length);
	void beginVideo(qhyccd_device_handle *handle);
	void I2CTwoWrite(uint16_t addr,unsigned short value);
	unsigned short I2CTwoRead(uint16_t addr);
	unsigned char MSB(unsigned int i);
	unsigned char LSB(unsigned int i);
	void SWIFT_MSBLSB(unsigned char * Data, int x, int y);

        QUsb(void)
        {
   
        }
        ~QUsb(void){}

       QStruct QCam;
       CCDREG ccdreg;
       int liveabort;
private:
       libusb_context *ctx;
};


#if 0
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
#endif /* __QHYCCD_H__ */
