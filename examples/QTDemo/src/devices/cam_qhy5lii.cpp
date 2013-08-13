/*
 * video_qhy5lii.cpp
 *
 *  Created on: 24.05.2011
 *      Author: gm
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <netinet/in.h>
#include <unistd.h>

#include "camera.h"
#include "cam_qhy5lii.h"
#include "timer.h"
#include "utils.h"
#include "image.h"
#include "qhy5lii_core.h"


// enables test image
//#define qhy5lii_TEST_IMAGE

//-------------------------------------------- qhy5lii ---------------------------------------------------------
// qhy5lii stuff
ccamera_qhy5lii::ccamera_qhy5lii() : m_qhy5lii_obj( NULL )
{
        m_qhy5lii_obj = new qhy5lii_core_shared();

        // load params
        m_high_params.load();
}


ccamera_qhy5lii::~ccamera_qhy5lii()
{
        //MUST BE called the first: stop thread and release resources
        stop();

        if( m_qhy5lii_obj )
                delete m_qhy5lii_obj;

        // save params
        m_high_params.save();
}


camera_model::model ccamera_qhy5lii::get_model( void ) const
{
        return camera_model::qhy5lii;
}


int ccamera_qhy5lii::open_device( void )
{
        int ret = EXIT_FAILURE;

        if( m_initialized )
                return 0;

        ret = m_qhy5lii_obj->open_device();

        if( ret == EXIT_SUCCESS )
        {
                qhy5lii_params_t p;
                p.exposure	= (int)m_high_params.Exposition;
                p.binn		= m_high_params.Binning;
                p.gain		= m_high_params.gain;
                p.out_frame_width = 0;
                p.out_frame_height = 0;

                ret = m_qhy5lii_obj->set_params( p.exposure,
                                p.binn,
                                p.gain,
                                1, // first_time init
                                &p.out_frame_width,
                                &p.out_frame_height,
                                NULL );
                if( ret == EXIT_SUCCESS )
                {
                        m_low_params  = p;
                        m_initialized = true;
                        log_i( "Connected to qhy5lii camera" );
                }
                else
                        close_device();
        }

        return ret;
 return m_qhy5lii_obj->open_device();
}


void ccamera_qhy5lii::close_device( void )
{
        m_qhy5lii_obj->close_device();

        m_initialized = false;
        log_i( "Disconnected" );
}


cam_base_params *ccamera_qhy5lii::alloc_params_object( void ) const
{
        return new cam_qhy5lii_params();
}


bool ccamera_qhy5lii::set_params( cam_base_params *params )
{
 cam_qhy5lii_params *h_params = NULL;
 qhy5lii_params_t p;
 bool ret = false;


        h_params = dynamic_cast<cam_qhy5lii_params *>(params);

        if( !m_initialized || !h_params )
                return false;

        p.exposure = (int)h_params->Exposition;
        p.binn     = h_params->Binning;
        p.gain	   = h_params->gain;
        p.out_frame_width  = 0;
        p.out_frame_height = 0;

        // goto thread
        ret = set_thread_task( MK_CMD("@SETPAR"), (char*)&p, sizeof(qhy5lii_params_t), NULL, 0, true );
        if( !ret )
        {
                log_e("ccamera_qhy5lii::set_params: set_thread_task error");
                return false;
        }

        m_high_params = *h_params;
        m_low_params  = p;

 return true;
}


bool ccamera_qhy5lii::get_params( cam_base_params *params )
{
 cam_qhy5lii_params *dst = dynamic_cast<cam_qhy5lii_params *>(params);
 bool ret = false;

        if( !m_initialized || !dst )
                return false;

        dst->reset();

        // goto thread
        ret = set_thread_task( MK_CMD("@GETPAR"), NULL, 0, (char*)&m_low_params, sizeof(qhy5lii_params_t), true );
        if( !ret )
        {
                log_e("ccamera_qhy5lii::get_params: set_thread_task error");
                return false;
        }

        m_high_params.Exposition 	= (double)m_low_params.exposure;
        m_high_params.Binning    	= m_low_params.binn;
        m_high_params.gain		 	= m_low_params.gain;

        *dst = m_high_params;

 return true;
}


bool ccamera_qhy5lii::get_params_copy( cam_base_params *params ) const
{
 cam_qhy5lii_params *dst = dynamic_cast<cam_qhy5lii_params *>(params);

        if( !m_initialized || !dst )
                return false;

        *dst = m_high_params;

 return true;
}


const cam_base_params &ccamera_qhy5lii::get_params_ref( void ) const
{
 return m_high_params;
}


bool ccamera_qhy5lii::get_frame( void )
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
                log_e("ccamera_qhy5lii::get_frame: set_thread_task error");
                return false;
        }

 return true;
}


bool ccamera_qhy5lii::get_frame_params( frame_params_t *params )
{
 frame_params_t frame_params;


         if( params != NULL )
                 memset( params, 0, sizeof(frame_params_t) );

         // for qhy5lii buffer is larger than wd*ht*pixsize
         assert( m_low_params.out_frame_width*m_low_params.out_frame_height*sizeof(uint8_t) <= (size_t)m_low_params.out_buffer_size );

         frame_params.width 	= m_low_params.out_frame_width;
         frame_params.height	= m_low_params.out_frame_height;
         frame_params.bpp   	= pixel_base::BPP_MONO_8;

        *params = frame_params;

 return true;
}


int ccamera_qhy5lii::do_command( cam_task_t *task )
{
        qhy5lii_params_t *p = NULL;
        ctimer progress_timer;
        int nsec = 0;
        unsigned long now;
        int ret = EXIT_SUCCESS;
        bool break_done = false;

        if( strncmp( task->cmd, "@SETPAR", task->cmd_len ) == 0 )
        {
                if( task->param_len != sizeof(qhy5lii_params_t) || !(p = reinterpret_cast<qhy5lii_params_t *>(task->param)) )
                {
                        log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
                        return 1;
                }

                int res = m_qhy5lii_obj->set_params( p->exposure, p->binn, p->gain, 0, &p->out_frame_width, &p->out_frame_height, &p->out_buffer_size );
                if( res != EXIT_SUCCESS )
                {
                        log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s failed.", task->cmd_len, task->cmd );
                        return 2;
                }

                log_i( "Config set_params OK" );
        }
        else
        if( strncmp( task->cmd, "@GETPAR", task->cmd_len ) == 0 )
        {
                if( task->out_len != sizeof(qhy5lii_params_t) || !(p = reinterpret_cast<qhy5lii_params_t *>(task->out)) )
                {
                        log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
                        return 3;
                }

                qhy5lii_params_t lp;
                lp = m_low_params;		// emulation of getting parameters (already got by @SETPAR)

                memcpy( task->out, &lp, sizeof(qhy5lii_params_t) );

                log_i( "Config get_params OK" );
        }
        else
        if( strncmp( task->cmd, "@IMG", task->cmd_len ) == 0 )
        {
                uint8_t *raw = NULL;
                if( task->out_len != m_low_params.out_frame_width*m_low_params.out_frame_height*1 || !(raw = reinterpret_cast<uint8_t *>(task->out)) )
                {
                        log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s has inconsistent params.", task->cmd_len, task->cmd );
                        return 4;
                }

                log_i( "Start capture..." );

                int res = m_qhy5lii_obj->start_exposure( m_low_params.exposure );
                if( res != EXIT_SUCCESS )
                {
                        log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s failed. Unable to start exposure", task->cmd_len, task->cmd );
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
                                int res = m_qhy5lii_obj->reset_camera(); //system( "utils/qhy5liireset.sh" );
                                if( res != EXIT_SUCCESS )
                                {
                                        log_e( "Failed to reset camera... '%s'", strerror(errno) );
                                }
                                else
                                {
                                        ret = m_qhy5lii_obj->set_params( m_low_params.exposure,
                                                        m_low_params.binn,
                                                        m_low_params.gain,
                                                        0,
                                                        NULL,
                                                        NULL,
                                                        NULL );
                                        if( res != EXIT_SUCCESS )
                                        {
                                                log_e( "ccamera_qhy5lii::do_command(): set_params() failed after reset" );
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
                                log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s failed. Unable to allocate raw data buffer", task->cmd_len, task->cmd );
                                return 6;
                        }
                        int res = m_qhy5lii_obj->read_exposure( raw_buffer, m_low_params.out_buffer_size );
                        if( res != EXIT_SUCCESS )
                        {
                                free( raw_buffer );
                                log_e( "ccamera_qhy5lii::do_command(): CMD = %.*s failed.", task->cmd_len, task->cmd );
                                return 7;
                        }

                        // convert to target buffer format
                        // decode
                        uint8_t *dst = raw;
                        uint8_t *src = raw_buffer;
                        uint8_t *d_ptr = dst;	// writer
                        unsigned int row_end, col_start;
                        unsigned int x_offset = 0; // full frame

                        row_end = QHY5LII_HEIGHT_B1;
                        col_start = x_offset + 20;

#ifdef qhy5lii_TEST_IMAGE
                        memset( raw_buffer, 32, m_low_params.out_buffer_size );
                        for( unsigned int row = 0; row < row_end; row++ )
                        {
                                for( unsigned int col = col_start; col < qhy5lii_WIDTH_B1 + col_start; col++ )
                                {
                                        if( col-20 == row )
                                                src[row * qhy5lii_MATRIX_WIDTH + col] = 128+rand()%32;
                                }
                        }
#endif

                        // copy
                        unsigned int col_end = QHY5LII_WIDTH_B1 + col_start;

                        switch( m_high_params.Binning )
                        {
                        case 1:
                                for( unsigned int row = 0; row < row_end; row++ )
                                {
                                        for( unsigned int col = col_start; col < col_end; col++ )
                                        {
                                                *d_ptr = src[row * QHY5LII_MATRIX_WIDTH + col];
                                                d_ptr++;
                                        }
                                }
                                break;
                        case 2:
                                for( unsigned int row = 0; row < row_end; row+=2 )
                                {
                                        for( unsigned int col = col_start; col < col_end; col+=2 )
                                        {
                                                unsigned int off = row * QHY5LII_MATRIX_WIDTH + col;
                                                unsigned int val = src[off] + src[off+1] + src[off+QHY5LII_MATRIX_WIDTH] + src[off+QHY5LII_MATRIX_WIDTH+1];
                                                *d_ptr = val < 255 ? (uint8_t)val : 255;
                                                d_ptr++;
                                        }
                                }
                                break;
                        default:
                                log_e( "ccamera_qhy5lii::do_command(): Unsupported binning = %d", m_high_params.Binning );
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

