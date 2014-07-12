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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <opencv/cv.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "qhybase.h"
#include "qhy5lii_c.h"
#include "ic8300.h"
#include "qhy21.h"
#include "qhy22.h"
#include "qhy8l.h"
#include "qhy10.h"
#include "qhy12.h"
#include "qhy9s.h"
#include "qhy8pro.h"
#include "qhy11.h"
#include "qhy23.h"
#include "img2p.h"
#include "qhy6.h"
#include "qhy5ii.h"
#include "qhy5lii_m.h"
#include "qhy8.h"

/* This is the maximum number of qhyccd cams. */
#define MAXDEVICES          (100)

#define FX2_CPUCS_ADDR		(0xE600)
#define FX2_INT_RAMSIZE		(0x4000)

#define VENDORCMD_TIMEOUT	(5000)
#define MAX_LINE_LENGTH		(512)
#define MAX_BYTES_PER_LINE	(256)
#define EEPROM_WRITE_SIZE	(1024)

#define ROUND_UP(n,v)		((((n) + ((v) - 1)) / (v)) * (v))
#define CHAR_TO_HEXVAL(c)	((((c) >= '0') && ((c) <= '9')) ? ((c) - '0') : ((((c) - 'A') & 0x0F) + 10))
#define GET_HEX_BYTE(char_p)	(((CHAR_TO_HEXVAL((char_p)[0])) << 4) | (CHAR_TO_HEXVAL((char_p)[1])))
#define GET_HEX_WORD(char_p)	(((CHAR_TO_HEXVAL((char_p)[0])) << 12) | ((CHAR_TO_HEXVAL((char_p)[1])) << 8) | \
((CHAR_TO_HEXVAL((char_p)[2])) << 4) | (CHAR_TO_HEXVAL((char_p)[3])))

/* List of supported programming targets */
typedef enum {
	FW_TARGET_NONE = 0,	// Invalid target
	FW_TARGET_RAM,		// Program firmware (hex) to RAM
	FW_TARGET_SM_I2C,	// Program VID and PID to Small I2C EEPROM
	FW_TARGET_LR_I2C	// Program VID and PID to Large I2C EEPROM
} fx2_fw_tgt_p;

