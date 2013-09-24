/*
 * cam_ic8300.cpp
 *
 *  Created on: 29.11.2011
 *      Author: gm
 *      Patched by Vladimir Volynsky
 */
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <netinet/in.h>
#include <errno.h>

#include "camera.h"
//#include "lusb.h"
#include "cam_ic8300.h"
#include "timer.h"
#include "utils.h"
#include "image.h"
#include <libqhyccd/common.h>

// disables hardware access for ic8300
//#define NO_ic8300

#define SENDREGS  0xB5


// Magic numbers have been researched by Vladimir Volynsky
const int ccamera_ic8300::m_binn_loading_time[ic8300_BINN_CNT] = { 11300, 4500, 3000, 2600 };
const int ccamera_ic8300::m_binn_loading_size[ic8300_BINN_CNT] = { 18429440, 4606976, 2051584, 1150976 };

ccamera_ic8300::ccamera_ic8300() :
	m_handle( NULL )
{
        //int ret = lusb::initialize();
        //if( ret != 0 )
        //	log_e( "ccamera_ic8300::ccamera_ic8300(): Could not initialize libusb" );

	// load params
	m_high_params.load();
}


ccamera_ic8300::~ccamera_ic8300()
{
	stop();

        //lusb::release();

	// save params
	m_high_params.save();
}


camera_model::model ccamera_ic8300::get_model( void ) const
{
	return camera_model::ic8300;
}


int ccamera_ic8300::open_device( void )
{
	int ret = EXIT_SUCCESS;
//int a=ret/0;
	if( m_initialized )
		return EXIT_SUCCESS;

#ifdef NO_ic8300
	if( lusb::is_initialized() )
		log_i( "ccamera_ic8300::open_device(): Success" );
#else
        ret = OpenCamera();
        if(ret <= 0)
        {
            log_e( "ccamera_ic8300::open_device(): Could not open the ic8300 device" );
            return EXIT_FAILURE;
        }
        ret = EXIT_SUCCESS;

#endif
        // set defaults
        ic8300_params_t p;
        p.exposure	= (int)m_high_params.Exposition;
        p.binn		= m_high_params.Binning;
        p.gain		= m_high_params.gain;
        p.offset	= m_high_params.offset;
        p.speed		= m_high_params.readout_speed;
        p.amp		= m_high_params.amp;
        p.shutter	= m_high_params.shutter;
        p.pwm		= m_high_params.pwm;
        p.fan		= m_high_params.fan;
        p.out_frame_width = 0;
        p.out_frame_height = 0;
        p.out_buffer_size = 0;

        ret = set_params( p.exposure,
                        p.binn,
                        p.gain,
                        p.offset,
                        p.speed,
                        p.amp,
                        p.shutter,
                        p.pwm,
                        &p.out_frame_width,
                        &p.out_frame_height,
                        NULL );
        if( ret != EXIT_SUCCESS )
        {
                log_e( "ccamera_ic8300::open_device(): set_params() failed" );
                return EXIT_FAILURE;
        }

        m_low_params  = p;
        m_initialized = true;
        log_i( "Connected to ic8300 camera" );
        return EXIT_SUCCESS;
}


void ccamera_ic8300::close_device( void )
{
	m_initialized = false;
	log_i( "Disconnected" );
}


cam_base_params *ccamera_ic8300::alloc_params_object( void ) const
{
	return new cam_ic8300_params();
}


bool ccamera_ic8300::set_params( cam_base_params *params )
{
        cam_ic8300_params *h_params = NULL;
        ic8300_params_t p;
        bool ret = false;

	h_params = dynamic_cast<cam_ic8300_params *>(params);

	if( !m_initialized || !h_params )
		return false;

	p.exposure	= (int)h_params->Exposition;
	p.binn		= h_params->Binning;
	p.gain		= h_params->gain;
	p.offset	= h_params->offset;
	p.speed		= h_params->readout_speed;
	p.amp		= h_params->amp;
	p.shutter	= h_params->shutter;
	p.pwm		= h_params->pwm;
	p.fan		= h_params->fan;
	p.out_frame_width = 0;
	p.out_frame_height = 0;
	p.out_buffer_size = 0;

	ret = set_thread_task( MK_CMD("@SETPAR"), (char*)&p, sizeof(ic8300_params_t), NULL, 0, true );
	if( !ret )
	{
		log_e("ccamera_ic8300::set_params: set_thread_task error");
		return false;
	}

	m_high_params = *h_params;
	m_low_params  = p;

 return true;
}


