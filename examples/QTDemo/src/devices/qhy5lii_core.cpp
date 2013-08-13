
/*
 * qhy5_core.cpp
 *
 *  Created on: 10.02.2011
 *      Author: gm
 *
 * Original code: Copyright(c) 2009 Geoffrey Hausheer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
//#include "lusb.h"
#include "qhy5lii_core.h"


// disables hardware access for QHY5lii
//#define NO_QHY5LII


pthread_mutex_t qhy5lii_core_shared::m_mutex = PTHREAD_MUTEX_INITIALIZER;
libusb_device_handle *qhy5lii_core_shared::m_handle = NULL;
int qhy5lii_core_shared::m_init_cnt = 0;
int qhy5lii_core_shared::m_exposuretime = 0;


int qhy5lii_core_shared::gain_lut[74] = {0x000,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,
                   0x00C,0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,
                   0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,0x01D,
                   0x01E,0x01F,0x051,0x052,0x053,0x054,0x055,0x056,0x057,
                   0x058,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,0x6CE,
                   0x6CF,0x6D0,0x6D1,0x6D2,0x6D3,0x6D4,0x6D5,0x6D6,0x6D7,
                   0x6D8,0x6D9,0x6DA,0x6DB,0x6DC,0x6DD,0x6DE,0x6DF,0x6E0,
                   0x6E1,0x6E2,0x6E3,0x6E4,0x6E5,0x6E6,0x6E7,0x6FC,0x6FD,0x6FE,0x6FF};


qhy5lii_core_shared::qhy5lii_core_shared( void )
{
#if 0
        int ret = lusb::initialize();

        if( ret != 0 )
                log_e( "qhy5lii_core_shared::qhy5lii_core_shared(): Could not initialize libusb" );
#endif
}


qhy5lii_core_shared::~qhy5lii_core_shared( void )
{
#if 0
        lusb::release();
#endif
}


int qhy5lii_core_shared::open_device( void )
{
 int res = EXIT_SUCCESS;

        pthread_mutex_lock( &m_mutex );

#ifdef NO_QHY5LII
        if( m_init_cnt == 0 && lusb::is_initialized() )
                log_i( "qhy5lii_core_shared::open_device(): Success" );
#else
#if 0
        if( m_handle == NULL && m_init_cnt == 0 && lusb::is_initialized() )
        {
                if( (m_handle = locate_device( QHY5LII_VENDOR_ID, QHY5LII_PRODUCT_ID)) == NULL )
                {
                        log_e( "qhy5lii_core_shared::open_device(): Could not open the QHY5LII device" );
                        res = EXIT_FAILURE;
                }
                else
                        log_i( "qhy5lii_core_shared object successfully opened." );

        }
#endif
#endif
        if( res == EXIT_SUCCESS )
                m_init_cnt++;

        pthread_mutex_unlock( &m_mutex );

 return res;
}


void qhy5lii_core_shared::close_device( void )
{
        pthread_mutex_lock( &m_mutex );

        if( m_init_cnt <= 0 )
        {
                log_e( "qhy5lii_core_shared::close_device(): Already closed or not opened yet" );
        }
        else
        {
                m_init_cnt--;
#ifdef NO_QHY5LII
                if( m_init_cnt == 0 )
                        log_i( "qhy5lii_core_shared::close_device(): Success" );
#else
                if( m_handle && m_init_cnt == 0 )
                {
                        //libusb_close( m_handle );
                        m_handle = NULL;
                        log_i( "qhy5lii_core_shared::close_device(): Success" );
                }
#endif
        }

        pthread_mutex_unlock( &m_mutex );
}




int qhy5lii_core_shared::reset_camera( void )
{
        int ret = EXIT_SUCCESS;

#ifdef NO_QHY5LII
        if( m_handle == NULL )
                return EXIT_SUCCESS;
#endif

        pthread_mutex_lock( &m_mutex );

        assert( m_handle != NULL );

        unsigned char data = 0x00;
        int transferred;

        log_i("Resetting\n");
        int res = 0;// libusb_bulk_transfer( m_handle, 1, &data, 1, &transferred, 5000 );
        if( res < 0 )
        {
                log_e( "qhy5lii_core_shared::reset_camera(): failed" );
                ret = EXIT_FAILURE;
        }

        pthread_mutex_unlock( &m_mutex );

        return ret;
}



int qhy5lii_core_shared::start_exposure( unsigned int exposure )
{
#ifdef NO_QHY5LII
        if( m_handle == NULL )
                return 0;
#endif

        int index, value, ret;
        unsigned char buffer[10]; // for debug purposes
        index = exposure >> 16;
        value = exposure & 0xffff;

        pthread_mutex_lock( &m_mutex );

        memcpy( buffer, "DEADEADEAD", 10 );
        usleep(20000);
        //ret = ctrl_msg( 0xc2, 0x12, value, index, buffer, 2 );

        pthread_mutex_unlock( &m_mutex );

        return ret;
}


int qhy5lii_core_shared::read_exposure( unsigned char *image, unsigned int image_size )
{
        int result, ret = EXIT_SUCCESS;

#ifdef NO_QHY5LII
        if( m_handle == NULL )
                return 0;
#endif

        //pthread_mutex_lock( &m_mutex );

        assert( m_handle != NULL );

        log_i("Reading %d bytes...", image_size);

        int res = 0;//libusb_bulk_transfer( m_handle, 0x82, image, image_size, &result, 20000);

        if( res < 0 || result != (int)image_size )
        {
                log_e( "Failed to read image. result = %d, Got: %d, expected %u", res, result, image_size );
                ret = EXIT_FAILURE;
        }
        else
        {
                log_i( "Done" );
        }

        //pthread_mutex_unlock( &m_mutex );

 return ret;
}


int qhy5lii_core_shared::set_params( int exposuretime, int binn, int gain, int first_time, int *out_width, int *out_height, int *out_buffer_size )
{
        unsigned char reg[19];
        int offset, value, index;
        int height = QHY5LII_HEIGHT_B1;
        int gain_val, gain_lut_sz = (int)(sizeof(gain_lut) / sizeof(int));

        memset( reg , 0 , sizeof(reg) );
        m_exposuretime = exposuretime;

        height -=  height % 4;
        offset = (QHY5LII_MATRIX_HEIGHT - height) / 2;
        index = (QHY5LII_MATRIX_WIDTH * (height + 26)) >> 16;
        value = (QHY5LII_MATRIX_WIDTH * (height + 26)) & 0xffff;
        //gain = gain * 0x6ff / 100;
        if( gain >= gain_lut_sz )
                gain = gain_lut_sz - 1;
        if( gain < 0 )
                gain = 0;
        gain_val = gain_lut[ gain ];
        STORE_WORD_BE(reg + 0,  gain_val);
        STORE_WORD_BE(reg + 2,  gain_val);
        STORE_WORD_BE(reg + 4,  gain_val);
        STORE_WORD_BE(reg + 6,  gain_val);
        STORE_WORD_BE(reg + 8,  offset);
        STORE_WORD_BE(reg + 10, 0);
        STORE_WORD_BE(reg + 12, height - 1);
        STORE_WORD_BE(reg + 14, 0x0521);
        STORE_WORD_BE(reg + 16, height + 25);
        reg[18] = 0xcc;

        int wd = QHY5LII_WIDTH_B1;
        int ht = QHY5LII_HEIGHT_B1;
        switch( binn )
        {
        case 1:
                break;
        case 2:
                wd = QHY5LII_WIDTH_B2;
                ht = QHY5LII_HEIGHT_B2;
                break;
        default:
                return EXIT_FAILURE;
        }
        if( out_width )
                *out_width  = wd;
        if( out_height )
                *out_height = ht;
        if( out_buffer_size )
                *out_buffer_size = QHY5LII_MATRIX_WIDTH * (height + 26) * 1; // pixelsize = 1 byte

#ifdef NO_QHY5LII
        if( m_handle == NULL )
                return EXIT_SUCCESS;
#endif

        pthread_mutex_lock( &m_mutex );

#if 0
        ctrl_msg(0x42, 0x13, value, index, reg, 19);
        usleep(20000);
        ctrl_msg(0x42, 0x14, 0x31a5, 0, reg, 0);
        usleep(10000);
        ctrl_msg(0x42, 0x16, first_time ? 1 : 0, 0, reg, 0);
#endif
        pthread_mutex_unlock( &m_mutex );

        return EXIT_SUCCESS;
}
