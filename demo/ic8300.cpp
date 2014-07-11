#include <stdio.h>
#include <libqhyccd/qhyccd.h>
#include <libusb-1.0/libusb.h>

unsigned char Imgdata[3584*2574*3];

int main(void)
{
    qhyccd_handle *camhandle;
    char camid[32];
    int ret;
    int num;
    int i;
    int w,h,bpp,channels;
//    ret = InitQHYCCDResource();
//    printf("InitSource %d\n",ret);

    libusb_init(NULL);
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(NULL,0x1618,0x8310);
    
    if(handle)
    {
        libusb_kernel_driver_active(handle,0);
        libusb_claim_interface(handle,0);
        SetQHYCCDFirmware(handle,"IC8300.HEX",1);
    }
/*
    num = ScanQHYCCD();
    if(num > 0)
    {
        printf("connected devices %d\n",num);
        for(i = 0;i < num;i++)
        {
            ret = GetQHYCCDId(i,camid);
            printf("camid:%s\n",camid); 
            if(camid[0] == 'I' && camid[1] == 'C')
            {
                camhandle = OpenQHYCCD(camid);
                if(camhandle)
                {
                    printf("open cam success\n");
                    break;
                }
            }
        }
    }
    if(camhandle)
    {
        ret = InitQHYCCD(camhandle);
        printf("Init ret %d\n",ret);
        if(ret == QHYCCD_SUCCESS)
        {
            ExpQHYCCDSingleFrame(camhandle);
            GetQHYCCDSingleFrame(camhandle,&w,&h,&bpp,&channels,Imgdata);   
            printf("w:%d h:%d bpp:%d channels:%d\n",w,h,bpp,channels);
        }        
    }


    CloseQHYCCD(camhandle);
*/
    ret = ReleaseQHYCCDResource();
    return 0;
}
