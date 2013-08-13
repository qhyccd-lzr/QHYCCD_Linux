/*
 * lusb.cpp
 *
 *  Created on: 23.05.2011
 *      Author: gm
 */

#include <libusb-1.0/libusb.h>
#include "lusb.h"
#include "utils.h"


int lusb::m_init_cnt = 0;

int lusb::initialize( void )
{
 int ret = 0;


 	if( m_init_cnt == 0 )
 	{
 		ret = libusb_init( NULL );
 		libusb_set_debug( NULL, 3 );

 		ret == 0 ? log_i( "lusb::initialize(): Success" ) : log_e( "lusb::initialize(): Failed: error code = %d", ret );
 	}
 	if( ret == 0 )
 		m_init_cnt++;

 return ret;
}


int lusb::release( void )
{
 int ret = 0;

	if( m_init_cnt <= 0 )
	{
		log_e( "lusb::release(): Alreade released or not initialized yet!" );
		ret = -1;
	}
	else
	{
		m_init_cnt--;
		if( m_init_cnt == 0 )
		{
			libusb_exit( NULL );
			log_i( "lusb::release(): Success" );
		}
	}

 return ret;
}


bool lusb::is_initialized( void )
{
	return m_init_cnt > 0;
}