bool ccamera_ic8300::get_params( cam_base_params *params )
{

 cam_ic8300_params *dst = dynamic_cast<cam_ic8300_params *>(params);
 bool ret = false;

	if( !m_initialized || !dst )
		return false;

	dst->reset();

	// goto thread
	ret = set_thread_task( MK_CMD("@GETPAR"), NULL, 0, (char*)&m_low_params, sizeof(ic8300_params_t), true );
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
	m_high_params.shutter	 	= m_low_params.shutter;
	m_high_params.pwm		 	= m_low_params.pwm;
	m_high_params.fan		 	= m_low_params.fan;

	*dst = m_high_params;

 return true;
}


bool ccamera_ic8300::get_params_copy( cam_base_params *params ) const
{

 cam_ic8300_params *dst = dynamic_cast<cam_ic8300_params *>(params);

	if( !m_initialized || !dst )
		return false;

	*dst = m_high_params;

 return true;
}


const cam_base_params &ccamera_ic8300::get_params_ref( void ) const
{
 return m_high_params;
}


bool ccamera_ic8300::exec_slow_ambiguous_synchronous_request( int req_num,
														const std::map< std::string, std::string > &params,
														std::map< std::string, std::string > *result )
{
#if 0
	bool ret = false;

	if( !m_initialized )
		return false;

	switch( req_num )
	{
	case ccamera_ic8300::temp_req:
	{
		(void)params;
		struct info_s info_out = { 0, 0 };
		ret = set_thread_task( MK_CMD("@GETTEMP"), NULL, 0, (char *)&info_out, sizeof(struct info_s), true );
		if( !ret )
		{
			log_e("ccamera_ic8300::exec_slow_ambiguous_synchronous_request(): req_num = %d: set_thread_task error", req_num );
			return false;
		}
		if( result )
		{
			char buf[64];
			snprintf( buf, sizeof(buf)-1, "%f", info_out.temperature );
			result->insert( std::make_pair( "temperature", std::string( buf ) ) );
		}
		log_i( "ccamera_ic8300::exec_slow_ambiguous_synchronous_request(): req = @GETTEMP: temp = %.1f", info_out.temperature );
		break;
	}
	default:
		log_e( "ccamera_ic8300::exec_slow_ambiguous_synchronous_request(): Unknown request nubler" );
		return false;

	}
#endif
	return true;
}


bool ccamera_ic8300::get_frame( void )
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
		log_e("ccamera_ic8300::get_frame: set_thread_task error");
		return false;
	}

 return true;
}


bool ccamera_ic8300::get_frame_params( frame_params_t *params )
{
 frame_params_t frame_params;


	 if( params != NULL )
		 memset( params, 0, sizeof(frame_params_t) );

	 frame_params.width 	= m_low_params.out_frame_width;
	 frame_params.height	= m_low_params.out_frame_height;
	 frame_params.bpp   	= pixel_base::BPP_MONO_16;

	*params = frame_params;

 return true;
}


