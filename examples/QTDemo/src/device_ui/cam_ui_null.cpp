/*
 * cam_ui_null.cpp
 *
 *  Created on: 01.05.2011
 *      Author: gm
 */

#include "cam_ui_null.h"
#include "utils.h"


cam_ui_null::cam_ui_null( ccamera_base *pcam, QWidget *parent ) :
    ui_device_base(pcam, parent)
{
	ui.setupUi( this->ui_device_base::m_hw_ui );
	this->ui_device_base::m_hw_ui->setWindowTitle( tr("HW params") );

	m_params = static_cast<cam_null_params *>(m_camera->alloc_params_object());

	connect( ui.OkButton, SIGNAL(clicked()), this, SLOT(onHwOkClick()) );

	fill_interface();
}


cam_ui_null::~cam_ui_null()
{
	delete m_params;
}


void cam_ui_null::onHwOkClick()
{
	if( is_valid() )
		this->ui_device_base::m_hw_ui->close();

}


bool cam_ui_null::is_valid( void )
{
 double Exposition;
 int Binning;
 bool ok;

 	// HW params must be validated first

 	//----------------------------------------
	Exposition = this->ui_device_base::ui.ExpositionEdit->text().toDouble( &ok );

	if( !ok )
	{
		u_msg( "Exposition is not a digit" );//
		return false;
	}
	if( Exposition < 0.0001 || Exposition > 3600 )
	{
		u_msg( "Exposition must be from 0.0001 to 3600sec" );
		return false;
	}

	//----------------------------------------
	Binning = this->ui_device_base::ui.BinningEdit->text().toInt( &ok, 10 );

	if( !ok )
	{
		u_msg( "Binning is not a digit" );
		return false;
	}

	m_params->Exposition = Exposition * 1000;
	m_params->Binning    = Binning;

 return true;
}


bool cam_ui_null::apply( void )
{
	if( !is_valid() )
		return false;

	// send params to device
	m_camera->set_params( m_params );
	m_camera->get_params( m_params );

	fill_interface();

 return true;
}


void cam_ui_null::fill_interface( void )
{
	m_camera->get_params_copy( m_params );

	// fill common params
	this->ui_device_base::ui.ExpositionEdit->setText( QString().setNum( m_params->Exposition/1000, 'f', 3 ) );
	this->ui_device_base::ui.BinningEdit->setText( QString().setNum( m_params->Binning ) );

	// fill device specific params
	//ui.OffsetEdit->setText( QString().setNum( m_params->offset ) );
	//ui.GainEdit->setText( QString().setNum( m_params->gain ) );
}
