/*
 * qhyusb->c
 *
 * libqhyusb program
 * 
 * Copyright (C) 2013 QHYCCD Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "qhyccd.h"
#include "common.h"

#if 0
#ifdef __cplusplus
extern "C"
{
#endif
#endif
QUsb *qhyusb;

qhyccd_device_model supported_models[] =
{
        {QHYCCD_IMG2S, (char *)"IMG2S", 0x1618, 0xA285},
        {QHYCCD_IMG50, (char *)"IMG50", 0x1618, 0x6669},
        {QHYCCD_QHY2E, (char *)"QHY2E", 0x1618, 0x2859},
        {QHYCCD_QHY5II,(char *)"QHY5II", 0x1618, 0x0921},
        {QHYCCD_QHY6,  (char *)"QHY6", 0x1618, 0x025A},
        {QHYCCD_QHY7,  (char *)"QHY7", 0x1618, 0x4023},
        {QHYCCD_QHY8M, (char *)"QHY8M", 0x1618, 0x6007},
        {QHYCCD_QHY9L, (char *)"QHY9L", 0x1618, 0x8311},
        {QHYCCD_QHY11, (char *)"QHY11", 0x1618, 0x1111},
        {QHYCCD_QHY16, (char *)"QHY16", 0x1618, 0x1601},
        {QHYCCD_QHY20, (char *)"QHY20", 0x1618, 0x8051},
        {QHYCCD_QHY21, (char *)"QHY21", 0x1618, 0x6741},
        {QHYCCD_QHY22, (char *)"QHY22", 0x1618, 0x6941},
        {QHYCCD_QHY9,  (char *)"QHY9",  0x1618, 0x8301},
        {0, NULL, 0, 0}
};


int QUsb::qhyccd_init(void)
{
        int ret;
        ret = libusb_init(&(qhyusb->ctx));
        return ret;
}

ssize_t QUsb::qhyccd_get_device_list(qhyccd_device ***list)
{
        ssize_t length = 0;
        libusb_device **usb_devices;
        ssize_t usb_devices_num;
        ssize_t i;

        *list = (qhyccd_device**)malloc(sizeof(qhyccd_device*));
        (*list)[length] = NULL;
        
        usb_devices_num = libusb_get_device_list(qhyusb->ctx, &usb_devices);
        
        if (usb_devices_num < 0) {
                return -1;
        }

        for (i = 0; i < usb_devices_num; i++) {
                libusb_device *device = usb_devices[i];
                if (is_supported_device(device)) {
                        libusb_ref_device(device);
                        (*list)[length] = device;
                        length++;
                        *list = (qhyccd_device**)
                                realloc(*list, sizeof(qhyccd_device*) * (length + 1));
                        (*list)[length] = NULL;
                }
        }
        
        libusb_free_device_list(usb_devices, 1);

        return length;
}


int QUsb::is_supported_device(libusb_device *device)
{
        struct libusb_device_descriptor dev_desc;
        if (libusb_get_device_descriptor(device, &dev_desc) < 0) {
                return 0;
        }

        const qhyccd_device_model *try_supported_model = supported_models;
        while (try_supported_model->name != NULL) {
                if (dev_desc.idVendor == try_supported_model->vendor_id &&
                    dev_desc.idProduct == try_supported_model->product_id) {
                        return 1;
                }
                try_supported_model++;
        }

        return 0;
}


void QUsb::qhyccd_free_device_list(qhyccd_device **list)
{
        qhyccd_device **cur = list;
        while (*cur != NULL) {
                libusb_unref_device(*cur);
                cur++;
        }
        if (list) {
                free(list);
        }
}

void QUsb::qhyccd_exit(void)
{   
    libusb_exit(qhyusb->ctx);
}

qhyccd_device_model* QUsb::qhyccd_get_device_model(qhyccd_device *device)
{
        struct libusb_device_descriptor dev_desc;
        if (libusb_get_device_descriptor(device, &dev_desc) < 0) {
                return NULL;
        }

        qhyccd_device_model *try_supported_model = supported_models;
        while (try_supported_model->name != NULL) 
        {
                #ifdef QHYCCD_DEBUG 
                printf("model name %s\n",try_supported_model->name);
                #endif
                if (dev_desc.idVendor == try_supported_model->vendor_id &&
                    dev_desc.idProduct == try_supported_model->product_id) {
                        return try_supported_model;
                }
                try_supported_model++;
        }

        return NULL;
}


int QUsb::qhyccd_open(qhyccd_device *device,
                             qhyccd_device_handle **device_handle)
{
        return libusb_open(device, device_handle);
}


void QUsb::qhyccd_close(qhyccd_device_handle *dev_handle)
{
        libusb_close(dev_handle);
}


int QUsb::qhyccd_vTXD(qhyccd_device_handle *dev_handle, uint8_t req,
                unsigned char* data, uint16_t length)
{
        int ret;
        ret = libusb_control_transfer(dev_handle, QHYCCD_REQUEST_WRITE, req,
                                      0, 0, data, length, 0);
        return ret;
}


int QUsb::qhyccd_vRXD(qhyccd_device_handle *dev_handle, uint8_t req,
                unsigned char* data, uint16_t length)
{
        int ret;
        ret = libusb_control_transfer(dev_handle, QHYCCD_REQUEST_READ, req,
                                      0, 0, data, length, 0);
        return ret;
}

int QUsb::qhyccd_vTXD_Ex(qhyccd_device_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length)
{
        int ret;
        ret = libusb_control_transfer(dev_handle, QHYCCD_REQUEST_WRITE, req,
                                      value, index, data, length, 0);
        return ret;
}

int QUsb::qhyccd_vRXD_Ex(qhyccd_device_handle *dev_handle, uint8_t req,uint16_t value,uint16_t index,unsigned char* data, uint16_t length)
{
        int ret;
        ret = libusb_control_transfer(dev_handle, QHYCCD_REQUEST_READ, req,
                                      value, index, data, length, 0);
        return ret;
}

int QUsb::qhyccd_iTXD(qhyccd_device_handle *dev_handle,
                unsigned char *data, int length)
{
        int ret;

        int length_transfered;

        ret = libusb_bulk_transfer(dev_handle, QHYCCD_INTERRUPT_WRITE_ENDPOINT, 
                                        data, length, &length_transfered, 0);
        return ret;
}


int QUsb::qhyccd_iRXD(qhyccd_device_handle *dev_handle,
                unsigned char *data, int length)
{
        int ret;

        int length_transfered;

        ret = libusb_bulk_transfer(dev_handle, QHYCCD_INTERRUPT_READ_ENDPOINT, 
                                        data, length, &length_transfered, 0);
        return ret;
}


int QUsb::qhyccd_readUSB2B(qhyccd_device_handle *dev_handle, unsigned char *data,
                     int p_size, int p_num, int* pos)
{
        int ret;
        int length_transfered;
        int i;
        unsigned char *buf;
        buf = (unsigned char*)malloc(p_size);

        int dataEndpoint;
        
        switch(qhyusb->QCam.CAMERA)
        {
        	  case DEVICETYPE_QHY9L:
            case DEVICETYPE_QHY5II:
            case DEVICETYPE_QHY5LII:
            {
                dataEndpoint = 0x82;
            }
            break;
            case DEVICETYPE_QHY9:
            {
                dataEndpoint = 0x86;
            }
            break;
            default:
            {
                dataEndpoint = 0x86;
            }
        }

        for (i = 0; i < p_num; ++i) {
                ret = libusb_bulk_transfer(dev_handle,
                                           dataEndpoint,
                                           buf, p_size,
                                           &length_transfered, 0);
                if (ret < 0) {
                        free(buf);
                        return ret;
                }
                memcpy(data + i * p_size, buf, p_size);
                *pos = i;
        }

        ret = length_transfered;

        free(buf);
        return ret;
}


int QUsb::qhyccd_readUSB2(qhyccd_device_handle *dev_handle, unsigned char *data,
                    int p_size, int p_num)
{
        int ret;
        int length_transfered;

        ret = libusb_bulk_transfer(dev_handle, QHYCCD_DATA_READ_ENDPOINT,
                                   data, p_size * p_num,
                                   &length_transfered, 0);
        
        return ret;
}


int QUsb::qhyccd_readUSB2_OnePackage3(qhyccd_device_handle *dev_handle,
                                unsigned char *data, int length)
{
        int ret;
        int length_transfered;

        ret = libusb_bulk_transfer(dev_handle, QHYCCD_DATA_READ_ENDPOINT,
                                   data, length, &length_transfered, 0);
        return ret;
}

void QUsb::beginVideo(qhyccd_device_handle *handle)
{
        unsigned char buf[1];
        buf[0] = 100;

        qhyccd_vTXD(handle, 0xb3, buf, 1);
}

void QUsb::I2CTwoWrite(uint16_t addr,unsigned short value)
{
    unsigned char data[2];
    data[0] = MSB(value);
    data[1] = LSB(value);

    libusb_control_transfer(qhyusb->QCam.ccd_handle,QHYCCD_REQUEST_WRITE,0xbb,0,addr,data,2,0);
}

unsigned short QUsb::I2CTwoRead(uint16_t addr)
{
    unsigned char data[2];

    libusb_control_transfer(qhyusb->QCam.ccd_handle,QHYCCD_REQUEST_READ,0xb7,0,addr,data,2,0);

    return data[0] * 256 + data[1];
}

unsigned char QUsb::MSB(unsigned int i)
{
        unsigned int j;
        j=(i&~0x00ff)>>8;
        return j;
}

unsigned char QUsb::LSB(unsigned int i)
{
        unsigned int j;
        j=i&~0xff00;
        return j;
}

void QUsb::SWIFT_MSBLSB(unsigned char * Data, int x, int y)
{
    int i,j;
    unsigned char tempData;
    unsigned long s;

    s=0;

    for (j = 0; j < y; j++) 
    {
        for (i = 0; i < x; i++) 
        {
	   tempData=Data[s];
	   Data[s]=Data[s+1];
	   Data[s+1]=tempData;
	   s=s+2;
        }
    }
}
#if 0
#ifdef __cplusplus
}
#endif
#endif
