/*
 * ui_abstract.cpp
 *
 *  Created on: 19.04.2011
 *      Author: gm
 */
#include "ui_device_base.h"


ui_device_base::ui_device_base( ccamera_base *p, QWidget *parent ) :
	QDialog( parent ),
	m_camera( p ),
	m_hw_ui( NULL )
{
	ui.setupUi(this);

	setWindowTitle( tr("Params") );

	m_hw_ui = new ui_hw_dialog_wrapper( this );

	connect( ui.HardwareParamsButton, SIGNAL( clicked() ), this, SLOT(onHardwareParamsClick()) );
	connect( ui.ApplyParamsButton, SIGNAL( clicked() ), this, SLOT(onApplyParamsClick()) );
}


ui_device_base::~ui_device_base()
{
	if( m_hw_ui )
	{
		delete m_hw_ui;
		m_hw_ui = NULL;
	}
}


void ui_device_base::showEvent ( QShowEvent * event )
{
	if( event->spontaneous() )
		return;

	fill_interface();
}


void ui_device_base::onHardwareParamsClick()
{
	m_hw_ui->exec();
}


void ui_device_base::onApplyParamsClick()
{
	apply();
}


//-----------------------------------------------------------------------------
ui_hw_dialog_wrapper::ui_hw_dialog_wrapper( QWidget *parent ) :
		QDialog( parent )
{
}


ui_hw_dialog_wrapper::~ui_hw_dialog_wrapper()
{
}


void ui_hw_dialog_wrapper::showEvent ( QShowEvent * ev )
{
	if( ev->spontaneous() )
		return;

	emit onShowSignal();
}


void ui_hw_dialog_wrapper::closeEvent( QCloseEvent * ev )
{
	(void)ev;

	emit onCloseSignal();
}


void ui_hw_dialog_wrapper::hideEvent( QHideEvent * ev )
{
	if( ev->spontaneous() )
		return;

	close();
}
