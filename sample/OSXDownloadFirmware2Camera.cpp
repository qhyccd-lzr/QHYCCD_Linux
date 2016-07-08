#include <stdio.h>
#include <libqhy/qhyccd.h>
#include <unistd.h>

int main(void)
{
    char path[1024] = "The Path to your firmware Directroy";
    /* 
    for example:
    firmware dirctory path is /opt/phd2_qhy/tmp/firmware
    path = "/opt/phd2_qhy/tmp/";
    */   

    InitQHYCCDResource();

    /*
    remember you should put the .HEX and .IMG files to firmware folder before run this sample 
    */
    OSXInitQHYCCDFirmware(path);

    ReleaseQHYCCDResource();
}
