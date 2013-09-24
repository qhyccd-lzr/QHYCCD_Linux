#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ccd.h"
#include "photovideownd.h"
#include "utils.h"
#include "maindef.h"

#include "cam_null.h"
#include "cam_qhy6.h"
#include "cam_qhy5lii.h"
#include "cam_ic8300.h"

#include "cam_ui_null.h"
#include "cam_ui_qhy5.h"
#include "cam_ui_qhy5lii.h"
#include "cam_ui_qhy6.h"
#include "cam_ui_ic8300.h"
#include "cam_ui_qhy8l.h"


// Init resources
ccd::ccd(QWidget *parent) : QWidget(parent)
{
	setlocale (LC_ALL, "C");

	ui.setupUi(this);

	setWindowTitle( QString(CPY_RIGHT(VERSION)) );

	move(0, 0);

	param_block = new params();
	param_block->load();

	// Manual init...
	m_drawer = new cdrawer();

	m_image_buffer = NULL;

	connect( ui.ConfigDeviceButton, SIGNAL( clicked() ), this, SLOT( onConfigDeviceButtonClick() ) );
	connect( ui.PhotoButton, SIGNAL( clicked() ), this, SLOT( onPhotoButtonClick() ) );
	connect( ui.VideoButton, SIGNAL( clicked() ), this, SLOT( onVideoButtonClick() ) );
	connect( ui.SessionsButton, SIGNAL( clicked() ), this, SLOT( onSessionsButtonClick() ) );
	connect( ui.HistogramButton, SIGNAL( clicked() ), this, SLOT( onHistogramButtonClick() ) );
	connect( ui.ConfigButton, SIGNAL( clicked() ), this, SLOT( onConfigButtonClick() ) );
	connect( ui.pushButton_FitToWindowScale, SIGNAL( clicked() ), this, SLOT( onFitToWindowScaleClick() ) );
	connect( ui.pushButton_About, SIGNAL( clicked() ), this, SLOT( onAboutButtonClick() ) );
	connect( ui.SaveShotButton, SIGNAL( clicked() ), this, SLOT( onSaveShotButtonClick() ) );

	// Correct main wnd
	this->layout()->setContentsMargins( 0, 0, 0, 0 );
	static_cast<QGridLayout *>(this->layout())->setVerticalSpacing(0);

	// Init scroller
	m_scroll_area = new QScrollArea( NULL );
	m_scroll_area->setWidget( m_drawer );


	m_drawer->setAttribute( Qt::WA_NoSystemBackground, true );
	ui.ScrollFrame->setAttribute( Qt::WA_NoSystemBackground, true );
	ui.ScrollFrame->setFrameShape( QFrame::NoFrame );

	m_scroll_area->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	m_scroll_area->setFocusPolicy( Qt::NoFocus );
	m_scroll_area->viewport()->setAutoFillBackground(false);
	m_scroll_area->setAttribute( Qt::WA_NoSystemBackground, true );

	m_scroll_layout = new QGridLayout;
	m_scroll_layout->setContentsMargins( 0, 0, 0, 0 );
	m_scroll_layout->addWidget( m_scroll_area );
	ui.ScrollFrame->setLayout( m_scroll_layout );

	m_got_frame_cnt = 0;

	// Init Camera
	switch( param_block->get_camera_model() )
	{
        case camera_model::unknown:
	case camera_model::null:
		m_camera = new ccamera_null();
		break;
	case camera_model::qhy5:
		m_camera = new ccamera_qhy5();
		break;
        case camera_model::qhy5lii:
                m_camera = new ccamera_qhy5lii();
                break;
	case camera_model::qhy6:
		m_camera = new ccamera_qhy6();
		break;
	case camera_model::ic8300:
		m_camera = new ccamera_ic8300();
		break;
	case camera_model::qhy8l:
		m_camera = new ccamera_qhy8l();
		break;
	default:
		log_e("Wrong camera class");
		abort();
	}

	m_cam_params = m_camera->alloc_params_object();

	// run and initialize...
	m_camera->start();

	m_camera->get_params( m_cam_params );
	// load params from file or reset if no config file
	m_cam_params->load();

	m_camera->set_params( m_cam_params );
	m_camera->get_params( m_cam_params );

	connect( m_camera, SIGNAL( receiveData(int, const void *, int) ), this, SLOT( onGetImage(int,const void *, int) )	);

	//-------------------------------------------------------------
	m_sess = new session( m_camera->get_params_ref() );

	// Init Camera
	switch( param_block->get_camera_model() )
	{
        case camera_model::unknown:
	case camera_model::null:
		device_wnd = new cam_ui_null( m_camera, this );
		break;
	case camera_model::qhy5:
		device_wnd = new cam_ui_qhy5( m_camera, this );
		break;
	case camera_model::qhy5lii:
		device_wnd = new cam_ui_qhy5lii( m_camera, this );
		break;
	case camera_model::qhy6:
		device_wnd = new cam_ui_qhy6( m_camera, this );
		break;
	case camera_model::ic8300:
		device_wnd = new cam_ui_ic8300( m_camera, this );
		break;
	case camera_model::qhy8l:
		device_wnd = new cam_ui_qhy8l( m_camera, this );
		break;
	default:
		log_e("Wrong camera UI class");
		abort();
	}

	photo_video_wnd = new photovideownd( this );

	histogram_wnd = new histogramwnd( this );
	// Image created so lets init histogram
	histogram_wnd->setup( m_image_buffer );

	session_wnd = new sessionswnd( this );

	config_wnd = new configwnd( param_block, this );

	// about dialog
	about_wnd = new about( this );

	//-------------------------------------------------------------

	m_camera->set_notify_wnd( photo_video_wnd );

	lock_ui( false );
}


