/*
 * cam_null.h
 *
 *  Created on: 10.10.2011
 *      Author: gm
 */

#ifndef CAM_NULL_H_
#define CAM_NULL_H_

#include "camera.h"


class cam_null_params : public cam_base_params
{
public:
	cam_null_params()
	{
		reset();
	}
	virtual ~cam_null_params(){}
	virtual void reset()
	{
		cam_base_params::reset();
	}
	virtual void load( void )
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::load( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			//expo = settings.value( "exposure", 1000 ).toUInt();
		settings.endGroup();
	}
	virtual void save( void ) const
	{
		QSettings settings( "GM_software", QString("ccd_cam_") + camera_model::to_string( m_model ) );

		this->cam_base_params::save( &settings );

		// camera dependent params
		settings.beginGroup("hw_dependent");
			//settings.setValue( "exposure", (unsigned)Exposition );
		settings.endGroup();
	}
	// device-dependent

private:
	static const camera_model::model m_model = camera_model::null;
};


class ccamera_null : public ccamera_base
{
	Q_OBJECT

public:
	ccamera_null();
	virtual ~ccamera_null();

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

	cam_null_params m_high_params;	// human-readable params
};

#endif /* CAM_NULL_H_ */
