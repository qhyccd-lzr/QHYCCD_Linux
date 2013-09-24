#include <stdlib.h>
#include <vector>

#include "camera.h"
#include "utils.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	base camera class
//
////////////////////////////////////////////////////////////////////////////////////////////////////
ccamera_base::ccamera_base()
{
	// init smth global....
	m_frame_break = false;

	//init buffer
	m_buffer.frame.width  = -1;
	m_buffer.frame.height = -1;
	m_buffer.frame.bpp	  = -1;
	m_buffer.data 		  = NULL;

	m_notify_wnd = NULL;
	m_initialized = false;
}


ccamera_base::~ccamera_base()
{
	if( m_buffer.data )
		free( m_buffer.data );
}


bool ccamera_base::start( void )
{
	int ret = 0;

	if( m_initialized )
		return true;

	ret = open_device();		// sets initialized var.
	if( ret != 0 )
	{
		log_e("ccamera_base::start: SHIT happened");
		return false;
	}


	// init thead staff
	pthread_mutex_init( &sync_mutex, NULL );
	pthread_mutex_init( &cv_mutex, NULL );
	pthread_cond_init( &cv, NULL );
	start_thread_flag 	= 0;
	quit_thread_flag 	= 0;
	data_thread_flag	= 0;
	m_async_task_in_progress = false;
	pthread_create( &cam_thread, NULL, &ccamera_base::camera_thread, this );

	m_initialized = true;

	return true;
}


void ccamera_base::stop( void )
{
	if( !m_initialized )
		return;

	stop_frame();

	// stop thread
	pthread_mutex_lock( &cv_mutex );
	start_thread_flag = 1;
	quit_thread_flag = 1;
	data_thread_flag = 1;
	pthread_cond_signal( &cv );
	pthread_mutex_unlock( &cv_mutex );

	pthread_join( cam_thread, NULL );

	// release stuff
	pthread_mutex_destroy( &sync_mutex );
	pthread_mutex_destroy( &cv_mutex );
	pthread_cond_destroy( &cv );


	close_device();	// sets initialized var.

	m_initialized = false;
}


void ccamera_base::stop_frame( void )
{
	m_frame_break = true;
}


bool ccamera_base::set_thread_task( const char *cmd, int cmd_len, char *param, int param_len, char *out, int out_len, bool is_blocked )
{
 bool ret = true;


 	 if( m_async_task_in_progress )
	 {
		 log_e("async task in progress...");
		 return false;
	 }


 	m_frame_break = false;

	pthread_mutex_lock( &sync_mutex );

	thr_task.cmd 		= cmd;
	thr_task.cmd_len 	= cmd_len;
	thr_task.param 		= param;
	thr_task.param_len 	= param_len;
	thr_task.out 		= out;
	thr_task.out_len 	= out_len;
	thr_task.is_blocked	= is_blocked;

	pthread_mutex_unlock( &sync_mutex );

	m_async_task_in_progress = !is_blocked;

	// start...
	log_i( m_async_task_in_progress ? "async task starting..." : "sync task starting...");
	pthread_mutex_lock( &cv_mutex );
	start_thread_flag = 1;
	data_thread_flag = 0;
	pthread_cond_signal( &cv );
	pthread_mutex_unlock( &cv_mutex );

	// wait for complete synchronous task...
	if( is_blocked )
	{
		pthread_mutex_lock( &cv_mutex );
		while( !data_thread_flag )
			pthread_cond_wait( &cv, &cv_mutex );  // infinite wait
		pthread_mutex_unlock( &cv_mutex );
		log_i("sync done...");

		// get result
		pthread_mutex_lock( &sync_mutex );
		ret = (thr_task.result == 0);
		pthread_mutex_unlock( &sync_mutex );
	}

 return ret;
}


void ccamera_base::continue_device( void )
{
	// signal synchronous
	pthread_mutex_lock( &cv_mutex );
	data_thread_flag = 1;
	pthread_cond_signal( &cv );
	pthread_mutex_unlock( &cv_mutex );

	// signal asynchronous
	m_async_task_in_progress = false;
	log_i("continued...");
}


