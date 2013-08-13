/*
 * qhy5_core.h
 *
 *  Created on: 10.02.2011
 *      Author: gm
 */

#ifndef QHY5_CORE_H_
#define QHY5_CORE_H_

#include <pthread.h>
#include <libusb-1.0/libusb.h>

#define QHY5_VENDOR_ID   0x16c0
#define QHY5_PRODUCT_ID  0x296d

enum {
        QHY5_NORTH = 0x20,
        QHY5_SOUTH = 0x40,
        QHY5_EAST  = 0x10,
        QHY5_WEST  = 0x80,
};


#define QHY5_MATRIX_WIDTH	1558
#define QHY5_MATRIX_HEIGHT  1048

#define QHY5_WIDTH_B1	1280
#define QHY5_HEIGHT_B1	1024

#define QHY5_WIDTH_B2	640
#define QHY5_HEIGHT_B2	512


#define STORE_WORD_BE(var, val) *(var) = ((val) >> 8) & 0xff; *((var) + 1) = (val) & 0xff

class qhy5_core_shared
{
public:
	qhy5_core_shared();
	virtual ~qhy5_core_shared();

	int open_device( void );		// open device
	void close_device( void );		// close device

	int reset_camera( void );
	int start_exposure( unsigned int exposure );
	int read_exposure( unsigned char *image, unsigned int image_size );
	int set_params( int exposuretime, int binn, int gain, int first_time, int *out_width, int *out_height, int *out_buffer_size );
private:
	int ctrl_msg( unsigned char request_type, unsigned char request, unsigned int value, unsigned int index, unsigned char *data, unsigned char len );
	libusb_device_handle *locate_device( unsigned int vid, unsigned int pid );

	// shared static data
	static pthread_mutex_t m_mutex;
	static struct libusb_device_handle *m_handle;
	static int m_init_cnt;
	static int m_exposuretime;
	static int gain_lut[74];
};

#endif /* QHY5_CORE_H_ */
