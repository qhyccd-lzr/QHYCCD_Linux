#include "configwnd.h"
#include "utils.h"


configwnd::configwnd(params *param_ref, QWidget *parent)
    : QDialog(parent),
      m_params_ref( param_ref )
{
	ui.setupUi(this);

	setWindowTitle( tr("Configuration") );

	connect( ui.comboBox_CameraList, SIGNAL(activated(int)), this, SLOT(onCameraListChanged(int)) );
	connect( ui.checkBox_MinimalColorProcessing, SIGNAL(stateChanged(int)), this, SLOT(onPostprocessingEnabledChanged(int)) );

	connect( ui.pushButton_OK, SIGNAL(clicked()), this, SLOT(onOkButtonClick()) );
	connect( ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(onCancelButtonClick()) );

}


configwnd::~configwnd()
{
}


void configwnd::showEvent( QShowEvent * event )
{
	if( event->spontaneous() )
		return;

	if( !m_params_ref )
	{
		u_msg("configwnd::showEvent: param receiver int initialized");
		return;
	}

	fill_interface();
}


void configwnd::closeEvent ( QCloseEvent * /*event*/ )
{
}


void configwnd::hideEvent ( QHideEvent * event )
{
	if( event->spontaneous() )
		return;

	close();
}


void configwnd::onCameraListChanged( int )
{
	u_msg( "Restart program to change camera model." );
}


void configwnd::onPostprocessingEnabledChanged( int state )
{
	(void)state;
	ui.frame_Postprocessing->setEnabled( ui.checkBox_MinimalColorProcessing->isChecked() );
}


void configwnd::onOkButtonClick()
{
	if( ui.comboBox_CameraList->currentIndex() == -1 )
	{
		u_msg("Camera model is not selected");
		return;
	}
	if( camera_model::from_string( ui.comboBox_CameraList->currentText().toAscii().data() ) == camera_model::unknown )
	{
		u_msg("Camera model is strange and inacceptable. Internal error.");
		return;
	}

	if( m_params_ref )
	{
		m_params_ref->set_camera_model( camera_model::from_string( ui.comboBox_CameraList->currentText().toAscii().data() ) );
		m_params_ref->set_post_processing_enabled( ui.checkBox_MinimalColorProcessing->isChecked() );
		m_params_ref->set_debayer_enabled( ui.checkBox_Debayerization->isChecked() );
		m_params_ref->set_save_preview( ui.checkBox_SavePreview->isChecked() );
	}

	close();
}


void configwnd::onCancelButtonClick()
{
	close();
}


void configwnd::fill_interface( void )
{
 int cur_cam = -1;


	ui.comboBox_CameraList->clear();

        for( int i = (int)camera_model::null;i != (int)camera_model::unknown;i++ )
	{
		ui.comboBox_CameraList->addItem( QString( camera_model::to_string( (camera_model::model)i  ) ) );
		if( (camera_model::model)i == m_params_ref->get_camera_model() )
                    cur_cam = i - (int)camera_model::null;
	}
	ui.comboBox_CameraList->setCurrentIndex( cur_cam );

	ui.checkBox_MinimalColorProcessing->setChecked( m_params_ref->get_post_processing_enabled() );
	ui.checkBox_Debayerization->setChecked( m_params_ref->get_debayer_enabled() );
	ui.frame_Postprocessing->setEnabled( m_params_ref->get_post_processing_enabled() );
	ui.checkBox_SavePreview->setChecked( m_params_ref->get_save_preview() );
}