void ccamera_base::set_notify_wnd( QObject * const pwnd )
{
     m_notify_wnd = pwnd;
}


bool ccamera_base::is_initialized( void ) const
{
 return m_initialized;
}


frame_params_t ccamera_base::get_buffer_frame( void )
{
 return m_buffer.frame;
}


bool ccamera_base::init_buffer( frame_params_t frame_params )
{
     if( frame_params.width <= 0 || frame_params.height <= 0 )
         return false;

     if( m_buffer.frame.width == frame_params.width && m_buffer.frame.height == frame_params.height && m_buffer.frame.bpp == frame_params.bpp )
         return true;

     if( m_buffer.data )
         free( m_buffer.data );

     m_buffer.frame = frame_params;

     m_buffer.data = malloc( m_buffer.frame.width * m_buffer.frame.height * ((m_buffer.frame.bpp+7) >> 3) );

 return m_buffer.data != NULL;
}


bool ccamera_base::exec_slow_ambiguous_synchronous_request( int req_num,
															const std::map< std::string, std::string > &params,
															std::map< std::string, std::string > *result )
{
	(void)req_num;
	(void)params;
	(void)result;
	return true;
}


std::vector<float> ccamera_base::get_debayer_settings( void ) const
{
	return std::vector<float>();
}


void *ccamera_base::camera_thread( void *param )
{
 ccamera_base *cam = (ccamera_base *)param;
 cam_task_t task_data;
 int quit = 0;


	 while( 1 )
	 {
		 // wait for new task...
		 pthread_mutex_lock( &cam->cv_mutex );
		 while( !cam->start_thread_flag )
			 pthread_cond_wait( &cam->cv, &cam->cv_mutex );  // infinite waiting
		 quit = cam->quit_thread_flag;
		 cam->start_thread_flag = 0;
		 pthread_mutex_unlock( &cam->cv_mutex );

		 // check for quit command
		 if( quit )
			 break;

		 log_i("New task started...");

		 // sync lock to access data
		 pthread_mutex_lock( &cam->sync_mutex );

		 task_data.cmd 			= cam->thr_task.cmd;
		 task_data.cmd_len 		= cam->thr_task.cmd_len;
		 task_data.param 		= cam->thr_task.param;
		 task_data.param_len 	= cam->thr_task.param_len;
		 task_data.out 			= cam->thr_task.out;
		 task_data.out_len 		= cam->thr_task.out_len;
		 task_data.is_blocked	= cam->thr_task.is_blocked;
		 task_data.result		= 0;

		 pthread_mutex_unlock( &cam->sync_mutex );

		 task_data.result = cam->do_command( &task_data );

		 // sync lock to set result
		 pthread_mutex_lock( &cam->sync_mutex );
		 cam->thr_task.result = task_data.result;
		 pthread_mutex_unlock( &cam->sync_mutex );

		 // synchronize data (sync mode)
		 if( task_data.is_blocked )
		 {
			 cam->continue_device();
		 }
		 else
		 {
			 // synchronize data (async mode)
			 // thread pauses at the beginning of next loop by 'start_thread_flag'
			 emit cam->receiveData( task_data.result, task_data.out, task_data.out_len );
		 }

		 log_i("Task complete");
	 }

	 log_i("camera thread finished");

 return NULL;
}


namespace camera_model
{
	const char *to_string( model m )
	{
		switch( m )
		{
		case null:
			return "null";
		case qhy5:
                        return "qhy5";
                case qhy5lii:
                        return "qhy5lii";
		case qhy6:
			return "qhy6";
		case ic8300:
			return "ic8300";
		case qhy8l:
			return "qhy8l";
		default:
			return "unknown";
		}
	}

	model from_string( const char *str )
	{
		if( !strcmp( str, "null" ) )
			return null;
                else if( !strcmp( str, "qhy5" ) )
			return qhy5;
                else if(!strcmp(str,"qhy5lii"))
                    return qhy5lii;
                else if( !strcmp( str, "qhy6" ) )
			return qhy6;
                else if( !strcmp( str, "ic8300" ) )
			return ic8300;
                else if( !strcmp( str, "qhy8l" ) )
			return qhy8l;
		else
			return unknown;
	}
}
