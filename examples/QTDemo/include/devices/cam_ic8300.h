/*
 * cam_qhy9.h
 *
 *  Created on: 29.11.2011
 *      Author: gm
 */

#ifndef CAM_QHY9_H_
#define CAM_QHY9_H_

#include <libusb-1.0/libusb.h>

#include "camera.h"


#define QHY9_VENDOR_ID   0x1618
#define QHY9_PRODUCT_ID  0x8311

#define QHY9_MATRIX_WIDTH   3584
#define QHY9_MATRIX_HEIGHT  2574




#define QHY9_WIDTH_B1	QHY9_MATRIX_WIDTH
#define QHY9_HEIGHT_B1	QHY9_MATRIX_HEIGHT

#define QHY9_WIDTH_B2	1792
#define QHY9_HEIGHT_B2	1287


#define QHY9_WIDTH_B3	1196
#define QHY9_HEIGHT_B3	858


#define QHY9_WIDTH_B4	896
#define QHY9_HEIGHT_B4	644

#define QHY9_BUFFER_LEN 9360000 // 3600*2600

#define QHY9_BINN_CNT 4



class cam_qhy9_params : public cam_base_params
{
public:
	cam_qhy9_params() : cam_base_params( 1500 )
	{
		reset();
	}
	virtual ~cam_qhy9_params(){}
	virtual void reset()
	{
		cam_base_params::reset();

		gain = 50;
		offset = 115;
		readout_speed = 0;
		amp = 2;
		shutter = 0;
		pwm = 125;
		fan = 1;
	}
	virtual void load( void )
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::load( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			gain = settings.value( "gain", 50 ).toInt();
			offset = settings.value( "offset", 115 ).toInt();
			readout_speed = settings.value( "readout_speed", 0 ).toInt();
			amp = settings.value( "amp", 2 ).toInt();
			shutter = settings.value( "shutter", 0 ).toInt();
			pwm = settings.value( "pwm", 125 ).toInt();
			fan = settings.value( "fan", 1 ).toInt();
		settings.endGroup();
	}
	virtual void save( void ) const
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::save( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			settings.setValue( "gain", gain );
			settings.setValue( "offset", offset );
			settings.setValue( "readout_speed", readout_speed );
			settings.setValue( "amp", amp );
			settings.setValue( "shutter", shutter );
			settings.setValue( "pwm", pwm );
			settings.setValue( "fan", fan );
		settings.endGroup();
	}
	// device-dependent
	int offset;
	int gain;
	int readout_speed;
	int amp;
	int shutter;
	int pwm;
	int fan;

private:
	static const camera_model::model m_model = camera_model::qhy9;
};


typedef struct
{
	int exposure;
	int binn;
	int gain;
	int offset;
	int speed;
	int amp;
	int shutter;
	int pwm;
	int fan;
	int out_frame_width;
	int out_frame_height;
	int out_buffer_size;
}qhy9_params_t;


class ccamera_qhy9 : public ccamera_base
{
	Q_OBJECT
public:
	ccamera_qhy9();
	virtual ~ccamera_qhy9();

	virtual camera_model::model get_model( void ) const;

	// Operations
	virtual bool get_frame( void );
	virtual cam_base_params *alloc_params_object( void ) const;			// returns default params. object
	virtual bool set_params( cam_base_params *params );	// sends params into camera
	virtual bool get_params( cam_base_params *params );		// returns params from camera
	virtual bool get_params_copy( cam_base_params *params ) const; // returns last params without cam. access
	virtual const cam_base_params &get_params_ref( void ) const;	// returns reference to params.
	virtual bool exec_slow_ambiguous_synchronous_request( int req_num,
													const std::map< std::string, std::string > &params,
													std::map< std::string, std::string > *result );
	enum ASR_numbers	//ambiguous synchronous request
	{
		temp_req = 1
	};

protected:
	virtual bool get_frame_params( frame_params_t *params );

private:
	struct info_s
	{
		double temperature;
		int voltage;
	};

	virtual int open_device( void );
	virtual void close_device( void );

	virtual int do_command( cam_task_t *task );

	libusb_device_handle *locate_device( unsigned int vid, unsigned int pid );
	int ctrl_msg( int request_type, int request, unsigned int value, unsigned int index, unsigned char *data, int len );

	int set_params( int exposuretime, int binn, int gain, int offset, int speed, int amp, int shutter, int pwm, int *out_width, int *out_height, int *out_buffer_size );
	int get_info( struct info_s *info_out, bool dumpp = true );

	unsigned char MSB( unsigned int i );
	unsigned char LSB( unsigned int i );

	int shutter( int cmd );
	int get_dc201( short *dc );
	double r_to_degree( double R ) const;
	double mv_to_degree( double v ) const;
	int set_dc201( int pwm );

	cam_qhy9_params m_high_params;	// human-readable params
	qhy9_params_t   m_low_params;

	struct libusb_device_handle *m_handle;
	// Magic numbers have been researched by Vladimir Volynsky
	static const int m_binn_loading_time[];
	static const int m_binn_loading_size[];
};


#endif /* CAM_QHY9_H_ */
