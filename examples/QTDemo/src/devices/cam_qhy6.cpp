/*
 * cam_qhy6.cpp
 *
 *  Created on: 22.11.2011
 *      Author: gm
 */
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <errno.h>

#include "camera.h"
#include "cam_qhy6.h"
#include "timer.h"
#include "utils.h"
#include "image.h"
#include "qhy6_core.h"


// fills data buffer by fake data
//#define NO_QHY6

ccamera_qhy6::ccamera_qhy6() : m_qhy6_obj( NULL )
{
	m_qhy6_obj = new qhy6_core_shared();

	// load params
	m_high_params.load();
}


ccamera_qhy6::~ccamera_qhy6()
{
	stop();

	if( m_qhy6_obj )
		delete m_qhy6_obj;

	// save params
	m_high_params.save();
}


camera_model::model ccamera_qhy6::get_model( void ) const
{
	return camera_model::qhy6;
}


int ccamera_qhy6::open_device( void )
{
	int ret = EXIT_FAILURE;

	if( m_initialized )
		return 0;

	ret = m_qhy6_obj->open_device();

	if( ret == EXIT_SUCCESS )
	{
		qhy6_params_t p;
		p.exposure	= (int)m_high_params.Exposition;
		p.binn		= m_high_params.Binning;
		p.gain		= m_high_params.gain;
		p.offset	= m_high_params.offset;
		p.speed		= m_high_params.readout_speed;
		p.amp		= m_high_params.amp;
		p.vbe		= m_high_params.vbe;
		p.out_frame_width = 0;
		p.out_frame_height = 0;

		ret = m_qhy6_obj->set_params( p.exposure,
				p.binn,
				p.gain,
				p.offset,
				p.speed,
				p.amp,
				p.vbe,
				&p.out_frame_width,
				&p.out_frame_height,
				NULL );
		if( ret == EXIT_SUCCESS )
		{
			m_low_params  = p;
			m_initialized = true;
			log_i( "Connected to QHY6 camera" );
		}
		else
			close_device();
	}

 return ret;
}


void ccamera_qhy6::close_device( void )
{
	m_qhy6_obj->close_device();

	m_initialized = false;
	log_i( "Disconnected" );
}


cam_base_params *ccamera_qhy6::alloc_params_object( void ) const
{
	return new cam_qhy6_params();
}


bool ccamera_qhy6::set_params( cam_base_params *params )
{
 cam_qhy6_params *h_params = NULL;
 qhy6_params_t p;
 bool ret = false;


	h_params = dynamic_cast<cam_qhy6_params *>(params);

	if( !m_initialized || !h_params )
		return false;

	p.exposure = (int)h_params->Exposition;
	p.binn     = h_params->Binning;
	p.gain	   = h_params->gain;
	p.offset   = h_params->offset;
	p.speed	   = h_params->readout_speed;
	p.amp	   = h_params->amp;
	p.vbe	   = h_params->vbe;
	p.out_frame_width  = 0;
	p.out_frame_height = 0;

	// goto thread
	ret = set_thread_task( MK_CMD("@SETPAR"), (char*)&p, sizeof(qhy6_params_t), NULL, 0, true );
	if( !ret )
	{
		log_e("ccamera_qhy6::set_params: set_thread_task error");
		return false;
	}

	m_high_params = *h_params;
	m_low_params  = p;

 return true;
}


bool ccamera_qhy6::get_params( cam_base_params *params )
{
 cam_qhy6_params *dst = dynamic_cast<cam_qhy6_params *>(params);
 bool ret = false;

	if( !m_initialized || !dst )
		return false;

	dst->reset();

	// goto thread
	ret = set_thread_task( MK_CMD("@GETPAR"), NULL, 0, (char*)&m_low_params, sizeof(qhy6_params_t), true );
	if( !ret )
	{
		log_e("ccamera_qhy6::get_params: set_thread_task error");
		return false;
	}

	m_high_params.Exposition 	= (double)m_low_params.exposure;
	m_high_params.Binning    	= m_low_params.binn;
	m_high_params.gain		 	= m_low_params.gain;
	m_high_params.offset	 	= m_low_params.offset;
	m_high_params.readout_speed = m_low_params.speed;
	m_high_params.amp		 	= m_low_params.amp;
	m_high_params.vbe		 	= m_low_params.vbe;

	*dst = m_high_params;

 return true;
}


