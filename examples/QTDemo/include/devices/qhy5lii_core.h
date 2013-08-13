#ifndef QHY5LII_CORE_H
#define QHY5LII_CORE_H

#include <pthread.h>


enum {
        QHY5LII_NORTH = 0x20,
        QHY5LII_SOUTH = 0x40,
        QHY5LII_EAST  = 0x10,
        QHY5LII_WEST  = 0x80,
};


#define QHY5LII_MATRIX_WIDTH	1558
#define QHY5LII_MATRIX_HEIGHT  1048

#define QHY5LII_WIDTH_B1	1280
#define QHY5LII_HEIGHT_B1	1024

#define QHY5LII_WIDTH_B2	640
#define QHY5LII_HEIGHT_B2	512


#define STORE_WORD_BE(var, val) *(var) = ((val) >> 8) & 0xff; *((var) + 1) = (val) & 0xff

class qhy5lii_core_shared
{
public:
        qhy5lii_core_shared();
        virtual ~qhy5lii_core_shared();

        int open_device( void );		// open device
        void close_device( void );		// close device

        int reset_camera( void );
        int start_exposure( unsigned int exposure );
        int read_exposure( unsigned char *image, unsigned int image_size );
        int set_params( int exposuretime, int binn, int gain, int first_time, int *out_width, int *out_height, int *out_buffer_size );
private:
        // shared static data
        static pthread_mutex_t m_mutex;
        static struct libusb_device_handle *m_handle;
        static int m_init_cnt;
        static int m_exposuretime;
        static int gain_lut[74];
};

#endif // QHY5LII_CORE_H
