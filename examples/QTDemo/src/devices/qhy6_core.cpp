/*
 * qhy6_core.cpp
 *
 *  Created on: 23.11.2011
 *      Author: gm
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "utils.h"
#include "lusb.h"
#include "qhy6_core.h"


// disables hardware access for QHY6
//#define NO_QHY6

#define SENDREGS  0xB5
#define BUFWORD(b,p,W)  b[p] = ((unsigned char)(W>>8)) ; b[p+1] = ((unsigned char)(W&0xFF))


pthread_mutex_t qhy6_core_shared::m_mutex = PTHREAD_MUTEX_INITIALIZER;
libusb_device_handle *qhy6_core_shared::m_handle = NULL;
int qhy6_core_shared::m_init_cnt = 0;
int qhy6_core_shared::m_exposuretime = 0;


qhy6_core_shared::qhy6_core_shared( void )
{
	int ret = lusb::initialize();

	if( ret != 0 )
		log_e( "qhy6_core_shared::qhy6_core_shared(): Could not initialize libusb" );
}


qhy6_core_shared::~qhy6_core_shared( void )
{
	lusb::release();
}


int qhy6_core_shared::open_device( void )
{
 int res = EXIT_SUCCESS;

	pthread_mutex_lock( &m_mutex );

#ifdef NO_QHY6
	if( m_init_cnt == 0 && lusb::is_initialized() )
		log_i( "qhy6_core_shared::open_device(): Success" );
#else
	if( m_handle == NULL && m_init_cnt == 0 && lusb::is_initialized() )
	{
		if( (m_handle = locate_device(QHY6_VENDOR_ID, QHY6_PRODUCT_ID)) == NULL )
		{
			log_e( "qhy6_core_shared::open_device(): Could not open the QHY6 device" );
			res = EXIT_FAILURE;
		}
		else
			log_i( "qhy6_core_shared object successfully opened." );
	}
#endif
	if( res == EXIT_SUCCESS )
		m_init_cnt++;

	pthread_mutex_unlock( &m_mutex );

 return res;
}


void qhy6_core_shared::close_device( void )
{
 	pthread_mutex_lock( &m_mutex );

	if( m_init_cnt <= 0 )
	{
		log_e( "qhy6_core_shared::close_device(): Already closed or not opened yet" );
	}
	else
	{
		m_init_cnt--;
#ifdef NO_QHY6
		if( m_init_cnt == 0 )
			log_i( "qhy6_core_shared::close_device(): Success" );
#else
		if( m_handle && m_init_cnt == 0 )
		{
			libusb_close( m_handle );
			m_handle = NULL;
			log_i( "qhy6_core_shared::close_device(): Success" );
		}
#endif
	}

	pthread_mutex_unlock( &m_mutex );
}


libusb_device_handle *qhy6_core_shared::locate_device( unsigned int vid, unsigned int pid )
{
	libusb_device_handle *device_handle = NULL;

	device_handle = libusb_open_device_with_vid_pid( NULL, vid, pid );

	if( device_handle == NULL )
		return NULL;

	int open_status = libusb_set_configuration( device_handle, 1 );
	(void)open_status;

	open_status = libusb_claim_interface( device_handle, 0 );

	return device_handle;
}


int qhy6_core_shared::ctrl_msg( int request_type, int request, unsigned int value, unsigned int index, unsigned char *data, int len )
{
	int result;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return 0;
#endif

	assert( m_handle != NULL );

	result = libusb_control_transfer( m_handle, request_type, request, value, index, data, len, 1000 );
//	if( DBG_VERBOSITY )
//	{
//		for( int i = 0; i < len; i++ )
//			printf(" %02x", data[i]);
//		log_i("\n");
//	}

	return result < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


#ifdef QHY6_WITH_ST4

int qhy6_core_shared::start_exposure( unsigned int exposure )
{
	(void)exposure;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return 0;
#endif

	pthread_mutex_lock( &m_mutex );

	int ret = EXIT_SUCCESS;
	unsigned char REG[2];

	REG[0] = 0;
	REG[1] = 100;
	ret = ctrl_msg( 0x01, 0xB3, 0, 0, REG, sizeof(REG) );

	pthread_mutex_unlock( &m_mutex );

	return ret;
}

#else

int qhy6_core_shared::start_exposure( unsigned int exposure )
{
	(void)exposure;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return 0;
#endif

	pthread_mutex_lock( &m_mutex );

	int ret = EXIT_SUCCESS;
	unsigned char REG[2];

	REG[0] = 0;
	REG[1] = 100;
	ret = ctrl_msg( 0x42, 0xB3, 0, 0, REG, sizeof(REG) );

	pthread_mutex_unlock( &m_mutex );

	return ret;
}
#endif


#ifdef QHY6_WITH_ST4

int qhy6_core_shared::read_exposure( unsigned char *image, unsigned int image_size )
{
	int result = 0, ret = EXIT_SUCCESS;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return 0;
#endif

	int magic_number = 256;  // 256 - starange magic number
	int corrected_size = (int)image_size - magic_number;

	//pthread_mutex_lock( &m_mutex );

	assert( m_handle != NULL );

	log_i("Reading %d bytes...", corrected_size);

	int res = libusb_bulk_transfer( m_handle, 0x82, image, corrected_size, &result, /*m_exposuretime+*/2000 );

	if( res < 0 || result != corrected_size )
	{
		log_e( "Failed to read image. result = %d, Got: %d, expected %u", res, result, corrected_size );
		ret = EXIT_FAILURE;
	}
	else
	{
		log_i( "Done" );
	}

	//pthread_mutex_unlock( &m_mutex );

 return ret;
}

