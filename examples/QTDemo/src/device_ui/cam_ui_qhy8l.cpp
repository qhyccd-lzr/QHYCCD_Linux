#include "cam_ui_qhy8l.h"
#include "utils.h"


cam_ui_qhy8l::cam_ui_qhy8l( ccamera_base *pcam, QWidget *parent ) :
	ui_device_base(pcam, parent)
{
	ui.setupUi( this->ui_device_base::m_hw_ui );
	this->ui_device_base::m_hw_ui->setWindowTitle( tr("HW params") );

	m_params = static_cast<cam_qhy8l_params *>(m_camera->alloc_params_object());

	m_timer.setInterval( 3000 );
	m_timer.stop();

	connect( ui.OkButton, SIGNAL(clicked()), this, SLOT(onHwOkClick()) );
	connect( &m_timer, SIGNAL(timeout()), this, SLOT(onTimer()) );

	connect( this->ui_device_base::m_hw_ui, SIGNAL(onShowSignal()), this, SLOT(onShow()) );
	connect( this->ui_device_base::m_hw_ui, SIGNAL(onCloseSignal()), this, SLOT(onClose()) );

	fill_interface();
}


cam_ui_qhy8l::~cam_ui_qhy8l()
{
	delete m_params;
}


void cam_ui_qhy8l::onHwOkClick()
{
	if( is_valid() )
		this->ui_device_base::m_hw_ui->close();
}


bool cam_ui_qhy8l::is_valid( void )
{
 int offset, gain;
 double exposure;
 int binning;
 int pwm;
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
 	pwm = ui.lineEdit_TEC->text().toInt( &ok, 10 );
 	if( !ok )
 	{
 		u_msg( "TEC power is not a digit" );
 		return false;
 	}
 	if( pwm < 0 || pwm > 100 )
 	{
 		u_msg( "TEC power be from 0% to 100%" );
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
	m_params->pwm    = pwm * 2.55;
	m_params->readout_speed = ui.checkBox_FastSpeed->isChecked() ? 1 : 0;

 return true;
}


bool cam_ui_qhy8l::apply( void )
{
	if( !is_valid() )
		return false;

	// send params to device
	m_camera->set_params( m_params );
	m_camera->get_params( m_params );

	fill_interface();

 return true;
}


void cam_ui_qhy8l::fill_interface( void )
{
	m_camera->get_params_copy( m_params );

	// fill common params
	this->ui_device_base::ui.ExpositionEdit->setText( QString().setNum( m_params->Exposition/1000, 'f', 3 ) );
	this->ui_device_base::ui.BinningEdit->setText( QString().setNum( m_params->Binning ) );

	// fill device specific params
	ui.OffsetEdit->setText( QString().setNum( m_params->offset ) );
	ui.GainEdit->setText( QString().setNum( m_params->gain ) );
	ui.lineEdit_TEC->setText( QString().setNum( ((double)m_params->pwm/2.55), 'f', 0 ) );
	ui.checkBox_FastSpeed->setChecked( m_params->readout_speed == 1 );
}


void cam_ui_qhy8l::onTimer()
{
	bool ret = 0;
	std::map< std::string, std::string > req_result;

	ret = m_camera->exec_slow_ambiguous_synchronous_request( ccamera_qhy8l::temp_voltage_req,
															std::map< std::string, std::string >(),
															&req_result );
	if( ret )
	{
		std::map< std::string, std::string >::const_iterator it = req_result.find( "temperature" );
		if( it != req_result.end() )
		{
			char buf[64];
			double temp = strtod( it->second.c_str(), NULL );
			snprintf( buf, sizeof(buf)-1, "%.1f", temp );
			ui.label_Temperature->setText( QString(buf) );
		}
		it = req_result.find( "voltage" );
		if( it != req_result.end() )
		{
			char buf[64];
			double temp = strtod( it->second.c_str(), NULL );
			snprintf( buf, sizeof(buf)-1, "%.1f", temp );
			//ui.label_Temperature->setText( QString(buf) );
		}
	}
}


void cam_ui_qhy8l::onShow()
{
	m_timer.start();
}


void cam_ui_qhy8l::onClose()
{
	m_timer.stop();
}
