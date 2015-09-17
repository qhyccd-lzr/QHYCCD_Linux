#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libqhy/qhyccd.h>


int main(void)
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    char ch = '0';
    char currentpos[64];
    int checktimes = 0;

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
        
        
        if(camhandle)
        {
            int colorret = IsQHYCCDControlAvailable(camhandle,CAM_COLOR);
            if(colorret != QHYCCD_ERROR)
            {
                printf("This is a color camera\n");
                
                if(colorret == BAYER_GB)
                    printf("the bayer format is BAYER_GB\n");
                else if(colorret == BAYER_GR)
                    printf("the bayer format is BAYER_GR\n");
                else if(colorret == BAYER_BG)
                    printf("the bayer format is BAYER_BG\n");
                else if(colorret == BAYER_RG)
                    printf("the bayer format is BAYER_RG\n");
            }
            else
            {
                printf("This is a mono camera\n");
            }
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
