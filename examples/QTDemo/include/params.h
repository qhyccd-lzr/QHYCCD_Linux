/*
 * params.h
 *
 *  Created on: 05.05.2011
 *      Author: gm
 */

#ifndef PARAMS_H_
#define PARAMS_H_

#include <map>
#include <string>
#include "camera.h"

class params
{
public:
	params();
	virtual ~params();

	bool load( void );
	bool save( void );

	void set_camera_model( const camera_model::model &mdl )
	{
		m_model = mdl;
		m_params_map[ "camera_model" ] = std::string(camera_model::to_string(m_model));
	}
	camera_model::model get_camera_model( void ) const { return m_model; }

	void set_post_processing_enabled( bool mode )
	{
		m_post_processing_enabled = mode;
		m_params_map[ "post_processing_enabled" ] = m_post_processing_enabled ? "true" : "false";
	}
	bool get_post_processing_enabled( void ) const { return m_post_processing_enabled; }

	void set_debayer_enabled( bool mode )
	{
		m_debayer_enabled = mode;
		m_params_map[ "debayer_enabled" ] = m_debayer_enabled ? "true" : "false";
	}
	bool get_debayer_enabled( void ) const { return m_debayer_enabled; }

	void set_save_preview( bool mode )
	{
		m_save_preview = mode;
		m_params_map[ "save_preview" ] = m_save_preview ? "true" : "false";
	}
	bool get_save_preview( void ) const { return m_save_preview; }

	const std::map< std::string, std::string > & get_param_map_ref( void ) const { return m_params_map; }
private:
	camera_model::model m_model;
	bool m_post_processing_enabled;
	bool m_debayer_enabled;
	bool m_save_preview;

	std::map< std::string, std::string > m_params_map;
};

#endif /* PARAMS_H_ */