bool ccamera_qhy6::get_params_copy( cam_base_params *params ) const
{
 cam_qhy6_params *dst = dynamic_cast<cam_qhy6_params *>(params);

	if( !m_initialized || !dst )
		return false;

	*dst = m_high_params;

 return true;
}


const cam_base_params &ccamera_qhy6::get_params_ref( void ) const
{
 return m_high_params;
}


bool ccamera_qhy6::get_frame( void )
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
		log_e("ccamera_qhy6::get_frame: set_thread_task error");
		return false;
	}

 return true;
}


bool ccamera_qhy6::get_frame_params( frame_params_t *params )
{
 frame_params_t frame_params;


	 if( params != NULL )
		 memset( params, 0, sizeof(frame_params_t) );

	 assert( m_low_params.out_frame_width*m_low_params.out_frame_height*sizeof(uint16_t) == (size_t)m_low_params.out_buffer_size );

	 frame_params.width 	= m_low_params.out_frame_width;
	 frame_params.height	= m_low_params.out_frame_height;
	 frame_params.bpp   	= pixel_base::BPP_MONO_16;

	*params = frame_params;

 return true;
}


int ccamera_qhy6::do_command( cam_task_t *task )
{
	qhy6_params_t *p = NULL;
	ctimer progress_timer;
	int nsec = 0;
	unsigned long now;
	int ret = EXIT_SUCCESS;
	bool break_done = false;

	if( strncmp( task->cmd, "@SETPAR", task->cmd_len ) == 0 )
	{
		if( task->param_len != sizeof(qhy6_params_t) || !(p = reinterpret_cast<qhy6_params_t *>(task->param)) )
		{
			log_e( "ccamera_qhy6::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 1;
		}

		int res = m_qhy6_obj->set_params( p->exposure, p->binn, p->gain, p->offset, p->speed, p->amp, p->vbe, &p->out_frame_width, &p->out_frame_height, &p->out_buffer_size );
		if( res != EXIT_SUCCESS )
		{
			log_e( "ccamera_qhy6::do_command(): CMD = %.*s failed.", task->cmd_len, task->cmd );
			return 2;
		}

		log_i( "Config set_params OK" );
	}
	else
	if( strncmp( task->cmd, "@GETPAR", task->cmd_len ) == 0 )
	{
		if( task->out_len != sizeof(qhy6_params_t) || !(p = reinterpret_cast<qhy6_params_t *>(task->out)) )
		{
			log_e( "ccamera_qhy6::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 3;
		}

		qhy6_params_t lp;
		lp = m_low_params;		// emulation of getting parameters (already got by @SETPAR)

		memcpy( task->out, &lp, sizeof(qhy6_params_t) );

		log_i( "Config get_params OK" );
	}
	else
	if( strncmp( task->cmd, "@IMG", task->cmd_len ) == 0 )
	{
		uint16_t *raw = NULL;
		if( task->out_len != m_low_params.out_buffer_size || !(raw = reinterpret_cast<uint16_t *>(task->out)) )
		{
			log_e( "ccamera_qhy6::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 4;
		}

		log_i( "Start capture..." );

		int res = m_qhy6_obj->start_exposure( m_low_params.exposure );
		if( res != EXIT_SUCCESS )
		{
			log_e( "ccamera_qhy6::do_command(): CMD = %.*s failed. Unable to start exposure", task->cmd_len, task->cmd );
			return 5;
		}

		// wait for exposition
		if( m_notify_wnd )
			QApplication::postEvent( m_notify_wnd, new cam_progress_Event(0) );

		progress_timer.start();

		// this camera can not break capturing! so...  we have to ignore m_frame_break
		while( (now = progress_timer.gettime()) < (unsigned long)m_high_params.Exposition )
		{
			if( m_frame_break && now < (unsigned long)m_high_params.Exposition-100 )
			{
				//hardware BREAK
				for( int i = 0;i < ccamera_base::try_reset_cnt;i++ )
				{
					int res = system( "utils/qhy6reset.sh" );
					if( res < 0 )
					{
						log_e( "Failed to reset camera by script... '%s'", strerror(errno) );
					}
					else
					{
						ret = m_qhy6_obj->set_params( m_low_params.exposure,
								m_low_params.binn,
								m_low_params.gain,
								m_low_params.offset,
								m_low_params.speed,
								m_low_params.amp,
								m_low_params.vbe,
								NULL,
								NULL,
								NULL );
						if( res == EXIT_SUCCESS )
							break;
						else
							log_e( "ccamera_qhy6::do_command(): set_params() failed after reset. Trying #%d", i+1 );
					}
				}
				break_done = true;
				break;
			}
			if( (unsigned long)m_high_params.Exposition - now > progress_time_limit )
			{
				nsec++;
				// emit time event...
				if( m_notify_wnd && nsec % 4 == 0 )
					QApplication::postEvent( m_notify_wnd, new cam_progress_Event(now) );
				usleep( 50000 );
			}
			else
				usleep( 1000 );
		}

		log_i( "waiting finished. time = %d", progress_timer.gettime() );

		// time has gone... read frame
		if( !break_done )
		{
			if( m_notify_wnd )
				QApplication::postEvent( m_notify_wnd, new cam_progress_Event(progress_timer.gettime()) );

			unsigned char *raw_buffer = (unsigned char *)malloc( m_low_params.out_buffer_size );
			if( !raw_buffer )
			{
				log_e( "ccamera_qhy6::do_command(): CMD = %.*s failed. Unable to allocate raw data buffer", task->cmd_len, task->cmd );
				return 6;
			}
			int res = m_qhy6_obj->read_exposure( raw_buffer, m_low_params.out_buffer_size );
			if( res != EXIT_SUCCESS )
			{
				free( raw_buffer );
				log_e( "ccamera_qhy6::do_command(): CMD = %.*s failed.", task->cmd_len, task->cmd );
				return 7;
			}
#ifdef NO_QHY6
			uint16_t *ptr = raw_buffer;
			for( int i = 0;i < m_low_params.out_frame_width*m_low_params.out_frame_height;i++ )
			{
				ptr[i] = rand() % 65535;
			}
#endif
			// convert to target buffer format
			// swap
#define SWAP(a,b) { a ^= b; a ^= (b ^= a); }
			unsigned data_size = m_low_params.out_buffer_size;
			if( htons(0x55aa) != 0x55aa )
				for( unsigned i = 0;i < data_size;i += 2 )
					SWAP( raw_buffer[i], raw_buffer[i+1] );
#undef SWAP

			// decode
			uint16_t *src1, *src2, *tgt;
			int line_sz = m_low_params.out_frame_width * sizeof( uint16_t );
			unsigned int t;
			switch( m_low_params.binn )
			{
			case 1:  //1X1 binning
				t = m_low_params.out_frame_height >> 1;
				src1 = (uint16_t*)raw_buffer;
				src2 = (uint16_t*)raw_buffer + m_low_params.out_frame_width * t;
				tgt = (uint16_t*)raw;

				while( t-- )
				{
					memcpy( tgt, src1, line_sz );
					tgt += m_low_params.out_frame_width;
					memcpy( tgt, src2, line_sz );
					tgt += m_low_params.out_frame_width;


					src1 += m_low_params.out_frame_width;
					src2 += m_low_params.out_frame_width;
				}
				break;
			case 2:  //2X2 binning
				log_e( "cvideo_qhy6::read_frame(): binning x2 not supported" );
				break;
			}
			free( raw_buffer );

			if( m_notify_wnd )
				QApplication::postEvent( m_notify_wnd, new cam_progress_Event(progress_timer.gettime()) );
		}
		if( m_frame_break )
			ret = ERRCAM_USER_BREAK;
		else
		{
			log_i( "downloading finished. time = %d", progress_timer.gettime() );
			log_i( "Capture done." );
		}
	}

 return ret;
}
