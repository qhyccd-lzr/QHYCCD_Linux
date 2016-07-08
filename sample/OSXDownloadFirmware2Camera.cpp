#include <stdio.h>
#include <libqhy/qhyccd.h>
#include <unistd.h>

int main(void)
{
    char path[1024] = "The Path to your firmware Directroy";
    /* 
    for example:
    firmware dirctory is /opt/phd2_qhy/tmp/firmware
    path = "/opt/phd2_qhy/tmp/"
    */   
    getcwd(path,1024);

    /*
     before you run this sample,please specify the path your firmware folder.the firmware folder must name "firmware".it is hardcode to my sdk.

     ex:
     OSXInitQHYCCDFirmware(./firmware/);
     */
    InitQHYCCDResource();

    /*
    remember you should put the .HEX and .IMG files to firmware folder before run this sample 
    */
    OSXInitQHYCCDFirmware(path);

    ReleaseQHYCCDResource();
}
