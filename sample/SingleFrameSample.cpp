
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libqhy/qhyccd.h>

int main(int argc,char *argv[])
{
    int num = 0;
    qhyccd_handle *camhandle = NULL;
    int ret = QHYCCD_ERROR;
    char id[32];
    int found = 0;
    int w,h,bpp,channels;
    unsigned char *ImgData;
    int camtime = 100000,camgain = 0,camoffset = 140,camspeed = 0,cambinx = 1,cambiny = 1;

    ret = InitQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Init SDK success!\n");
    }
    else
    {
        goto failure;
    }
    
    num = ScanQHYCCD();
    if(num > 0)
    {
        printf("Yes!Found QHYCCD,the num is %d \n",num);
    }
    else
    {
        printf("Not Found QHYCCD,please check the usblink or the power\n");
        goto failure;
    }

    for(int i = 0;i < num;i++)
    {
        ret = GetQHYCCDId(i,id);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("connected to the first camera from the list,id is %s\n",id);
            found = 1;
            break;
        }
    }

    if(found == 1)
    {
        camhandle = OpenQHYCCD(id);
        if(camhandle != NULL)
        {
            printf("Open QHYCCD success!\n");
        }
        else
        {
            printf("Open QHYCCD fail \n");
            goto failure;
        }

        ret = InitQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("Init QHYCCD success!\n");
        }
        else
        {
            printf("Init QHYCCD fail code:%d\n",ret);
            goto failure;
        }
     
        double chipw,chiph,pixelw,pixelh;
        ret = GetQHYCCDChipInfo(camhandle,&chipw,&chiph,&w,&h,&pixelw,&pixelh,&bpp);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("GetQHYCCDChipInfo success!\n");
            printf("CCD/CMOS chip information:\n");
            printf("Chip width %3f mm,Chip height %3f mm\n",chipw,chiph);
            printf("Chip pixel width %3f um,Chip pixel height %3f um\n",pixelw,pixelh);
            printf("Chip Max Resolution is %d x %d,depth is %d\n",w,h,bpp);
        }
        else
        {
            printf("GetQHYCCDChipInfo fail\n");
            goto failure;
        }
        
        
        ret = SetQHYCCDResolution(camhandle,0,0,w,h);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDResolution success!\n");
        }
        else
        {
            printf("SetQHYCCDResolution fail\n");
            goto failure;
        }
        
        ret = SetQHYCCDBinMode(camhandle,cambinx,cambiny);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("SetQHYCCDBinMode success!\n");
        }
        else
        {
            printf("SetQHYCCDBinMode fail\n");
            goto failure;
        }
        
        ret = ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("ExpQHYCCDSingleFrame success!\n");
        }
        else
        {
            printf("ExpQHYCCDSingleFrame fail\n");
            goto failure;
        }
        
        int length = GetQHYCCDMemLength(camhandle);
        
        if(length > 0)
        {
            ImgData = (unsigned char *)malloc(length);
            memset(ImgData,0,length);
        }
        else
        {
            printf("Get the min memory space length failure \n");
            goto failure;
        }

        ret = GetQHYCCDSingleFrame(camhandle,&w,&h,&bpp,&channels,ImgData);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("GetQHYCCDSingleFrame succeess! \n");
            
            //to do anything you like:

            delete(ImgData);       
        }
        else
        {
            printf("GetQHYCCDSingleFrame fail:%d\n",ret);
        }
    }
    else
    {
        printf("The camera is not QHYCCD or other error \n");
        goto failure;
    }
    if(camhandle)
    {
        ret = CancelQHYCCDExposingAndReadout(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("CancelQHYCCDExposingAndReadout success!\n");
        }
        else
        {
            printf("CancelQHYCCDExposingAndReadout fail\n");
            goto failure;
        }
        
        ret = CloseQHYCCD(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("Close QHYCCD success!\n");
        }
        else
        {
            goto failure;
        }
    }

    ret = ReleaseQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Rlease SDK Resource  success!\n");
    }
    else
    {
        goto failure;
    }

    return 0;

failure:
    printf("some fatal error happened\n");
    return 1;
}