static char fx2_vendax[][256] = {
	":0a0d3e0000010202030304040505",
	":10064d00E4F52CF52BF52AF529C203C200C202C2",
	":10065d0001120C6C7E0A7F008E0A8F0B75120A75",
	":10066d00131275080A75091C75100A75114A7514",
	":10067d000A751578EE54C07003020752752D0075",
	":10068d002E808E2F8F30C3749A9FFF740A9ECF24",
	":10069d0002CF3400FEE48F288E27F526F525F524",
	":1006ad00F523F522F521AF28AE27AD26AC25AB24",
	":1006bd00AA23A922A821C3120D035037E5302524",
	":1006cd00F582E52F3523F583E0FFE52E2524F582",
	":1006dd00E52D3523F583EFF0E4FAF9F8E5242401",
	":1006ed00F524EA3523F523E93522F522E83521F5",
	":1006fd002180B3852D0A852E0B74002480FF740A",
	":10070d0034FFFEC3E5139FF513E5129EF512C3E5",
	":10071d000D9FF50DE50C9EF50CC3E50F9FF50FE5",
	":10072d000E9EF50EC3E5099FF509E5089EF508C3",
	":10073d00E5119FF511E5109EF510C3E5159FF515",
	":10074d00E5149EF514D2E843D82090E668E04409",
	":10075d00F090E65CE0443DF0D2AF90E680E054F7",
	":10076d00F0538EF8C2031207FF30010512039AC2",
	":10077d00013003F2120D6450EDC203120C0D2000",
	":10078d001690E682E030E704E020E1EF90E682E0",
	":10079d0030E604E020E0E4120BB6120D6680C722",
	":0b0d330090E50DE030E402C322D322",
	":10039a0090E6B9E0700302048514700302052E24",
	":1003aa00FE70030205C424FB700302047F147003",
	":1003ba0002047914700302046D14700302047324",
	":1003ca00056003020639120D68400302064590E6",
	":1003da00BBE024FE603B14605624FD6016146040",
	":1003ea0024067075E50A90E6B3F0E50B90E6B4F0",
	":1003fa00020645120D33500FE51290E6B3F0E513",
	":10040a0090E6B4F002064590E6A0E04401F00206",
	":10041a0045E50C90E6B3F0E50D90E6B4F0020645",
	":10042a00E50E90E6B3F0E50F90E6B4F002064590",
	":10043a00E6BAE0FF120BE2AA06A9077B01EA494B",
	":10044a00600DEE90E6B3F0EF90E6B4F002064590",
	":10045a00E6A0E04401F002064590E6A0E04401F0",
	":10046a00020645120CF1020645120D5002064512",
	":10047a000D48020645120CDF020645120D6A4003",
	":10048a0002064590E6B8E0247F602B14603C2402",
	":10049a006003020524A200E433FF25E0FFA202E4",
	":1004aa00334F90E740F0E4A3F090E68AF090E68B",
	":1004ba007402F0020645E490E740F0A3F090E68A",
	":1004ca00F090E68B7402F002064590E6BCE0547E",
	":1004da00FF7E00E0D3948040067C007D0180047C",
	":1004ea00007D00EC4EFEED4F243EF582740D3EF5",
	":1004fa0083E493FF3395E0FEEF24A1FFEE34E68F",
	":10050a0082F583E0540190E740F0E4A3F090E68A",
	":10051a00F090E68B7402F002064590E6A0E04401",
	":10052a00F0020645120D6C400302064590E6B8E0",
	":10053a0024FE601D2402600302064590E6BAE0B4",
	":10054a000105C20002064590E6A0E04401F00206",
	":10055a004590E6BAE0705990E6BCE0547EFF7E00",
	":10056a00E0D3948040067C007D0180047C007D00",
	":10057a00EC4EFEED4F243EF582740D3EF583E493",
	":10058a00FF3395E0FEEF24A1FFEE34E68F82F583",
	":10059a00E054FEF090E6BCE05480FF131313541F",
	":1005aa00FFE0540F2F90E683F0E04420F0020645",
	":1005ba0090E6A0E04401F0020645120D6E507C90",
	":1005ca00E6B8E024FE60202402705B90E6BAE0B4",
	":1005da000104D200806590E6BAE06402605D90E6",
	":1005ea00A0E04401F0805490E6BCE0547EFF7E00",
	":1005fa00E0D3948040067C007D0180047C007D00",
	":10060a00EC4EFEED4F243EF582740D3EF583E493",
	":10061a00FF3395E0FEEF24A1FFEE34E68F82F583",
	":10062a00E04401F0801590E6A0E04401F0800C12",
	":10063a000080500790E6A0E04401F090E6A0E044",
	":03064a0080F022",
	":03003300020D60",
	":040d600053D8EF32",
	":100c6c00D200E4F51A90E678E05410FFC4540F44",
	":090c7c0050F51713E433F51922",
	":0107ff0022",
	":060d6400D322D322D322",
	":080d480090E6BAE0F518D322",
	":100cdf0090E740E518F0E490E68AF090E68B04F0",
	":020cef00D322",
	":080d500090E6BAE0F516D322",
	":100cf10090E740E516F0E490E68AF090E68B04F0",
	":020d0100D322",
	":060d6a00D322D322D322",
	":1000800090E6B9E0245EB40B0040030203989000",
	":100090009C75F003A4C58325F0C5837302019202",
	":1000a000019202010D0200BD0200D70200F30201",
	":1000b0003C02018C02011602012902016290E740",
	":1000c000E519F0E490E68AF090E68B04F090E6A0",
	":1000d000E04480F002039890E60AE090E740F0E4",
	":1000e00090E68AF090E68B04F090E6A0E04480F0",
	":1000f00002039890E740740FF0E490E68AF090E6",
	":100100008B04F090E6A0E04480F002039890E6BA",
	":10011000E0F51702039890E67AE054FEF0E490E6",
	":100120008AF090E68BF002039890E67AE04401F0",
	":10013000E490E68AF090E68BF002039890E74074",
	":1001400007F0E490E68AF090E68B04F090E6A0E0",
	":100150004480F07FE87E031207ADD204120B8702",
	":10016000039890E6B5E054FEF090E6BFE090E68A",
	":10017000F090E6BEE090E68BF090E6BBE090E6B3",
	":10018000F090E6BAE090E6B4F002039875190143",
	":10019000170190E6BAE0753100F532A3E0FEE4EE",
	":1001a000423190E6BEE0753300F534A3E0FEE4EE",
	":1001b000423390E6B8E064C06003020282E53445",
	":1001c00033700302039890E6A0E020E1F9C3E534",
	":1001d0009440E533940050088533358534368006",
	":1001e00075350075364090E6B9E0B4A335E4F537",
	":1001f000F538C3E5389536E53795355060E53225",
	":1002000038F582E5313537F583E0FF74402538F5",
	":1002100082E434E7F583EFF00538E53870020537",
	":1002200080D0E4F537F538C3E5389536E5379535",
	":10023000501874402538F582E434E7F58374CDF0",
	":100240000538E5387002053780DDAD367AE77940",
	":100250007EE77F40AB07AF32AE311208B8E490E6",
	":100260008AF090E68BE536F02532F532E5353531",
	":10027000F531C3E5349536F534E5339535F53302",
	":1002800001BD90E6B8E064406003020398E51A70",
	":10029000051209678F1AE53445337003020398E4",
	":1002a00090E68AF090E68BF090E6A0E020E1F990",
	":1002b000E68BE0753500F53690E6B9E0B4A338E4",
	":1002c000F537F538C3E5389536E5379535400302",
	":1002d000037C74402538F582E434E7F583E0FFE5",
	":1002e000322538F582E5313537F583EFF00538E5",
	":1002f000387002053780CDE4F537F538C3E53895",
	":1003000036E53795355075851A39E51A64016044",
	":10031000E5322538FFE5313537FEE51A24FFFDE4",
	":1003200034FF5EFEEF5D4E6010E5322538FFE51A",
	":10033000145FFFC3E51A9FF539C3E5369538FFE5",
	":10034000359537FEC3EF9539EE94005007C3E536",
	":100350009538F539E5322538FFE5313537FE7440",
	":100360002538F582E434E7AD82FCAB39120A9CE5",
	":10037000392538F538E43537F5378080E5362532",
	":10038000F532E5353531F531C3E5349536F534E5",
	":0a039000339535F533020296C322",
	":100c3200C0E0C083C08290E6B5E04401F0D20153",
	":0f0c420091EF90E65D7401F0D082D083D0E032",
	":100c9d00C0E0C083C0825391EF90E65D7404F0D0",
	":100cad0082D083D0E032C0E0C083C0825391EF90",
	":0c0cbd00E65D7402F0D082D083D0E032",
	":100b1900C0E0C083C08290E680E030E70E85080C",
	":100b290085090D85100E85110F800C85100C8511",
	":100b39000D85080E85090F5391EF90E65D7410F0",
	":070b4900D082D083D0E032",
	":100c8500C0E0C083C082D2035391EF90E65D7408",
	":080c9500F0D082D083D0E032",
	":100b5000C0E0C083C08290E680E030E70E85080C",
	":100b600085090D85100E85110F800C85100C8511",
	":100b70000D85080E85090F5391EF90E65D7420F0",
	":070b8000D082D083D0E032",
	":0109ff0032",
	":100d700032323232323232323232323232323232",
	":100d800032323232323232323232323232323232",
	":030d9000323232",
	":100a00001201000200000040B404041000000102",
	":100a100000010A06000200000040010009022E00",
	":100a200001010080320904000004FF0000000705",
	":100a300002020002000705040200020007058602",
	":100a40000002000705880200020009022E000101",
	":100a50000080320904000004FF00000007050202",
	":100a600040000007050402400000070586024000",
	":100a700000070588024000000403090410034300",
	":100a80007900700072006500730073000E034500",
	":0c0a90005A002D005500530042000000",
	":100bb60090E682E030E004E020E60B90E682E030",
	":100bc600E119E030E71590E680E04401F07F147E",
	":0c0bd600001207AD90E680E054FEF022",
	":100b870030040990E680E0440AF0800790E680E0",
	":100b97004408F07FDC7E051207AD90E65D74FFF0",
	":0f0ba70090E65FF05391EF90E680E054F7F022",
	":1007ad008E3A8F3B90E600E054187012E53B2401",
	":1007bd00FFE4353AC313F53AEF13F53B801590E6",
	":1007cd0000E05418FFBF100BE53B25E0F53BE53A",
	":1007dd0033F53AE53B153BAE3A7002153A4E6005",
	":0607ed00120C2180EE22",
	":100be200A907AE14AF158F828E83A3E064037017",
	":100bf200AD0119ED7001228F828E83E07C002FFD",
	":100c0200EC3EFEAF0580DFE4FEFF2290E682E044",
	":100c1200C0F090E681F043870100000000002274",
	":100c220000F58690FDA57C05A3E582458370F922",
	":03004300020800",
	":03005300020800",
	":10080000020C3200020CB300020C9D00020C8500",
	":10081000020B1900020B50000209FF00020D7000",
	":10082000020D7100020D7200020D7300020D7400",
	":10083000020D7500020D7600020D7700020D7800",
	":10084000020D7900020D7000020D7A00020D7B00",
	":10085000020D7C00020D7D00020D7E00020D7F00",
	":10086000020D8000020D7000020D7000020D7000",
	":10087000020D8100020D8200020D8300020D8400",
	":10088000020D8500020D8600020D8700020D8800",
	":10089000020D8900020D8A00020D8B00020D8C00",
	":1008a000020D8D00020D8E00020D8F00020D9000",
	":0808b000020D9100020D9200",
	":100a9c008E3C8F3D8C3E8D3F8B40C28743B28012",
	":100aac000D58120D24120CC95004D2048059E519",
	":100abc00600FE53C90E679F0120CC95004D20480",
	":100acc0046E53D90E679F0120CC95004D2048037",
	":100adc00E4F541E541C395405021053FE53FAE3E",
	":100aec007002053E14F5828E83E090E679F0120D",
	":100afc00145004D2048010054180D890E678E044",
	":0d0b0c0040F0120C51C20453B27FA20422",
	":0f0d240090E6787480F0E51725E090E679F022",
	":100c5100120D58120D24120D1490E678E04440F0",
	":0b0c6100120D5890E678E030E1E922",
	":080d580090E678E020E6F922",
	":1008b8008E3C8F3D8D3E8A3F8B40120D58120D24",
	":1008c800120CC9500122E519600CE53C90E679F0",
	":1008d800120CC9500122E53D90E679F0120CC950",
	":1008e800012290E6787480F0E51725E0440190E6",
	":1008f80079F0120D1450012290E679E0F541120D",
	":1009080014500122E4F541E53E14FFE541C39F50",
	":100918001C90E679E0FFE5402541F582E4353FF5",
	":1009280083EFF0120D14500122054180DA90E678",
	":100938007420F0120D1450012290E679E0FFE540",
	":100948002541F582E4353FF583EFF0120D145001",
	":0f0958002290E6787440F090E679E0F541C322",
	":100d140090E678E0FF30E0F8EF30E202D322C322",
	":100cc90090E678E0FF30E0F8EF30E202D322EF20",
	":060cd900E102D322C322",
	":10096700E51970037F01227A107B407D40E4FFFE",
	":100977001208B8E4F53A7400253AF582E43410F5",
	":1009870083E53AF0053AE53AB440EB7C107D007B",
	":1009970040E4FFFE120A9CE4F53AE53AF4FF7400",
	":1009a700253AF582E43410F583EFF0053AE53AB4",
	":1009b70040E87A107B007D40E4FFFE1208B89010",
	":1009c70000E0F53AE53A30E005753B018008633A",
	":1009d7003F053A853A3BE4F53AE53AC394405015",
	":1009e700AF3A7E007C107D40AB3B120A9CE53B25",
	":0809f7003AF53A80E4AF3B22",
	":030000000207F3",
	":0c07f300787FE4F6D8FD75814102064D",
	":100d0300EB9FF5F0EA9E42F0E99D42F0E89C45F0",
	":010d130022",
	":00000001FF"
};
#define FX2_VENDAX_SIZE		(sizeof (fx2_vendax) / (256 * sizeof(char)))

