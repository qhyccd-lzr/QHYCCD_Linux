//---------------------------------------------------------------------------

#ifndef CameraH
#define CameraH
//---------------------------------------------------------------------------

#include <QObject>
#include <QEvent>
#include <QApplication>
#include <QSettings>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <libqhyccd/common.h>


namespace camera_model
{
	enum model
	{
                null = 1,
		qhy5,
                qhy5lii,
		qhy6,
		ic8300,
		qhy8l,
		unknown
	};

	const char *to_string( model m );
	model from_string( const char *str );
}


#define MK_CMD(x)	(char*)x, sizeof(x)-1


class cam_progress_Event : public QEvent
{
public:
	static const QEvent::Type camEvType;

	cam_progress_Event( unsigned long n ) : QEvent( camEvType ){ ticks = n; };
	~cam_progress_Event(){};

	unsigned long ticks;
};


// CCD high parameters
class cam_base_params
{
public:
	cam_base_params( int dt = 0 ) : download_time( dt ) {}
	virtual ~cam_base_params(){}
	cam_base_params &operator=( const cam_base_params &v )
	{
		if( this == &v )
			return *this;
		Exposition = v.Exposition;
		Binning = v.Binning;
		interframe_time = v.interframe_time;
		return *this;
	}
	virtual void reset()
	{
		Exposition = 1000; // 1000ms.
		Binning    = 1;
	}
	virtual void load( void ) = 0;
	virtual void save( void ) const = 0;

	// common params
	const  int download_time;
	double Exposition;	// msec
	int    Binning;
	int    interframe_time;	//msec

protected:
	virtual void load( QSettings * const sett )
	{
		if( !sett )
			return;

		// base params
		sett->beginGroup("base");
			Exposition      = (double)sett->value( "exposure", 1000 ).toUInt();
			Binning         = (int)sett->value( "binning", 1 ).toUInt();
			interframe_time = (int)sett->value( "interframe_time", 0 ).toUInt();
		sett->endGroup();
	}

	virtual void save( QSettings * const sett ) const
	{
		if( !sett )
			return;

		// base params
		sett->beginGroup("base");
			sett->setValue( "exposure", (unsigned)Exposition );
			sett->setValue( "binning", (unsigned)Binning );
			sett->setValue( "interframe_time", (unsigned)interframe_time );
		sett->endGroup();
	}
};


typedef struct
{
	int width, height, bpp;
}frame_params_t;


typedef struct
{
	frame_params_t frame;
	void *data;
}buffer_t;


typedef struct
{
	const char *cmd;
	int 		cmd_len;
	char 	   *param;		// command parameters.
							// for synchronous tasks may point to stack mem
							// for asynchronous tasks MUST point to heap mem!!!
	int 		param_len;
	char 	   *out;		// output data. accessible only from handler
							// for synchronous tasks may point to stack mem
							// for asynchronous tasks MUST point to heap mem!!!
	int 		out_len;
	bool 		is_blocked;	// false - means asynchronous exec. another thread must unlock cam-thread after eating result
							// only 1 task may be executed at one time, so any tasks would be rejected during async task
	int 		result;		// 0 - OK, other - error codes
}cam_task_t;


////////////////////////////////////////////////////////////////////
class ccamera_base : public QObject
{
	Q_OBJECT

public:
	enum error_codes	// old legacy error codes
	{
	     ERRCAM_OK = 0,
	     ERRCAM_SOCK_CONNECT,
	     ERRCAM_SOCK_START,
	     ERRCAM_SOCK_ALLOC,
	     ERRCAM_SOCK_WRONG_ADDRESS,
	     ERRCAM_SOCK_RECV,
	     ERRCAM_SOCK_SEND,
	     ERRCAM_NO_MEMORY,
	     ERRCAM_USER_BREAK,
	     ERRCAM_TIMEOUT
	};

	ccamera_base();
	virtual ~ccamera_base();

	virtual camera_model::model get_model( void ) const = 0;

	bool start( void );
	void stop( void );

	virtual void stop_frame( void );
	bool set_thread_task( const char *cmd, int cmd_len, char *param, int param_len, char *out, int out_len, bool is_blocked );
	void continue_device( void );

	virtual void set_notify_wnd( QObject * const pwnd );
	bool is_initialized( void ) const;

	virtual bool get_frame( void ) = 0;
	frame_params_t get_buffer_frame( void );
	virtual cam_base_params *alloc_params_object( void ) const = 0;			// returns default params. object
	virtual bool set_params( cam_base_params *params ) = 0;			// sends params into camera
	virtual bool get_params( cam_base_params *params ) = 0;			// returns params from cam.
	virtual bool get_params_copy( cam_base_params *params ) const = 0;	// returns last params without cam. access
	virtual const cam_base_params &get_params_ref( void ) const = 0;	// returns reference to params.
	// this function allow any camera module perform miscelaneous nonstandard requests such as temperature calls and etc.
	virtual bool exec_slow_ambiguous_synchronous_request( int req_num,
														const std::map< std::string, std::string > &params,
														std::map< std::string, std::string > *result );	// returns true if success
	virtual std::vector<float> get_debayer_settings( void ) const;

	static const unsigned int progress_time_limit = 100;	// no progress shows if exposure is shorter than this limit
signals:
	void receiveData( int ret, const void *ptr, int len );

protected:
	enum mics_consts
	{
		try_reset_cnt = 3
	};

	bool init_buffer( frame_params_t frame_params );
	virtual bool get_frame_params( frame_params_t *params ) = 0;	// returns frame params from cam.

	buffer_t m_buffer;	// final image buffer

	QObject *m_notify_wnd;
	bool m_initialized;  	// if device opened = true

	volatile bool m_frame_break;

private:
	// thread stuff
	pthread_t cam_thread;
	pthread_cond_t cv;			// sunc cond. var.
	pthread_mutex_t	cv_mutex;	// cond. var. mutex
	pthread_mutex_t	sync_mutex;	// cond. var. mutex
	int	start_thread_flag;		// start flag
	int	quit_thread_flag;		// quit flag
	int	data_thread_flag;		// data flag
	bool m_async_task_in_progress;
	cam_task_t thr_task;

	virtual int open_device( void ) = 0;
	virtual void close_device( void ) = 0;
	virtual int  do_command( cam_task_t *task ) = 0;

	static void *camera_thread( void *param );
};

#endif
