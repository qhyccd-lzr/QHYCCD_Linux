/*
 * params.cpp
 *
 *  Created on: 05.05.2011
 *      Author: gm
 */

#include <QSettings>
#include <stdio.h>
#include <unistd.h>

#include "params.h"
#include "utils.h"


params::params() :
	m_model( camera_model::null ),
	m_post_processing_enabled( false ),
	m_debayer_enabled( false )
{
}

params::~params()
{
}


bool params::load( void )
{
 QSettings settings( "GM_software", "ccd" );
 QString str;

	 if( access(settings.fileName().toUtf8().data(), R_OK|W_OK) != 0 )
	 {
		 log_e( "Unable to find config file %s\nusing defaults", settings.fileName().toUtf8().data() );
		 return false;
	 }

	 // main params
	 settings.beginGroup("main");
	 	 str = settings.value( "camera_model", QString( camera_model::to_string( m_model ) ) ).toString();
	 	 if( m_model != camera_model::unknown )
	 		 m_model = camera_model::from_string( str.toAscii().data() );
	 	 else
	 	 {
	 		 m_model = camera_model::null;
	 		 log_e("params::load(): Found unsupported camera mode '%s'. Falling to default model '%s'", str.toAscii().data(), camera_model::to_string( m_model ) );
	 	 }
	 	 m_params_map[ "camera_model" ] = std::string(camera_model::to_string(m_model));

	 	 m_post_processing_enabled = settings.value( "post_processing_enabled", false ).toBool();
	 	 m_params_map[ "post_processing_enabled" ] = m_post_processing_enabled ? "true" : "false";

	 	 m_debayer_enabled = settings.value( "debayer_enabled", false ).toBool();
	 	 m_params_map[ "debayer_enabled" ] = m_debayer_enabled ? "true" : "false";

	 	 m_save_preview = settings.value( "save_preview", true ).toBool();
	 	 m_params_map[ "save_preview" ] = m_save_preview ? "true" : "false";
	 settings.endGroup();

 return true;
}


bool params::save( void )
{
 QSettings settings("GM_software", "ccd");

 	// main params
 	settings.beginGroup("main");
 		settings.setValue( "camera_model", QString( camera_model::to_string( m_model ) ) );
 		settings.setValue( "post_processing_enabled", m_post_processing_enabled );
 		settings.setValue( "debayer_enabled", m_debayer_enabled );
 		settings.setValue( "save_preview", m_save_preview );
 	settings.endGroup();

 return true;
}