/*
  This is the length for camera ID
 */
#define ID_STR_LEN 0x20

typedef struct libusb_device           qhyccd_device;
typedef struct libusb_device_handle    qhyccd_handle;

struct cydev {
    qhyccd_device *dev;          /* as above ... */
    qhyccd_handle *handle;       /* as above ... */
    unsigned short vid;          /* Vendor ID */
    unsigned short pid;          /* Product ID */
    unsigned char is_open;       /* When device is opened, val = 1 */
    char id[64];
    QHYBASE *qcam;
};

struct VPD 
{
    unsigned short    vid;
    unsigned short    pid;
};

unsigned short camvid[MAXDEVICES] = 
{
	0x1618,0x1618,0x1618,0x1618,0x1618,0x1618,0x1618,0x1618,0x1618,
    0x1618,0x1618,0x1618,0x1618,0X1618
};

unsigned short campid[MAXDEVICES] =
{
	0x0921,0x8311,0x6741,0x6941,0x6005,0x1001,0x1201,0x8301,0x6003,
    0x1101,0x8141,0x2851,0x025a,0x6001
};

unsigned short fpid[MAXDEVICES] =
{
    0x0920,0x8310,0x6740,0x6940,0x6004,0x1000,0x1200,0x8300,0x6002,
    0x1100,0x8140,0x2850,0x0259,0x6000
};

