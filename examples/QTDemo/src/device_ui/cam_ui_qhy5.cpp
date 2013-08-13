/*
 * cam_ui_qhy5.cpp
 *
 *  Created on: 20.05.2012
 *      Author: gm
 */

#include "cam_ui_qhy5.h"
#include "utils.h"


cam_ui_qhy5::cam_ui_qhy5( ccamera_base *pcam, QWidget *parent ) :
    ui_device_base(pcam, parent)
{
	ui.setupUi( this->ui_device_base::m_hw_ui );
	this->ui_device_base::m_hw_ui->setWindowTitle( tr("HW params") );

	m_params = static_cast<cam_qhy5_params *>(m_camera->alloc_params_object());

	connect( ui.OkButton, SIGNAL(clicked()), this, SLOT(onHwOkClick()) );

	fill_interface();
}


cam_ui_qhy5::~cam_ui_qhy5()
{
	delete m_params;
}


void cam_ui_qhy5::onHwOkClick()
{
	if( is_valid() )
		this->ui_device_base::m_hw_ui->close();

}


bool cam_ui_qhy5::is_valid( void )
{
 int gain;
 double exposure;
 int binning;
 bool ok;

 	// HW params must be validated first
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
	if( binning != 1 && binning != 2 )
	{
		u_msg( "Binning must be equal 1 or 2" );
		return false;
	}

	m_params->Exposition = exposure * 1000;
	m_params->Binning    = binning;

	m_params->gain	 = gain;

 return true;
}


bool cam_ui_qhy5::apply( void )
{
	if( !is_valid() )
		return false;

	// send params to device
	m_camera->set_params( m_params );
	m_camera->get_params( m_params );

	fill_interface();

 return true;
}


void cam_ui_qhy5::fill_interface( void )
{
	m_camera->get_params_copy( m_params );

	// fill common params
	this->ui_device_base::ui.ExpositionEdit->setText( QString().setNum( m_params->Exposition/1000, 'f', 3 ) );
	this->ui_device_base::ui.BinningEdit->setText( QString().setNum( m_params->Binning ) );

	// fill device specific params
	ui.GainEdit->setText( QString().setNum( m_params->gain ) );
}
