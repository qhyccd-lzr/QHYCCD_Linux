#include <math.h>

#include "histogramwnd.h"
#include "ccd.h"
#include "utils.h"
#include "graph.h"

//---------------------------------------------------------------------------


void CMarkerMove::onpressfunc( const QString &, double, double, int, int )
{
}


void CMarkerMove::onmovefunc( const QString &name, double x, double )
{
	if( name == "Low" )
	{
	    ui->spinBox_Low->setValue( (int)x );
	}
	else
	if( name == "Up" )
	{
		ui->spinBox_High->setValue( (int)x );
	}
}


void CMarkerMove::onreleasefunc( const QString &, double, double )
{
}


//--------------------------------------------------------

histogramwnd::histogramwnd(ccd *parent)
    : QDialog(parent),
      pmain_wnd(parent),
      m_drawer( NULL ),
      m_drawlayout( NULL ),
      m_histo_view( NULL ),
      m_image( NULL ),
      m_bpp( 0 ),
      m_channel_cnt( 0 ),
      m_channel_bpp( 0 ),
      m_applyLUT( false ),
      m_on_marker_move( NULL )
{
	ui.setupUi(this);

	setWindowTitle( tr("Histogram") );

	// connects all
	connect( ui.PreviewButton, SIGNAL(clicked()), this, SLOT(onPreviewButtonClick()) );
	connect( ui.spinBox_Low, SIGNAL(valueChanged(int)), this, SLOT(onLowRangeChange(int)) );
	connect( ui.spinBox_High, SIGNAL(valueChanged(int)), this, SLOT(onHighRangeChange(int)) );

	m_drawer = new CDrawWidget( NULL );

	m_drawlayout = new QGridLayout;
	m_drawlayout->setContentsMargins( 0, 0, 0, 0 );
	m_drawlayout->addWidget( m_drawer );
	ui.frame->setLayout( m_drawlayout );

	lock_ui( true );
}


histogramwnd::~histogramwnd()
{
	if( m_histo_view )
		delete m_histo_view;
	if( m_on_marker_move )
		delete m_on_marker_move;
	if( m_drawer )
		delete m_drawer;
	if( m_drawlayout)
		delete m_drawlayout;
}


void histogramwnd::showEvent( QShowEvent *event )
{
 QRect rc;

	if( event->spontaneous() )
		return;

	rc = QApplication::desktop()->availableGeometry();
	move( 0,  rc.height() - frameGeometry().height() );
}


//-------------------------------------------------------------------------------------------------
bool histogramwnd::setup( cimage_base * const pimg )
{
 int pixmax;
 QString title, up, low, tmp;

 	 if( pimg == NULL )
 	 {
 		 lock_ui( true );
 		 return false;
 	 }

 	 m_image = pimg;

 	 if( m_image->bpp() == m_bpp )
 	 {
 		 lock_ui( false );
 		 return true;
 	 }

     m_bpp = m_image->bpp();
     m_channel_cnt = m_image->channel_count();
     if( m_channel_cnt == 0 )
     {
    	 log_e("histogramwnd::Setup(): m_channel_cnt == 0");
    	 abort();
     }
     m_channel_bpp = pixel_base::bytes_per_pixel( m_bpp ) * 8 / m_channel_cnt;

     if( m_histo_view != NULL )
     {
         delete m_histo_view;
         m_histo_view = NULL;
     }
     pixmax = (int)(1 << m_channel_bpp);

     title = "Histogram";
     m_histo_view = new CGraph( ui.frame, pixmax, title);
     for( int i = 0;i < m_channel_cnt;i++ )
     {
    	 m_histo_view->AddLine( "channel_"+QString().setNum(i), QColor((i == 0 ? 255 : 0), (i == 1 ? 255 : 0), (i == 2 ? 255 : 0), 100), LS_SOLID );
     	 m_histo_view->SetData( "channel_"+QString().setNum(i), (double *)m_image->hist_channel(i) );
     }
     m_histo_view->AddLine( "Line", QColor(255, 255, 255), LS_WIRE );
     m_histo_view->SetData( "Line", (double *)m_image->hist_line() );

     m_on_marker_move = new CMarkerMove( &ui );

     up = "Up"; tmp = "U";
     m_histo_view->AddMarker( up, pixmax, 0, QColor(255, 255, 0), tmp, MS_LOCKED, m_on_marker_move );
     low = "Low"; tmp = "L";
     m_histo_view->AddMarker( low, 0, 0, QColor(255, 255, 0), tmp, MS_LOCKED, m_on_marker_move );

     m_drawer->SetSource( m_histo_view );

     ui.spinBox_Low->setMinimum( 0 );
     ui.spinBox_Low->setMaximum( pixmax-1 );
     ui.spinBox_Low->setValue( 0 );

     ui.spinBox_High->setMinimum( 0 );
     ui.spinBox_High->setMaximum( pixmax-1 );
     ui.spinBox_High->setValue( pixmax-1 );

     lock_ui( false );

 return true;
}


void histogramwnd::onPreviewButtonClick()
{
 double x, y;
 int low, up;
 QString str;

     if( m_image == NULL )
     {
         u_msg("NULL destination");
         return;
     }
     if( !m_histo_view )
         return;

     str = "Low";
     m_histo_view->GetMarkerPos( str, x, y );
     low = (int)x;

     str = "Up";
     m_histo_view->GetMarkerPos( str, x, y );
     up = (int)x;

     if( up <= low )
     {
         u_msg( "Upper level must be greater than Low level." );
         return;
     }

     m_image->create_histogram_preview( low, up, ui.LogButton->isChecked(), ui.AutoHist->isChecked(), m_applyLUT );
     for( int i = 0;i < m_channel_cnt;i++ )
    	 m_histo_view->SetData( "channel_"+QString().setNum(i), (double *)m_image->hist_channel(i) );
     m_histo_view->SetData( "Line", (double *)m_image->hist_line() );
     m_drawer->update();

     pmain_wnd->m_drawer->set_data( m_image->get_preview_ptr() );
     pmain_wnd->m_drawer->update();
}


void histogramwnd::onLowRangeChange( int val )
{
//
 int x;
 int ch_cnt;
 QString str;


     if( !m_histo_view )
         return;

     x = val;

     ch_cnt = (int)(1 << m_channel_bpp);

     if( x < 0 )
     {
         x = 0;
         ui.spinBox_Low->setValue( x );
     }
     if( x >= ch_cnt )
     {
         x = ch_cnt-1;
         ui.spinBox_Low->setValue( x );
     }

     str = "Low";
     m_histo_view->SetMarkerPos( str, (double)x, 0 );
     m_drawer->update();
}


void histogramwnd::onHighRangeChange( int val )
{
//
 int x;
 int ch_cnt;
 QString str;


     if( !m_histo_view )
         return;

     x = val;

     ch_cnt = (int)(1 << m_channel_bpp);

     if( x < 1 )
     {
         x = 1;
         ui.spinBox_High->setValue( x );
     }
     if( x >= ch_cnt )
     {
         x = ch_cnt-1;
         ui.spinBox_High->setValue( x );
     }

     str = "Up";
     m_histo_view->SetMarkerPos( str, (double)x, 0 );
     m_drawer->update();
}


void histogramwnd::preview( void )
{
	onPreviewButtonClick();
}


void histogramwnd::lock_ui( bool lock )
{
	ui.spinBox_Low->setEnabled( !lock );
	ui.spinBox_High->setEnabled( !lock );
	ui.PreviewButton->setEnabled( !lock );
}