/* Global vid pid struct */
static struct VPD vpd[MAXDEVICES];

/* Global var for include vid,pid,qhybase clase... */
static struct cydev cydev[MAXDEVICES];

/* Number of cams */		
static int numdev;

static libusb_device **list;


static int qhyccd_handle2index(qhyccd_handle *h)	
{
    int index = QHYCCD_ERROR_INDEX;
    int i;
    for ( i = 0; i < numdev; ++i ) 
    {
        if(h == cydev[i].handle)
        {
            index = i;
            break;
        }
    }
    return index;
}

static int ReleaseQHYCCDClass(int index)
{  
    if(cydev[index].qcam)
    {
        delete(cydev[index].qcam);
        return QHYCCD_SUCCESS;
    }
    return QHYCCD_ERROR;
}

int InitQHYCCDResource()
{
    int i = 0;
   
    /* init vid pid for vpd struct */
    for(i = 0;i < MAXDEVICES;i++)
    {
        vpd[i].vid = camvid[i];
        vpd[i].pid = campid[i];
    }

    libusb_init(NULL);
    return QHYCCD_SUCCESS;
}

int ReleaseQHYCCDResource()
{  
    int i = 0;
    int ret = QHYCCD_ERROR;
    for(;i < numdev;i++)
    {
        ret = ReleaseQHYCCDClass(i);
        if(ret != QHYCCD_SUCCESS)
        {
            break;
        }
    }
 
    libusb_exit(NULL);
    return ret;
}

static int cyusb_get_device_descriptor(qhyccd_handle *h, struct libusb_device_descriptor *desc)
{
    qhyccd_device *tdev = libusb_get_device(h);
    return ( libusb_get_device_descriptor(tdev,desc));
}

static int QHY5IISeriesMatch(qhyccd_handle *handle)
{
    /* qhy5ii series model */
    int model;
    /* color cam or mono cam */
    int color;
    int ret;
    /* mem for cam info */
    unsigned char data[16];

    if(handle)
    {
        ret = libusb_control_transfer(handle,0xC0,0xCA,0,0x10,data,0x10,2000);

        if(ret == 0x10)
        {
            model = data[0];
            color = data[1];
       
            if(model == 1)
            {
                return DEVICETYPE_QHY5II;
            }
            else if(model == 6 && color == 0)
            {
                return DEVICETYPE_QHY5LII_M;
            }
            else if(model == 6 && color == 1)
            {
                return DEVICETYPE_QHY5LII_C;
            }
        }
	return ret;
    }
    return QHYCCD_ERROR;
}