// On close application we MUST free ALL resources
ccd::~ccd()
{
	//m_camera->stop();
	delete m_camera;

	delete m_cam_params;

	if( device_wnd != NULL )
		delete device_wnd;
	if( photo_video_wnd != NULL )
		delete photo_video_wnd;
	if( histogram_wnd )
		delete histogram_wnd;
	if( session_wnd )
		delete session_wnd;
	if( config_wnd )
		delete config_wnd;
	if( about_wnd )
		delete about_wnd;

	delete m_drawer;
	delete m_image_buffer;

	delete m_sess;

	delete m_scroll_area;
	delete m_scroll_layout;

	// last!
	if( param_block )
		delete param_block;
}


//------------------------------------------
void ccd::showEvent( QShowEvent * ev )
{
	if( ev->spontaneous() )
		return;

	out_info();
}


void ccd::closeEvent( QCloseEvent *event )
{
	if( !u_yes("Do you want to quit?") )
	{
		event->ignore();
		return;
	}

	if( device_wnd->isVisible() )
		device_wnd->close();

	if( photo_video_wnd->isVisible() )
		photo_video_wnd->close();

	if( histogram_wnd->isVisible() )
		histogram_wnd->close();

	if( about_wnd->isVisible() )
		about_wnd->close();

	// save params
	param_block->save();
}

//------------------------------------------


void ccd::onConfigDeviceButtonClick()
{
	if( !device_wnd->isVisible() )
	{
		device_wnd->show();
		device_wnd->move( 0, geometry().y() + ui.frame_2->y() + ui.ConfigDeviceButton->height() + 4 );
	}
}


void ccd::onPhotoButtonClick()
{
	lock_ui( true );

    photo_video_wnd->set_mode( CCD_PHOTO_MODE );
    photo_video_wnd->move( device_wnd->frameSize().width(), geometry().y() + ui.frame_2->y() + ui.ConfigDeviceButton->height() + 4 );
    photo_video_wnd->show();
}


void ccd::onVideoButtonClick()
{
	lock_ui( true );

	photo_video_wnd->set_mode( CCD_VIDEO_MODE );
	photo_video_wnd->move( device_wnd->frameSize().width(), geometry().y() + ui.frame_2->y() + ui.ConfigDeviceButton->height() + 4 );
	photo_video_wnd->show();
}


void ccd::onSessionsButtonClick()
{
	session_wnd->exec();
}


void ccd::onHistogramButtonClick( void )
{
	histogram_wnd->show();
}


void ccd::onConfigButtonClick()
{
	config_wnd->exec();
}


void ccd::onFitToWindowScaleClick()
{
	if( ui.pushButton_FitToWindowScale->isChecked() )
	{
		m_drawer->set_scale_size( m_scroll_area->width()-2*m_scroll_area->frameWidth(),
								  m_scroll_area->height()-2*m_scroll_area->frameWidth() );
	}
	else
		m_drawer->set_scale_size( 0, 0 );

	m_drawer->update();
}


void ccd::onAboutButtonClick()
{
	about_wnd->show();

//	if( m_image_buffer && m_image_buffer->bpp() != 16 )
//	{
//		delete m_image_buffer;
//		m_image_buffer = NULL;
//	}
//	if( !m_image_buffer )
//		m_image_buffer = new cimage<pixel_mono16>();
//	m_image_buffer->load( "/home/stepanenko/300_6.tif" );
//	std::vector<float> debayer_settings;
//	debayer_settings.resize( 5 );
//
//	// offset
//	debayer_settings[0] = 0;
//	debayer_settings[1] = 1;
//	// correction coefficients
//	debayer_settings[2] = 1.0/0.96;		// r
//	debayer_settings[3] = 1.0;			// g
//	debayer_settings[4] = 1.0/0.773;	// b
//	log_i( "debayer_data.size=%d", debayer_settings.size() );
//
//	m_image_buffer->set_preview_debayer_settings( debayer_settings );
//	show_ccd_image();
}


