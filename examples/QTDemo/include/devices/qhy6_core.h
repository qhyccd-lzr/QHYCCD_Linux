/*
 * qhy6_core.h
 *
 *  Created on: 23.11.2011
 *      Author: gm
 */

#ifndef QHY6_CORE_H_
#define QHY6_CORE_H_

#include <pthread.h>
#include <libusb-1.0/libusb.h>

#define QHY6_WITH_ST4

enum {
        QHY6_NORTH = 0x04,
        QHY6_SOUTH = 0x02,
        QHY6_EAST  = 0x01,
        QHY6_WEST  = 0x08,
};

#ifdef QHY6_WITH_ST4
	// new
	#define QHY6_VENDOR_ID   0x1618
	#define QHY6_PRODUCT_ID  0x025A

	#define QHY6_MATRIX_WIDTH	800
	#define QHY6_MATRIX_HEIGHT	596

#else
	// old
	#define QHY6_VENDOR_ID   0x16C0 //0x1618 //0x16C0
	#define QHY6_PRODUCT_ID  0x081D //0x0259 //0x081D

	#define QHY6_MATRIX_WIDTH	796
	#define QHY6_MATRIX_HEIGHT	596
#endif



#define QHY6_WIDTH_B1	QHY6_MATRIX_WIDTH
#define QHY6_HEIGHT_B1	QHY6_MATRIX_HEIGHT

#define QHY6_WIDTH_B2	(QHY6_WIDTH_B1>>1)
#define QHY6_HEIGHT_B2	(QHY6_HEIGHT_B1>>1)

#define QHY6_BUFFER_LEN 480000	// 800*600


class qhy6_core_shared
{
public:
	qhy6_core_shared();
	virtual ~qhy6_core_shared();

	int open_device( void );		// open device
	void close_device( void );		// close device

	int start_exposure( unsigned int exposure );
	int read_exposure( unsigned char *image, unsigned int image_size );
	int set_params( int exposuretime, int binn, int gain, int offset, int speed, int amp, int vbe, int *out_width, int *out_height, int *out_buffer_size );
private:
	int ctrl_msg( int request_type, int request, unsigned int value, unsigned int index, unsigned char *data, int len );
	libusb_device_handle *locate_device( unsigned int vid, unsigned int pid );

	// shared static data
	static pthread_mutex_t m_mutex;
	static struct libusb_device_handle *m_handle;
	static int m_init_cnt;
	static int m_exposuretime;
};

#endif /* QHY6_CORE_H_ */
