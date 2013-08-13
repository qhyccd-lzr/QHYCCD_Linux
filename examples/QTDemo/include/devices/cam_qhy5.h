/*
 * cam_qhy5.h
 *
 *  Created on: 24.05.2011
 *      Author: gm
 */

#ifndef CAM_QHY5_H_
#define CAM_QHY5_H_

#include "camera.h"


class qhy5_core_shared;


class cam_qhy5_params : public cam_base_params
{
public:
	cam_qhy5_params() : cam_base_params( 500 )
	{
		reset();
	}
	virtual ~cam_qhy5_params(){}
	virtual void reset()
	{
		cam_base_params::reset();

		gain = 10;
	}
	virtual void load( void )
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::load( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			gain = settings.value( "gain", 10 ).toInt();
		settings.endGroup();
	}
	virtual void save( void ) const
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::save( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			settings.setValue( "gain", gain );
		settings.endGroup();
	}
	// device-dependent
	int gain;

private:
	static const camera_model::model m_model = camera_model::qhy5;
};


typedef struct
{
	int exposure;
	int binn;
	int gain;
	int out_frame_width;
	int out_frame_height;
	int out_buffer_size;
}qhy5_params_t;


class ccamera_qhy5 : public ccamera_base
{
	Q_OBJECT
public:
	ccamera_qhy5();
	virtual ~ccamera_qhy5();

	virtual camera_model::model get_model( void ) const;

	// Operations
	virtual bool get_frame( void );
	virtual cam_base_params *alloc_params_object( void ) const;			// returns default params. object
	virtual bool set_params( cam_base_params *params );	// sends params into camera
	virtual bool get_params( cam_base_params *params );		// returns params from camera
	virtual bool get_params_copy( cam_base_params *params ) const; // returns last params without cam. access
	virtual const cam_base_params &get_params_ref( void ) const;	// returns reference to params.

protected:
	virtual bool get_frame_params( frame_params_t *params );

private:
	virtual int open_device( void );
	virtual void close_device( void );

	virtual int do_command( cam_task_t *task );

	cam_qhy5_params m_high_params;	// human-readable params
	qhy5_params_t   m_low_params;

	qhy5_core_shared *m_qhy5_obj;

};

#endif /* CAM_QHY5_H_ */