void ccd::onSaveShotButtonClick()
{
	if( !m_image_buffer )
	{
		u_msg( tr("No image to save").toAscii().data() );
		return;
	}

	QFileDialog *pDlg = new QFileDialog( this );

 	pDlg->setAcceptMode( QFileDialog::AcceptSave );
 	pDlg->setWindowTitle( "Save image" );
 	pDlg->setFilter( tr("Preview BMP (*.bmp);;Image TIFF (*.tif)") );
 	pDlg->setViewMode( QFileDialog::List );
 	pDlg->setFileMode( QFileDialog::AnyFile );

 	if( pDlg->exec() )
 	{
 		QStringList fnames = pDlg->selectedFiles();
 		if( fnames.count() )
 		{
 			QString str = fnames.at(0);
 			m_image_buffer->save( (str + ".tif").toUtf8().data() );
 			if( param_block->get_save_preview() )
 			{
 				const QImage *pBMP = m_image_buffer->get_preview_ptr();
 				if( pBMP )
 					pBMP->save( str+".bmp", "bmp", -1 );
 			}
 		}
 	}

 	delete pDlg;
}


bool ccd::save_frame( void )
{
 QString fname;
 const QImage *pBMP = NULL;
 int res;

	res = m_sess->get_fname( &fname );
	if( res != 0 )
		return false;
	m_image_buffer->save( (fname + ".tif").toUtf8().data() );
	if( param_block->get_save_preview() && (pBMP = m_image_buffer->get_preview_ptr()) != NULL )
	 	pBMP->save( fname+".bmp", "bmp", -1 );

 return true;
}


void ccd::onGetImage( int ret_code, const void *src, int len )
{
	bool is_session = photo_video_wnd->is_session();
	bool do_continue = false;
	frame_params_t frame_params;

	if( ret_code != ccamera_base::ERRCAM_OK )
	{
		log_e( "%s error code = %d", ret_code == ccamera_base::ERRCAM_USER_BREAK ? "User break" : "Async command error...", ret_code );
	}
	else
	{
		frame_params = m_camera->get_buffer_frame();

		cimage_base::get_compatible_image( &m_image_buffer, frame_params.width, frame_params.height, frame_params.bpp );
		if( !m_image_buffer )
		{
			log_e("ccd::onGetImage(): Fatal internal error. cimage_base::get_compatible_image() failed. Close application immediately");
			return;
		}
		m_image_buffer->assign_raw_data( (char*)src, len, frame_params.width, frame_params.height, frame_params.bpp );
		m_image_buffer->set_preview_debayer_settings( m_camera->get_debayer_settings() );

		show_ccd_image();

		if( is_session )
		{
			save_frame();
			do_continue = m_sess->inc();
		}

		m_got_frame_cnt++;
		log_i("Got image!!!");
		out_info();
	}

	log_i("Async onGetImage...");
	m_camera->continue_device();	// MUST BE called!!! to continue device thread after async task

	if( ret_code == ccamera_base::ERRCAM_OK &&
		(photo_video_wnd->get_mode() == CCD_VIDEO_MODE || (is_session && do_continue)) )
	{
		m_camera->get_frame();
	}
	else
		photo_video_wnd->lock_ui( false );
}


bool ccd::show_ccd_image( void )
{
	histogram_wnd->setup( m_image_buffer );

 	if( histogram_wnd->isVisible() && ui.AutoApplyHistogram->isChecked() )
    {
        histogram_wnd->preview();  // set data & update
    }
    else
    {
    	m_image_buffer->create_preview();
        m_drawer->set_data( m_image_buffer->get_preview_ptr() );
        m_drawer->update();
    }

 return true;
}


void ccd::lock_ui( bool set )
{
	//device_wnd->setEnabled( !set );
	ui.PhotoButton->setEnabled( !set );
	ui.VideoButton->setEnabled( !set );
	ui.SessionsButton->setEnabled( !set );
	ui.ConfigButton->setEnabled( !set );
	//ui.pushButton_FitToWindowScale->setEnabled( !set );
}


void ccd::out_info( void )
{
	char str[200];
	int cntr[S_MAX_IDX];

	m_sess->get_counters( cntr, S_MAX_IDX );

	snprintf( str, sizeof(str)-1, "Fr:%d Cnt:%d  cur:%d", cntr[S_FROM_IDX], cntr[S_COUNT_IDX], cntr[S_CUR_IDX]);
	ui.l_frames->setText( QString( str ) );
	ui.l_info->setText( QString().setNum(m_got_frame_cnt) + " frm." );
	ui.l_driver->setText( camera_model::to_string( m_camera->get_model() ) );
}