static int QHYCCDSeriesMatch(int index,qhyccd_handle *handle)
{
    int ret = QHYCCD_ERROR_NOTSUPPORT;

    switch(cydev[index].pid)
    {
        case 0x0921:
        {
            ret = QHY5IISeriesMatch(handle);
            break;
        }
        case 0x8311:
        {
            ret = DEVICETYPE_IC8300;
            break;
        }
        case 0x6741:
        {
            ret = DEVICETYPE_QHY21;
            break;
        }
        case 0x6941:
        {
            ret = DEVICETYPE_QHY22;
            break;
        }
        case 0x6005:
        {
            ret = DEVICETYPE_QHY8L;
            break;
        }
        case 0x1001:
        {
            ret = DEVICETYPE_QHY10;
            break;
        }
        case 0x1201:
        {
            ret = DEVICETYPE_QHY12;
            break;
        }
        case 0x8301:
        {
            ret = DEVICETYPE_QHY9S;
            break;
        }
        case 0x6003:
        {
            ret = DEVICETYPE_QHY8PRO;
            break;
        }
        case 0x1101:
        {
            ret = DEVICETYPE_QHY11;
            break;
        }
        case 0x8141:
        {
            ret = DEVICETYPE_QHY23;
            break;
        }
        case 0x2851:
        {
            ret = DEVICETYPE_IMG2P;
            break;
        }
        case 0x025a:
        {
            ret = DEVICETYPE_QHY6;
            break;
        }
        case 0x6001:
        {
            ret = DEVICETYPE_QHY8;
            break;
        }
        default:
        {
            fprintf(stderr,"current pid is 0x%x\n",cydev[index].pid);
            ret = DEVICETYPE_UNKNOW;
        }
        break;
    }

    return ret;
}

static int DeviceIsQHYCCD(int index,qhyccd_device *d)
{
    int i;
    int found = 0;
    struct libusb_device_descriptor desc;
    int vid;
    int pid;

    libusb_get_device_descriptor(d, &desc);
    vid = desc.idProduct;
    pid = desc.idProduct;

    for ( i = 0; i < MAXDEVICES; ++i ) 
    {
        if ( (vpd[i].vid == desc.idVendor) && (vpd[i].pid == desc.idProduct) ) 
	{
            cydev[index].vid = desc.idVendor;
            cydev[index].pid = desc.idProduct;
            found = 1;
	    break;
        }
    }
    return found;
}

static int GetIdFromCam(qhyccd_handle *handle,char *id)
{
    int model,color,i;
    int ret = QHYCCD_ERROR;
    unsigned char data[ID_STR_LEN];
    char str[17];
    
    /* init the str with '\0',for some old camera */
    memset(str,'\0',17);

    if(handle)
    {
        ret = libusb_control_transfer(handle,0xC0, 0xca,0x10,0x10,data,0x10,2000);

        if(ret == 0x10)
        {         
            for(i = 0;i < 16;i++)
                sprintf(str+i,"%x",data[i]);
            id[16] = '\0';
        }
        else
        {
            fprintf(stderr,"get info from camera failure\n");
        }
        strcat(id,str);
    }
    return ret;
}

