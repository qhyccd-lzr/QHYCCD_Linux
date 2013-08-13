#include <Qt>
#include "ccd.h"
#include "photovideownd.h"
#include "utils.h"


const QEvent::Type cam_progress_Event::camEvType = (QEvent::Type)QEvent::registerEventType();



photovideownd::photovideownd(ccd *parent)
    : QDialog(parent), pmain_wnd(parent)
{

	ui.setupUi(this);

	//
	ui.StopButton->setEnabled( false );

	connect( ui.checkBox_IsSession, SIGNAL( stateChanged(int) ), this, SLOT(onSessionStateChange(int)) );
	connect( ui.StartButton, SIGNAL( clicked() ), this, SLOT(onStartButtonClick()) );
	connect( ui.StopButton, SIGNAL( clicked() ), this, SLOT(onStopButtonClick()) );

	//--------------
	mode = ccd::CCD_PHOTO_MODE;
	is_sess = false;

}

photovideownd::~photovideownd()
{
}


bool photovideownd::event(QEvent *ev)
{
	if( ev->type() == cam_progress_Event::camEvType )
	{
		cameraEvent( static_cast<cam_progress_Event *>(ev) );
		return true;
	}

 return QWidget::event( ev );
}


void photovideownd::showEvent( QShowEvent * ev )
{
	if( ev->spontaneous() )
		return;

	switch( mode )
	{
	 case ccd::CCD_VIDEO_MODE:
	      setWindowTitle( tr("Video mode") );
	      ui.checkBox_IsSession->setEnabled( false );
	  break;
	 case ccd::CCD_GUIDE_MODE:
	  	  setWindowTitle( tr("Guide mode") );
	  break;
	 case ccd::CCD_PHOTO_MODE:
	  	  setWindowTitle( tr("Photo mode") );
	  	ui.checkBox_IsSession->setEnabled( true );
	  break;
	}

	if( get_mode() != ccd::CCD_PHOTO_MODE )
	    pmain_wnd->device_wnd->apply();

	ui.checkBox_IsSession->setChecked( false );
	ui.ProgressBar->setValue( 0 );
}


void photovideownd::closeEvent( QCloseEvent *ev )
{
	(void)ev;
	onStopButtonClick();

	if( get_mode() != ccd::CCD_PHOTO_MODE )
	{
	    set_mode( ccd::CCD_PHOTO_MODE );
	    pmain_wnd->device_wnd->apply();
	}

	pmain_wnd->lock_ui( false );
}


void photovideownd::hideEvent ( QHideEvent * ev )
{
	if( ev->spontaneous() )
		return;

	close();
}


void photovideownd::onSessionStateChange( int state )
{
	is_sess = (state == Qt::Checked);
}


void photovideownd::onStartButtonClick()
{
 int res;

	if( is_session() )
	{
		if( (res = pmain_wnd->m_sess->check()) != S_ERR_OK )
		{
			u_msg("Error: %s", session::errstr( res ));
			return;
		}
	}

	const cam_base_params &p_ref = pmain_wnd->m_camera->get_params_ref();

	ui.ProgressBar->setFormat( "%p% : " + QString().setNum( p_ref.Exposition/1000, 'f', 1 ) + "s" );

	bool retval = pmain_wnd->m_camera->get_frame();
	if( retval )
		lock_ui( true );
}


void photovideownd::onStopButtonClick()
{
	pmain_wnd->m_camera->stop_frame();
	//lock_ui( false );
}


void photovideownd::set_mode( int val )
{
     switch( val )
     {
      case ccd::CCD_VIDEO_MODE:
           mode = val;
       break;
      case ccd::CCD_GUIDE_MODE:
           mode = val;
       break;
      default:
           mode = ccd::CCD_PHOTO_MODE;
     }
}


int  photovideownd::get_mode( void )
{
 return mode;
}


bool photovideownd::is_session( void )
{
	return is_sess;
}


void photovideownd::lock_ui( bool set )
{
	ui.checkBox_IsSession->setEnabled( mode == ccd::CCD_PHOTO_MODE ? (!set) : false );
	ui.StartButton->setEnabled( !set );
	ui.StopButton->setEnabled( set );
}


void photovideownd::cameraEvent( cam_progress_Event *ev )
{
     if( ev->ticks == 0 )
     {
    	 const cam_base_params &p_ref = pmain_wnd->m_camera->get_params_ref();

    	 ui.ProgressBar->setMaximum( p_ref.Exposition + p_ref.download_time );
     }
     else
     {
         ui.ProgressBar->setValue( (long)ev->ticks <= ui.ProgressBar->maximum() ? ev->ticks : ui.ProgressBar->maximum() );
     }
}