int ccamera_ic8300::do_command( cam_task_t *task )
{
	ctimer progress_timer;
	int nsec = 0;
	ic8300_params_t *p = NULL;
	unsigned long now;
	int ret = EXIT_SUCCESS;


	// check that we are not in main thread!
	if( strncmp( task->cmd, "@SETPAR", task->cmd_len ) == 0 )
	{
		if( task->param_len != sizeof(ic8300_params_t) || !(p = reinterpret_cast<ic8300_params_t *>(task->param)) )
		{
			log_e( "ccamera_ic8300::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 1;
		}

		int res = set_params( p->exposure, p->binn, p->gain, p->offset, p->speed, p->amp, p->shutter, p->pwm, &p->out_frame_width, &p->out_frame_height, &p->out_buffer_size );
		if( res != EXIT_SUCCESS )
		{
			log_e( "ccamera_ic8300::do_command(): CMD = %.*s failed.", task->cmd_len, task->cmd );
			return 2;
		}
	}
	else
	if( strncmp( task->cmd, "@GETPAR", task->cmd_len ) == 0 )
	{
		if( task->out_len != sizeof(ic8300_params_t) || !(p = reinterpret_cast<ic8300_params_t *>(task->out)) )
		{
			log_e( "ccamera_ic8300::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 3;
		}

		ic8300_params_t lp;
		lp = m_low_params;		// emulation of getting parameters (already got by @SETPAR)

		memcpy( task->out, &lp, sizeof(ic8300_params_t) );

		log_i( "Config get_params OK" );
	}
	else
	if( strncmp( task->cmd, "@IMG", task->cmd_len ) == 0 )
	{
                uint16_t *raw = NULL;
		if( task->out_len != m_low_params.out_buffer_size || !(raw = reinterpret_cast<uint16_t *>(task->out)) )
		{
			log_e( "ccamera_ic8300::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
                        log_e("%d %d",task->out_len,m_low_params.out_buffer_size);
                        return 4;
		}

		log_i( "Start capture..." );

                BeginLive();
		// wait for exposition
		if( m_notify_wnd )
			QApplication::postEvent( m_notify_wnd, new cam_progress_Event(0) );

		progress_timer.start();

		// this camera can not break capturing! so...  we have to ignore m_frame_break

		while( (now = progress_timer.gettime()) < (unsigned long)m_high_params.Exposition )
		{
			if( m_frame_break && now < (unsigned long)m_high_params.Exposition-100 )
			{
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

                if( m_notify_wnd )
                        QApplication::postEvent( m_notify_wnd, new cam_progress_Event(progress_timer.gettime()) );

#ifdef NO_ic8300
                for( int i = 0;i < m_low_params.out_frame_width*m_low_params.out_frame_height;i++ )
                {
                        raw[i] = rand() % 65535;
                }
                for( int i = 0;i < m_low_params.out_frame_height;i++ )
                {
                        raw[i*m_low_params.out_frame_width+i] = 65535;
                }
#else
                GetImageData(m_low_params.out_frame_width,m_low_params.out_frame_height,
                             16,1,(unsigned char *)task->out);
#endif

                // flip vertical
                uint16_t line[ic8300_MATRIX_WIDTH];
                uint8_t *raw_byte = (uint8_t*)raw;
                int line_sz = m_low_params.out_frame_width*sizeof(uint16_t);
                int line_cnt = m_low_params.out_frame_height >> 1;
                int off1 = 0;
                int off2 = (m_low_params.out_frame_height-1) * line_sz;
                for( int i = 0;i < line_cnt;i++ )
                {
                        memmove( (void*)line, raw_byte+off1, line_sz );
                        memmove( raw_byte+off1, raw_byte+off2, line_sz );
                        memmove( raw_byte+off2, line, line_sz );
                        off1 += line_sz;
                        off2 -= line_sz;
                }

                if( m_notify_wnd )
                        QApplication::postEvent( m_notify_wnd, new cam_progress_Event(progress_timer.gettime()) );


                log_i( "downloading finished. time = %d", progress_timer.gettime() );
                log_i( "Capture done." );

	}
	else
	if( strncmp( task->cmd, "@GETTEMP", task->cmd_len ) == 0 )
	{
#if 0
		struct info_s *info_out = NULL;
		if( task->out_len != sizeof(struct info_s) || !(info_out = reinterpret_cast<struct info_s *>(task->out)) )
		{
			log_e( "ccamera_ic8300::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
			return 8;
		}
		ret = get_info( info_out );
#endif
	}


 return ret;
}

#if 0
libusb_device_handle *ccamera_ic8300::locate_device( unsigned int vid, unsigned int pid )
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
#endif

int ccamera_ic8300::ctrl_msg( int request_type, int request, unsigned int value, unsigned int index, unsigned char *data, int len )
{
        int result = 0;
#if 0
#ifdef NO_ic8300
	if( m_handle == NULL )
		return 0;
#endif

	assert( m_handle != NULL );

	result = libusb_control_transfer( m_handle, request_type, request, value, index, data, len, 1000 );
#endif
	return result < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


unsigned char ccamera_ic8300::MSB(unsigned int i)
{
	unsigned int j;
	j = (i&~0x00ff)>>8;
	return j;
}


unsigned char ccamera_ic8300::LSB( unsigned int i )
{
	unsigned int j;
	j = i&~0xff00;
	return j;
}


int ccamera_ic8300::set_params( int exposuretime, int binn, int gain, int offset, int speed, int amp, int shutter, int pwm, int *out_width, int *out_height, int *out_buffer_size )
{
	unsigned char REG[64];
	int PatchNumber, time, Vbin, Hbin, antiamp;
	int ret = EXIT_SUCCESS;
	unsigned char time_H,time_M,time_L;
	int width = 0;
	int height = 0;
	(void)Vbin;
	(void)Hbin;

	memset( REG , 0 , sizeof(REG) );
	time = exposuretime;

	antiamp = 0;
	switch ( amp )
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
		width = ic8300_WIDTH_B1; height = ic8300_HEIGHT_B1; Vbin = binn; Hbin = binn;
		break;
	case 2:
		width = ic8300_WIDTH_B2; height = ic8300_HEIGHT_B2; Vbin = binn; Hbin = binn;
		break;
	case 3:
		width = ic8300_WIDTH_B3; height = ic8300_HEIGHT_B3; Vbin = binn; Hbin = binn;
		break;
	case 4:
		width = ic8300_WIDTH_B4; height = ic8300_HEIGHT_B4; Vbin = binn; Hbin = binn;
		break;
	default:
		return 2;
	}

	// Subframe not implemented as of yet
	//totalsize=width*2*height + reg.TopSkipPix*2;
	//
	int totalsize = width * 2 * height;

	if( out_width )
		*out_width = width;
	if( out_height )
		*out_height = height;
	if( out_buffer_size )
		*out_buffer_size = totalsize;

        SetExposeTime(exposuretime);
        SetGain(gain);
        SetOffset(offset);
        SetResolution(width,height);

#ifdef NO_ic8300
	if( m_handle == NULL )
		return EXIT_SUCCESS;
#endif
	return ret;
}


int ccamera_ic8300::shutter( int cmd )
{
	unsigned char REG[1];
	int ret = EXIT_FAILURE;
#if 0
	//0=open  1=close  2=free
	REG[0] = cmd;

	ret = ctrl_msg( 0x01, 0xC7, 0, 0, REG, 1 );

	if( ret != EXIT_SUCCESS )
		return ret;

	if( cmd < 2 )
	{
		usleep( 250 * 1000 );
	}
#endif
	return EXIT_SUCCESS;
}


int ccamera_ic8300::get_info( struct info_s *info_out, bool dump )
{
	short dc = 0;
	int ret = EXIT_SUCCESS;
#if 0
	ret = get_dc201( &dc );
	if( ret != EXIT_SUCCESS )
	{
		log_e( "ccamera_ic8300::get_info(): get_dc201() failed" );
		return ret;
	}
	double temp = mv_to_degree( 1.024 * (float)dc );
	if( info_out )
	{
		info_out->temperature = temp;
		info_out->voltage 	  = (int)dc;
	}
	if( dump )
		log_i( "temperature = %.1f deg. voltage = %d mV", temp, dc );
#endif
	return ret;
}


int ccamera_ic8300::get_dc201( short *dc )
{
#if 0
	unsigned char REG[4] = {0, 0, 0, 0};

	int ret = ctrl_msg( 0x81, 0xC5, 0, 0, REG, sizeof(REG) );
	if( ret != EXIT_SUCCESS )
		return ret;

	*dc = short(REG[1]) * 256 + short(REG[2]);

	return ret;
#endif

        return 0;
}


double ccamera_ic8300::r_to_degree( double R ) const
{
	double  T;
	double LNR;

	if( R > 400 )
		R = 400;
	if( R < 1 )
		R = 1;

	LNR = log( R );

	T = 1 / ( 0.002679+0.000291*LNR + LNR*LNR*LNR*4.28e-7  );

	T = T - 273.15;

	return T;
}


double ccamera_ic8300::mv_to_degree( double v ) const
{
	double R;
	double T;

	R = 33/(v/1000 + 1.625)-10;
	T = r_to_degree( R );

	return T;
}


int ccamera_ic8300::set_dc201( int pwm )
{
#if 0
	unsigned char REG[3] = {0, 0, 0};

	pwm = pwm < 0 ? 0 : pwm;
	pwm = pwm > 255 ? 255 : pwm;

	if( pwm == 0 )	// TEC off
	{
		REG[0] = 0x01;
		REG[1] = 0x0;
		REG[2] = 0x01;
	}
	else			// TEC manual
	{
		REG[0] = 0x1;
		REG[1] = (unsigned char)pwm;
		REG[2] = 0x85;
	}

	int result = 0;
	int res = 0;

	res = libusb_bulk_transfer( m_handle, 0x01, REG, sizeof(REG), &result, 1000 );
	usleep( 100000 );
	res = libusb_bulk_transfer( m_handle, 0x01, REG, sizeof(REG), &result, 1000 );
	usleep( 100000 );
	log_i( "set_dc201():" );

	return res;
#endif
        return 0;
}
