/*
 * cam_ui_qhy6.cpp
 *
 *  Created on: 22.11.2011
 *      Author: gm
 */

#include "cam_ui_qhy6.h"
#include "utils.h"


cam_ui_qhy6::cam_ui_qhy6( ccamera_base *pcam, QWidget *parent ) :
    ui_device_base(pcam, parent)
{
	ui.setupUi( this->ui_device_base::m_hw_ui );
	this->ui_device_base::m_hw_ui->setWindowTitle( tr("HW params") );

	m_params = static_cast<cam_qhy6_params *>(m_camera->alloc_params_object());

	connect( ui.OkButton, SIGNAL(clicked()), this, SLOT(onHwOkClick()) );

	fill_interface();
}


cam_ui_qhy6::~cam_ui_qhy6()
{
	delete m_params;
}


void cam_ui_qhy6::onHwOkClick()
{
	if( is_valid() )
		this->ui_device_base::m_hw_ui->close();

}


bool cam_ui_qhy6::is_valid( void )
{
 int offset, gain;
 double exposure;
 int binning;
 bool ok;

 	// HW params must be validated first
 	//----------------------------------------
  	offset = ui.OffsetEdit->text().toInt( &ok, 10 );
  	if( !ok )
  	{
  		u_msg( "Offset is not a digit" );
  		return false;
  	}
  	if( offset < 0 || offset > 255 )
 	{
 		u_msg( "Offset must be from 0 to 255" );
 		return false;
 	}

  	//----------------------------------------
 	gain = ui.GainEdit->text().toInt( &ok, 10 );
 	if( !ok )
 	{
 		u_msg( "Gain is not a digit" );
 		return false;
 	}
 	if( gain < 0 || gain > 100 )
 	{
 		u_msg( "Gain must be from 0 to 100" );
 		return false;
 	}

 	//----------------------------------------
 	exposure = this->ui_device_base::ui.ExpositionEdit->text().toDouble( &ok );

	if( !ok )
	{
		u_msg( "Exposure is not a digit" );//
		return false;
	}
	if( exposure < 0.0001 || exposure > 3600 )
	{
		u_msg( "Exposition must be from 0.0001 to 3600sec" );
		return false;
	}

	//----------------------------------------
	binning = this->ui_device_base::ui.BinningEdit->text().toInt( &ok, 10 );

	if( !ok )
	{
		u_msg( "Binning is not a digit" );
		return false;
	}

	m_params->Exposition = exposure * 1000;
	m_params->Binning    = binning;

	m_params->offset = offset;
	m_params->gain	 = gain;

 return true;
}


bool cam_ui_qhy6::apply( void )
{
	if( !is_valid() )
		return false;

	// send params to device
	m_camera->set_params( m_params );
	m_camera->get_params( m_params );

	fill_interface();

 return true;
}


void cam_ui_qhy6::fill_interface( void )
{
	m_camera->get_params_copy( m_params );

	// fill common params
	this->ui_device_base::ui.ExpositionEdit->setText( QString().setNum( m_params->Exposition/1000, 'f', 3 ) );
	this->ui_device_base::ui.BinningEdit->setText( QString().setNum( m_params->Binning ) );

	// fill device specific params
	ui.OffsetEdit->setText( QString().setNum( m_params->offset ) );
	ui.GainEdit->setText( QString().setNum( m_params->gain ) );
}