#else

int qhy6_core_shared::read_exposure( unsigned char *image, unsigned int image_size )
{
	int result, ret = EXIT_SUCCESS;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return 0;
#endif

	unsigned int magic_number = 0;  // 256 - starange magic number
	unsigned int corrected_size = image_size - magic_number;

	//pthread_mutex_lock( &m_mutex );

	assert( m_handle != NULL );

	if( DBG_VERBOSITY )
		log_i("Reading %d bytes...", corrected_size);

	libusb_bulk_transfer( m_handle, 0x82, image, corrected_size, &result, m_exposuretime+2000 );

	if( result == (int)corrected_size )
	{
		if( DBG_VERBOSITY )
			log_i( "Done" );
	}
	else
	{
		log_e("Failed to read image. Got: %d, expected %u", result, corrected_size);
		ret = EXIT_FAILURE;
	}

	//pthread_mutex_unlock( &m_mutex );

 return ret;
}

#endif

#ifdef QHY6_WITH_ST4

static unsigned char MSB( unsigned int i )
{
	unsigned int j;
	j = (i&~0x00ff) >> 8;
	return j;
}


static unsigned char LSB( unsigned int i )
{
	unsigned int j;
	j = i&~0xff00;
	return j;
}


int qhy6_core_shared::set_params( int exposuretime, int binn, int gain, int offset, int speed, int amp, int vbe, int *out_width, int *out_height, int *out_buffer_size )
{
	(void)vbe;
	unsigned char REG[64];
	int PatchNumber, time, Vbin, Hbin, antiamp;
	int ret = EXIT_SUCCESS;
	unsigned char time_H,time_M,time_L;
	int width = 0;
	int height = 0;

	int Total_P, P_Size;

	memset( REG , 0 , sizeof(REG) );
	m_exposuretime = exposuretime;
	time = (int)(exposuretime - (exposuretime / 100));

	antiamp = 0;
	switch( amp )
	{
	case 0:
		antiamp = 1;
		break;
	case 1:
		antiamp = 0;
		break;
	case 2:
		if( time > 550 )
			antiamp = 1;
		break;
	default:
		return EXIT_FAILURE;
	}

	switch( binn )
	{
	case 1:
		width  = QHY6_WIDTH_B1; height = QHY6_HEIGHT_B1; Vbin = binn; Hbin = binn; P_Size = 932*1024;
		break;
	case 2:
		width  = QHY6_WIDTH_B2; height = QHY6_HEIGHT_B2; Vbin = binn; Hbin = binn; P_Size = 234*1024;
		break;
	default:
		return EXIT_FAILURE;
	}

	int totalsize = width * 2 * height;

	if( out_width )
		*out_width = width;
	if( out_height )
		*out_height = height;
	if( out_buffer_size )
		*out_buffer_size = totalsize;

	if( fmod(totalsize, P_Size) != 0 )
	{
		Total_P = totalsize / P_Size + 1;
		PatchNumber = Total_P * P_Size - totalsize;
	}
	else
	{
		Total_P = totalsize / P_Size;
		PatchNumber = 0;
	}

	time_L = fmod( time, 256 );
	time_M = (time - time_L) / 256;
	time_H = (time - time_L - time_M * 256) / 65536;


	REG[0] = (int)((gain*63)/100);  				//Camera gain   range 0-63

	REG[1] = offset; 						//Offset : range 0-255 default is 120

	REG[2] = time_H;  					//unit is ms       24bit
	REG[3] = time_M;
	REG[4] = time_L;

	REG[5] = Hbin;						// Horizonal BINNING    0 = 1= No bin
	REG[6] = Vbin;						// Vertical Binning        0 =  1= No bin

	REG[7] = MSB( width );					// The readout X  Unit is pixel 16Bit
	REG[8] = LSB( width );

	REG[9] = MSB( height);					// The readout Y  unit is line 16Bit
	REG[10] = LSB( height);

	REG[11] = 0;						// SKIP_TO use for subframe    Skip lines on top 16Bit
	REG[12] = 0;

	REG[13] = 0;						// SKIP_BOTTOM use for subframe Skip lines on Bottom 16Bit
	REG[14] = 0;						// VerticalSize + SKIP_TOP +  SKIP_BOTTOM  should be the actual CCD Y size

	REG[15] = 0;						// LiveVideo no use for QHY8-9-11   16Bit set to 0
	REG[16] = 0;

	REG[17] = MSB( PatchNumber );				// PatchNumber 16Bit
	REG[18] = LSB( PatchNumber );

	REG[19] = 0;						// AnitInterlace no use for QHY8-9-11  16Bit set to 0
	REG[20] = 0;

	REG[22] = 0;						// MultiFieldBIN no use for QHY6-9  set to 0

	REG[29] = 0x0000;						// ClockADJ no use for QHY9-11  16Bit set to 0
	// [Qhy6Pro StrToInt(Form7->LableFineTone->Caption ); Investigation needed]
	REG[30] = 0;

	REG[32] = antiamp;					// 1: anti-amp light mode

	REG[33] = speed;						// 0: low speed     1: high speed

	REG[35] = 0; 						// TgateMode if set to 1 , the camera will exposure forever, till the ForceStop command coming
	REG[36] = 0;						// ShortExposure no use for QHY6-9 set to 0
	REG[37] = 0;						// VSUB no use for QHY8-9-11   set to 0
	REG[38] = 1;						// Unknown reg.CLAMP (I think "right" value is 1) but can be also 0.

	REG[42] = 0;						// TransferBIT no use for QHY6-8-9-11 set to 0

	REG[46] = 30;						// TopSkipNull unit is line.

	REG[47] = 0;						// TopSkipPix no use for QHY6-9-11 16Bit set to 0
	REG[48] = 0;

	REG[51] = 0;						// QHY9 0: programme control mechanical shutter automaticly   1: programme will not control shutter.
	REG[52] = 0;						// DownloadCloseTEC no use for QHY6-9   set to 0

	REG[53] = 0;						// Unknown: (reg.WindowHeater&~0xf0)*16+(reg.MotorHeating&~0xf0)


	REG[58] = 100;						// SDRAM_MAXSIZE no use for QHY6-8-9-11   set to 100
	REG[63] = 0;						// Unknown reg.Trig

#ifdef NO_QHY6
	if( m_handle == NULL )
		return EXIT_SUCCESS;
#endif

	pthread_mutex_lock( &m_mutex );

	ret = ctrl_msg( 0x01, SENDREGS, 0, 0, REG, sizeof(REG) );

	pthread_mutex_unlock( &m_mutex );

	return ret;
}
#else
int qhy6_core_shared::set_params( int exposuretime, int binn, int gain, int offset, int speed, int amp, int vbe, int *out_width, int *out_height, int *out_buffer_size )
{

	unsigned char RM =  0x04;
	unsigned char HM =  0x00;
	int H_SIZE, V_SIZE, Padding, ret, time;
	unsigned char Regs[24];
	int width = 0;
	int height = 0;

	memset( Regs , 0 , 24 );

	m_exposuretime = exposuretime;

	time = (int) (exposuretime - (exposuretime/100));
	if( amp == 2 )
	{
		if( time > 550 )
			amp = 0;
	}
	if( amp == 0 )
	{
		HM = 0x40;
		time = MAX( 1, time - 500 );
	}

	switch ( binn )
	{
	case 1:
		H_SIZE = QHY6_WIDTH_B1*2 ;
		V_SIZE = QHY6_HEIGHT_B1;
		width = QHY6_WIDTH_B1;
		height = QHY6_HEIGHT_B1;
		Regs[6] = vbe-1;
		break;
	case 2:
		H_SIZE =  QHY6_WIDTH_B2*2;
		V_SIZE =  QHY6_HEIGHT_B2;
		RM |= 0x08;
		width  =  QHY6_WIDTH_B2;
		height =  QHY6_HEIGHT_B2;
		Regs[6] = 2;
		RM |= 0x48;
		break;
	default:
		return 2;
	}

	int totalsize = H_SIZE * V_SIZE;
	totalsize = ((totalsize>>10)+1)<<10;
	Padding = totalsize-(H_SIZE*V_SIZE);

	if( out_width )
		*out_width = width;
	if( out_height )
		*out_height = height;
	if( out_buffer_size )
		*out_buffer_size = totalsize;

	if( speed )
		RM |= 0x80;

	Regs[0] = (int)((gain*63)/100);
	Regs[1] = (time >> 16) & 0xFF;
	Regs[2] = (time >> 8 ) & 0xFF;
	Regs[3] = (time      ) & 0xFF;
	Regs[4] = RM;
	Regs[5] = HM;
	Regs[7] = offset;
	BUFWORD( Regs,  8 , 0xACAC );
	BUFWORD( Regs, 14 , H_SIZE );
	BUFWORD( Regs, 16 , V_SIZE );
	BUFWORD( Regs, 18 , Padding );
	Regs[20] = (binn == 4) ? 1 : 0;
	Regs[23] = 0xAC;

#ifdef NO_QHY6
	if( m_handle == NULL )
		return EXIT_SUCCESS;
#endif

	pthread_mutex_lock( &m_mutex );

	ret = ctrl_msg( 0x42, SENDREGS, 0, 0, Regs, sizeof(Regs) );

	pthread_mutex_unlock( &m_mutex );

	return ret;
}

#endif