int InitQHYCCDClass(int camtype,int index)
{
    int ret;
    
    /* just for some old camera*/
    memset(cydev[index].id,'\0',32);

    switch(camtype)
    {
        case DEVICETYPE_QHY5LII_C:
        {
            cydev[index].qcam = new QHY5LII_C();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY5LII-C-",10);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY5LII_M:
        {
            cydev[index].qcam = new QHY5LII_M();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY5LII-M-",10);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY5II:
        {
            cydev[index].qcam = new QHY5II();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY5II-M-",9);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_IC8300:
        {
            cydev[index].qcam = new IC8300();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"IC8300-M-",9);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY21:
        {
            cydev[index].qcam = new QHY21();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY21-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY22:
        {
            cydev[index].qcam = new QHY22();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY22-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY8L:
        {
            cydev[index].qcam = new QHY8L();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY8L-C-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY10:
        {
            cydev[index].qcam = new QHY10();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY10-C-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY12:
        {
            cydev[index].qcam = new QHY12();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY12-C-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY9S:
        {
            cydev[index].qcam = new QHY9S();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY9S-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY8PRO:
        {
            cydev[index].qcam = new QHY8PRO();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY8PRO-C-",10);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY11:
        {
            cydev[index].qcam = new QHY11();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY11-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY23:
        {
            cydev[index].qcam = new QHY23();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY23-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_IMG2P:
        {
            cydev[index].qcam = new IMG2P();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"IMG2P-M-",8);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY6:
        {
            cydev[index].qcam = new QHY6();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY6-M-",7);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        case DEVICETYPE_QHY8:
        {
            cydev[index].qcam = new QHY8();
            if(cydev[index].qcam != NULL)
            {
                memcpy(cydev[index].id,"QHY8-C-",7);
                ret = QHYCCD_SUCCESS;
            }
            else
            {
                ret = QHYCCD_ERROR_INITCLASS;
            }
            break;
        }
        default:
        {
            fprintf(stderr,"The camtype is not correct\n");
            ret = QHYCCD_ERROR_NOTSUPPORT;
        }
    }
    return ret;
}

int ScanQHYCCD(void)
{
    qhyccd_device *tdev;
    qhyccd_handle *handle = NULL;
    int found = 0;
    int i;
    int ret;
    int camret;
    int nid;

    numdev = libusb_get_device_list(NULL, &list);
    if ( numdev < 0 ) 
    {
        return QHYCCD_ERROR_NO_DEVICE;
    }
    nid = 0;

    for ( i = 0; i < numdev; ++i ) 
    {
        tdev = list[i];
        ret = DeviceIsQHYCCD(nid,tdev);
        if (ret > 0) 
        {
            cydev[nid].dev = tdev;
	        ret = libusb_open(tdev, &cydev[nid].handle);

	        if (ret)
	        {
                fprintf(stderr,"Open QHYCCD error\n");
                return QHYCCD_ERROR;
	        }

           handle = cydev[nid].handle;

           camret = QHYCCDSeriesMatch(nid,handle);
           if(camret == QHYCCD_ERROR_NOTSUPPORT)
           {
               fprintf(stderr,"SDK not support this camera now\n");
               libusb_close(handle);
               continue;
           }
         
           ret = InitQHYCCDClass(camret,nid);
           if(ret != QHYCCD_SUCCESS)
           {
               fprintf(stderr,"Init QHYCCD class error\n");
               libusb_close(handle);
               continue;
           }

	       GetIdFromCam(handle,cydev[nid].id);

	       libusb_close(handle);
           cydev[nid].handle = NULL;
	       cydev[nid].is_open = 0;
	       ++nid;
	    }
    }

    numdev = nid;
    return nid;
}

int GetQHYCCDId(int index,char *id)
{
    if(numdev > 0)
    {
        if(index < numdev)
        {   
            /* just copy from global var*/
            memcpy(id,cydev[index].id,ID_STR_LEN);
            return QHYCCD_SUCCESS;
        }
    }
    return QHYCCD_ERROR;
}

qhyccd_handle *OpenQHYCCD(char *id)
{
    int i;
    /* for return value */
    int ret;

    for(i = 0;i < numdev;i++)
    {
        ret = strcmp(cydev[i].id,id);

        if(!ret)
        {
            ret = cydev[i].qcam->ConnectCamera(cydev[i].dev,&cydev[i].handle);
               
            if(ret == QHYCCD_SUCCESS)
            { 
                cydev[i].is_open = 1;
                return cydev[i].handle;
            } 
        }
        else
        {
            fprintf(stderr,"lock is %s and key is %s\n,not match\n",cydev[i].id,id);
        }
    }
    return NULL;
}

int CloseQHYCCD(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        ret = cydev[index].qcam->DisConnectCamera(handle);
        if(ret == QHYCCD_SUCCESS)
        {
            cydev[index].is_open = 0;       
        }
    }
    return ret;
}

int InitQHYCCD(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->InitChipRegs(handle);
        }
    }
    return ret;
}

int IsQHYCCDControlAvailable(qhyccd_handle *handle,CONTROL_ID controlId)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->IsChipHasFunction(controlId);
        }
    }
    return ret;
}

int IsQHYCCDColor(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->IsColorCam();
        }
    }
    return ret;
}

int IsQHYCCDCool(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->IsCoolCam();
        }
    }
    return ret;
}

int SetQHYCCDParam(qhyccd_handle *handle,CONTROL_ID controlId, double value)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            switch(controlId)
            {
                case CONTROL_WBR:
	        { 
                    ret = cydev[index].qcam->SetChipWBRed(handle,value);
		    break;
	        }
	        case CONTROL_WBG:
	        {
                    ret = cydev[index].qcam->SetChipWBGreen(handle,value);
		    break;
	        }
                case CONTROL_WBB:
                {
                    ret = cydev[index].qcam->SetChipWBBlue(handle,value);
                    break;
	        }
	        case CONTROL_EXPOSURE:
	        {
		    ret = cydev[index].qcam->SetChipExposeTime(handle,value);
		    break;
	        }
	        case CONTROL_GAIN:
	        {
		    ret = cydev[index].qcam->SetChipGain(handle,value);
		    break;
	        }
	        case CONTROL_OFFSET:
	        {
		    ret = cydev[index].qcam->SetChipOffset(handle,value);
		    break;
	        }
	        case CONTROL_SPEED:
	        {
                    ret = cydev[index].qcam->SetChipSpeed(handle,value);
                    break;
                }
                case CONTROL_USBTRAFFIC:
                {
                    ret = cydev[index].qcam->SetChipUSBTraffic(handle,value);
                    break;
                }  
                case CONTROL_TRANSFERBIT:
                {  
                    ret = cydev[index].qcam->SetChipBitsMode(handle,value);
                    break;
                }
                case CONTROL_ROWNOISERE:
                {
                    ret = cydev[index].qcam->DeChipRowNoise(handle,value);
                    break;
                }
                case CONTROL_MANULPWM:
                {
                    ret = cydev[index].qcam->SetChipCoolPWM(handle,value);
                    break;
                }
                default:
                {
                    ret = QHYCCD_ERROR_NOTSUPPORT;
                }
            }
        }
    }
    return ret;
}

double GetQHYCCDParam(qhyccd_handle *handle,CONTROL_ID controlId)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            switch(controlId)
            {
                case CONTROL_WBR:
                {
                    ret = cydev[index].qcam->GetChipWBRed();
                    break;
                }
                case CONTROL_WBG:
                {
                    ret = cydev[index].qcam->GetChipWBGreen();
                    break;
                }
                case CONTROL_WBB:
                {
                    ret = cydev[index].qcam->GetChipWBBlue();
                    break;
                }
                case CONTROL_EXPOSURE:
                {
                    ret = cydev[index].qcam->GetChipExposeTime();
                    break;
                }
                case CONTROL_GAIN:
                {
                    ret = cydev[index].qcam->GetChipGain();
                    break;
                }
                case CONTROL_OFFSET:
                {
                    ret = cydev[index].qcam->GetChipOffset();
                    break;
                }
                case CONTROL_SPEED:
                {
                    ret = cydev[index].qcam->GetChipSpeed();
                    break;
                }
                case CONTROL_USBTRAFFIC:
                {
                    ret = cydev[index].qcam->GetChipUSBTraffic();
                    break;
                } 
                case CONTROL_TRANSFERBIT:
                {
                    ret = cydev[index].qcam->GetChipBitsMode();
                    break;
                }
                case CONTROL_CURTEMP:
                {
                    ret = cydev[index].qcam->GetChipCoolTemp(handle);
                    break;
                }
                case CONTROL_CURPWM:
                {
                    ret = cydev[index].qcam->GetChipCoolPWM();
                    break;
                }
                default:
                {
                    ret = QHYCCD_ERROR_NOTSUPPORT;
                }
            }
        }
    }
    return ret;
}

