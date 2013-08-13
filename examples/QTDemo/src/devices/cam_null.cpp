/*
 * cam_null.cpp
 *
 *  Created on: 10.10.2011
 *      Author: gm
 */
#include <unistd.h>
#include <string.h>

#include "camera.h"
#include "cam_null.h"
#include "timer.h"
#include "utils.h"
#include "image.h"


ccamera_null::ccamera_null()
{
	// load params
	m_high_params.load();
}


ccamera_null::~ccamera_null()
{
	stop();

	// save params
	m_high_params.save();
}


camera_model::model ccamera_null::get_model( void ) const
{
	return camera_model::null;
}


int ccamera_null::open_device( void )
{
	m_initialized = true;
	log_i( "Connected to NULL-camera" );
 return 0;
}


void ccamera_null::close_device( void )
{
	m_initialized = false;
	log_i( "Disconnected" );
}


cam_base_params *ccamera_null::alloc_params_object( void ) const
{
	return new cam_null_params();
}


bool ccamera_null::set_params( cam_base_params *params )
{
 cam_null_params *h_params = NULL;


	h_params = dynamic_cast<cam_null_params *>(params);

	if( !m_initialized || !h_params )
		return false;

	m_high_params	= *h_params;

 return true;
}


bool ccamera_null::get_params( cam_base_params *params )
{
 cam_null_params *dst = dynamic_cast<cam_null_params *>(params);

	if( !m_initialized || !dst )
		return false;

	dst->reset();

	m_high_params.Exposition		= m_high_params.Exposition; //1000;
	m_high_params.Binning			= 1;

	*dst = m_high_params;

 return true;
}


bool ccamera_null::get_params_copy( cam_base_params *params ) const
{
 cam_null_params *dst = dynamic_cast<cam_null_params *>(params);

	if( !m_initialized || !dst )
		return false;

	*dst = m_high_params;

 return true;
}


const cam_base_params &ccamera_null::get_params_ref( void ) const
{
 return m_high_params;
}


bool ccamera_null::get_frame( void )
{
 frame_params_t frame_params;
 int len;
 bool ret = false;

	if( !m_initialized )
		return false;

	ret = get_frame_params( &frame_params );

	if( !ret )
		return false;

	init_buffer( frame_params );

	len = frame_params.width * frame_params.height;

	ret = set_thread_task( MK_CMD("@IMG"), NULL, 0, (char*)m_buffer.data, len*pixel_base::bytes_per_pixel(frame_params.bpp), false );
	if( !ret )
	{
		log_e("ccamera_null::get_frame: set_thread_task error");
		return false;
	}

 return true;
}


bool ccamera_null::get_frame_params( frame_params_t *params )
{
 frame_params_t frame_params;


	 if( params != NULL )
		 memset( params, 0, sizeof(frame_params_t) );

	 frame_params.width 	= 1000;
	 frame_params.height	= 800;
	 frame_params.bpp   	= pixel_base::BPP_RGB_24; //pixel_base::BPP_MONO_16; //pixel_base::BPP_RGB_24

	*params = frame_params;

 return true;
}


int ccamera_null::do_command( cam_task_t *task )
{
	ctimer progress_timer;
	int nsec = 0;
	unsigned long now;
	int ret = ERRCAM_OK;
	//bool break_done = false;

	if( strncmp( task->cmd, "@IMG", task->cmd_len ) == 0 )
	{
		unsigned char *raw = NULL;
		if( task->out_len != (1000 * 800 * pixel_base::bytes_per_pixel(pixel_base::BPP_RGB_24)) || !(raw = reinterpret_cast<unsigned char *>(task->out)) )
		{
			log_e( "ccamera_null::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 6;
		}

		log_i( "Start capture..." );

		// wait for exposition
		if( m_notify_wnd )
			QApplication::postEvent( m_notify_wnd, new cam_progress_Event(0) );

		progress_timer.start();

		while( (now = progress_timer.gettime()) < (unsigned long)m_high_params.Exposition )
		{
			if( m_frame_break && now < (unsigned long)m_high_params.Exposition-100 )
			{
				//TODO: set hardware BREAK
				//break_done = true;
				break;
			}
			if( (unsigned long)m_high_params.Exposition - now > progress_time_limit )
			{
				nsec++;
				// emit time event...
				if( m_notify_wnd && nsec % 4 == 0 )
					QApplication::postEvent( m_notify_wnd, new cam_progress_Event(now) );
				if( m_frame_break && now < (unsigned long)m_high_params.Exposition-100 )
					break;
				usleep( 50000 );
			}
			else
				usleep( 1000 );
		}

		unsigned char *ptr = (unsigned char *)raw;
		for( int i = 0;i < 800*1000*3;i++ )
		{
			if( i%3 == 0 )
				ptr[i] = rand() % 256;
			else
				ptr[i] = rand() % 50;
		}

//		unsigned short *ptr = (unsigned short *)raw;
//		for( int i = 0;i < 800*1000;i++ )
//		{
//			ptr[i] = rand() % 32000;
//		}
		log_i( "Capture done." );
	}



	if( m_frame_break )
	{
		ret = ERRCAM_USER_BREAK;
	}

 return ret;
}