int GetQHYCCDParamMinMaxStep(qhyccd_handle *handle,CONTROL_ID controlId,double *min,double *max,double *step)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->GetControlMinMaxStepValue(controlId,min,max,step);
        }
    }
    return ret;
}

int SetQHYCCDResolution(qhyccd_handle *handle,int width,int height)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->SetChipResolution(handle,width,height);
        }
    }
    return ret;
}

int GetQHYCCDMemLength(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->GetChipMemoryLength();
        }
    }
    return ret;
}

int ExpQHYCCDSingleFrame(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->BeginSingleExposure(handle);
        }
    }
    return ret;
}

int GetQHYCCDSingleFrame(qhyccd_handle *handle,int *w,int *h,int *bpp,int *channels,unsigned char *imgdata)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->GetSingleFrame(handle,w,h,bpp,channels,imgdata);
        }
    }
    return ret;
}

int StopQHYCCDExpSingle(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->StopSingleExposure(handle);
        }
    }
    return ret;
}

int BeginQHYCCDLive(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->BeginLiveExposure(handle);
        }
    }
    return ret;
}

int GetQHYCCDLiveFrame(qhyccd_handle *handle,int *w,int *h,int *bpp,int *channels,unsigned char *imgdata)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->GetLiveFrame(handle,w,h,bpp,channels,imgdata);
        }
    }
    return ret;
}

int StopQHYCCDLive(qhyccd_handle *handle)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->StopLiveExposure(handle);
        }
    }
    return ret;
}

int SetQHYCCDBinMode(qhyccd_handle *handle,int wbin,int hbin)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->SetChipBinMode(handle,wbin,hbin);
        }
    }
    return ret;
}

int SetQHYCCDBitsMode(qhyccd_handle *handle,int bits)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->SetChipBitsMode(handle,bits);
        }
    }
    return ret;
}

int ControlQHYCCDTemp(qhyccd_handle *handle,double targettemp)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->AutoTempControl(handle,targettemp);
        }
    }
    return ret;

}

int ControlQHYCCDGuide(qhyccd_handle *handle,int direction,int duration)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->Send2GuiderPort(handle,direction,duration);
        }
    }
    return ret;
}

int ControlQHYCCDCFW(qhyccd_handle *handle,int pos)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(handle);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            ret = cydev[index].qcam->Send2CFWPort(handle,pos);
        }
    }
    return ret;
}

int SetQHYCCDTrigerMode(qhyccd_handle *handle,int trigerMode)
{
    return QHYCCD_ERROR;
}

void Bits16ToBits8(qhyccd_handle *h,unsigned char *InputData16,unsigned char *OutputData8,int imageX,int imageY,unsigned short B,unsigned short W)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(h);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            cydev[index].qcam->Bit16To8_Stretch(InputData16,OutputData8,imageX,imageY,B,W);
        }
    }
}

void HistInfo192x130(qhyccd_handle *h,int x,int y,unsigned char *InBuf,unsigned char *OutBuf)
{
    int index = QHYCCD_ERROR_INDEX;
    int ret = QHYCCD_ERROR;

    index = qhyccd_handle2index(h);

    if(index != QHYCCD_ERROR_INDEX)
    {
        if(cydev[index].is_open)
        {
            cydev[index].qcam->HistInfo(x,y,InBuf,OutBuf);
        }
    }
}

/*
struct BINRESOLUTION *SupportBIN(int bin)
{
    if(mainClass->camOpen)
    {
        return mainClass->IsSupportBIN(bin);
    }
}
*/

static int qhyccd_reset(libusb_device_handle *h,int cpu_enable)
{
	unsigned char reset = (cpu_enable) ? 0 : 1;
	int r;
    
	r = libusb_control_transfer(h, 0x40, 0xA0, FX2_CPUCS_ADDR, 0x00, &reset, 0x01, VENDORCMD_TIMEOUT);
	if ( r != 1 ) {
		fprintf (stderr, "ERROR: FX2 reset command failed\n");
		return -1;
	}
    
	return 0;
}

static int qhyccd_load_vendax(libusb_device_handle *h)
{
	int r, j;
    unsigned int i;
	unsigned char *fw_p;
	unsigned char  databuf[MAX_BYTES_PER_LINE];
	unsigned char  num_bytes = 0;
	unsigned short address = 0;
    
	for ( i = 0; ((i < FX2_VENDAX_SIZE) && (fx2_vendax[i][8] == 0x30)); i++ ) {
		fw_p = (unsigned char *)&fx2_vendax[i][1];
		num_bytes = GET_HEX_BYTE(fw_p);
		fw_p += 2;
		address   = GET_HEX_WORD(fw_p);
		fw_p += 6;
		for ( j = 0; j < num_bytes; j++ ) {
			databuf[j] = GET_HEX_BYTE(fw_p);
			fw_p += 2;
		}
        
		r = libusb_control_transfer(h, 0x40, 0xA0, address, 0x00, databuf, num_bytes, VENDORCMD_TIMEOUT);
		if ( r != num_bytes ) {
			fprintf (stderr, "Error in control_transfer\n");
			return -2;
		}
	}
    
	r = qhyccd_reset (h, 1);
	if ( r != 0 ) {
		fprintf (stderr, "Error: Failed to get FX2 out of reset\n");
		return -3;
	}
    
	return 0;
}

static int SetQHYCCDFirmware(libusb_device_handle *h,const char *filename,int extended)
{
	FILE *fp1 = NULL;
	int i, r, len = 0;
	char hexdata[MAX_LINE_LENGTH];
	char *fw_p;
	unsigned char databuf[MAX_BYTES_PER_LINE];
	unsigned char num_bytes = 0;
	unsigned short address = 0;
    
	struct stat filbuf;
	int filsz;
    
	fp1 = fopen(filename, "rb");
	if ( !fp1 ) {
		fprintf(stderr, "Error: File does not exist\n");
		return -1;
	}
    
	stat(filename, &filbuf);
	filsz = filbuf.st_size;
    
	r = qhyccd_reset (h, 0);
	if ( r != 0 ) {
		fprintf (stderr, "Error: Failed to force FX2 into reset\n");
		fclose(fp1);
		return -1;
	}
	sleep(1);
    
	if ( extended ) {
		r = qhyccd_load_vendax(h);
		if ( r != 0 ) {
			fprintf (stderr, "Failed to download Vend_Ax firmware to aid programming\n");
			fclose(fp1);
			return -2;
		}
	}
    
LoadRam:
	while ( fgets(hexdata, MAX_LINE_LENGTH, fp1) != NULL ) {
		len += strlen(hexdata);
		if ( hexdata[8] == '1' )
			break;
        
		fw_p       = &hexdata[1];
		num_bytes  = GET_HEX_BYTE(fw_p);
		fw_p      += 2;
		address    = GET_HEX_WORD(fw_p);
		if (((extended) && (address >= FX2_INT_RAMSIZE)) || ((!extended) && (address < FX2_INT_RAMSIZE))) {
			fw_p += 6;
			for (i = 0; i < num_bytes; i++) {
				databuf[i] = GET_HEX_BYTE(fw_p);
				fw_p += 2;
			}
            
			r = libusb_control_transfer(h, 0x40, ((extended) ? 0xA3 : 0xA0), address, 0x00, databuf, num_bytes, VENDORCMD_TIMEOUT);
			if ( r != num_bytes ) {
				fprintf (stderr, "Vendor write to RAM failed\n");
				fclose(fp1);
				return -3;
			}
		}
	}
    
	if ( extended ) {
		/* All data has been loaded on external RAM. Now halt the CPU and load the internal RAM. */
		r = qhyccd_reset(h, 0);
		if ( r != 0 ) {
			fprintf(stderr, "Error: Failed to halt FX2 CPU\n");
			fclose(fp1);
			return -4;
		}
        
		extended = 0;
		fseek(fp1, 0, SEEK_SET);
		sleep(1);
		goto LoadRam;
	}
    
	fclose(fp1);
    
	/* Now release CPU from reset. */
	r = qhyccd_reset(h, 1);
	if ( r != 0 ) {
		fprintf(stderr, "Error: Failed to release FX2 from reset\n");
		return -5;
	}
    
	return 0;
}

int OSXInitQHYCCDFiramware()
{
    int ret = QHYCCD_ERROR;
    int i;
    libusb_device_handle *h;
    
    libusb_init(NULL);
    
    for(i = 0;i < 12;i++)
    {
        h = libusb_open_device_with_vid_pid(NULL,camvid[i],fpid[i]);
    
        if(h)
        {
            libusb_kernel_driver_active(h,0);
            libusb_claim_interface(h,0);
            switch(fpid[i])
            {
                case 0x0920:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY5II.HEX",1);
                    break;
                }
                case 0x8310:
                {
                    SetQHYCCDFirmware(h,"firmware/IC8300.HEX",1);
                    break;
                }
                case 0x6740:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY21.HEX",1);
                    break;
                }
                case 0x6940:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY22.HEX",1);
                    break;
                }
                case 0x6004:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY8L.HEX",1);
                    break;
                }
                case 0x1000:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY10.HEX",1);
                    break;
                }
                case 0x1200:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY12.HEX",1);
                    break;
                }
                case 0x8300:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY9S.HEX",1);
                    break;
                }
                case 0x1100:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY11.HEX",1);
                    break;
                }
                case 0x8140:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY23.HEX",1);
                    break;
                }
                case 0x2850:
                {
                    SetQHYCCDFirmware(h,"firmware/IMG2P.HEX",1);
                    break;
                }
                case 0x0259:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY6.HEX",1);
                    break;
                }
                case 0x6000:
                {
                    SetQHYCCDFirmware(h,"firmware/QHY8.HEX",1);
                    break;
                }
            }

            libusb_close(h);
            ret = QHYCCD_SUCCESS;
        }
    }
    
    libusb_exit(NULL);
    return ret;
}
