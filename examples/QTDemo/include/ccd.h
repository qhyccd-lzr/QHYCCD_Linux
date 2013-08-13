#ifndef CCD_H
#define CCD_H

#include <Qt>
#include <QtGui/QWidget>
#include <QLayout>
#include <QScrollArea>
#include <QPainter>
#include <QButtonGroup>
#include <QCloseEvent>

#include "ui_ccd.h"
#include "camera.h"
#include "photovideownd.h"
#include "histogramwnd.h"
#include "sessionswnd.h"
#include "configwnd.h"
#include "about.h"
#include "image.h"
#include "ui_device_base.h"
#include "params.h"


class cdrawer;


class ccd : public QWidget
{
    Q_OBJECT
friend class photovideownd;
friend class histogramwnd;
friend class sessionswnd;
friend class session;

public:
	enum operation_mode
	{
		CCD_VIDEO_MODE = 1,
		CCD_GUIDE_MODE,
		CCD_PHOTO_MODE
	};

    ccd(QWidget *parent = 0);
    ~ccd();

    void lock_ui( bool set );
	bool show_ccd_image( void );

// my own event handlers
protected:
	void showEvent ( QShowEvent * event );
	void closeEvent( QCloseEvent *event );


// my own slots
protected slots:
	void onConfigDeviceButtonClick();
	void onPhotoButtonClick();
	void onVideoButtonClick();
	void onSessionsButtonClick();
	void onHistogramButtonClick();
	void onConfigButtonClick();
	void onFitToWindowScaleClick();
	void onAboutButtonClick();
	void onSaveShotButtonClick();

	void onGetImage( int, const void *, int );

private:
	QScrollArea *m_scroll_area;
	QGridLayout *m_scroll_layout;

	cdrawer *m_drawer; // Drawing widget
	cimage_base  *m_image_buffer;
	ccamera_base *m_camera;
	cam_base_params *m_cam_params;
	session	 *m_sess;

	//devicewnd *device_wnd;
	ui_device_base *device_wnd;
	photovideownd *photo_video_wnd;
	histogramwnd *histogram_wnd;
	sessionswnd *session_wnd;
	configwnd *config_wnd;
	about *about_wnd;

	params *param_block;

	int m_got_frame_cnt;

	bool save_frame( void );
	void out_info( void );

private:
    Ui::MainWnd ui;


};




class cdrawer : public QWidget
{
    Q_OBJECT

public:
	cdrawer(QWidget *parent = NULL ) :
		QWidget(parent),
		m_pimage( NULL ),
		m_scaled_pimage( NULL ),
		m_draw_ptr( NULL ),
		m_fit_wd( 0 ),
		m_fit_ht( 0 ),
		m_scale_k( 1.0 )
	{
		m_scaled_pimage = new QImage();
	}
    virtual ~cdrawer()
    {
    	delete m_scaled_pimage;
    }

    bool set_data( const QImage *psrc )
    {
    	m_pimage = psrc;

    	if( m_pimage == NULL )
    		return false;

    	set_scale_size( m_fit_wd, m_fit_ht );

     return true;
    }

    bool set_scale_size( int fitto_wd, int fitto_ht )
    {
    	m_fit_wd = fitto_wd;
    	m_fit_ht = fitto_ht;

    	if( !m_pimage )
    	    return false;

    	double k = 1.0;
    	if( m_fit_wd == 0 || m_fit_ht == 0 )
    		k = 1.0;
    	else
    	{
    		double aspect_image = (double)m_pimage->width()/(double)m_pimage->height();
    		double aspect_fit_size = (double)m_fit_wd/(double)m_fit_ht;

    		if( aspect_fit_size > aspect_image )
    			k = (double)m_fit_ht / (double)m_pimage->height();
    		else
    			k = (double)m_fit_wd / (double)m_pimage->width();
    	}
    	return set_scale( k );
    }

    bool set_scale( double k )
    {
    	if( k <= 0 || k > 1.0 )
    		m_scale_k = 1.0;
    	else
    		m_scale_k = k;

    	if( !m_pimage )
    	    return false;

    	if( m_scale_k == 1.0 )
    		m_draw_ptr = m_pimage;
    	else
    	{
    		QSize sz = QSize( (double)m_pimage->width()*k, (double)m_pimage->height()*k );
    		*m_scaled_pimage = m_pimage->scaled( sz, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    		m_draw_ptr = m_scaled_pimage;
    	}

    	resize( m_draw_ptr->size() );

    	return true;
    }

protected:
    void paintEvent(QPaintEvent *)
    {
    	if( !m_draw_ptr )
    		return;

    	QPainter painter;

    	painter.begin(this);
    	painter.drawImage( 0, 0, *m_draw_ptr );
    	painter.end();
    };

private:
    const QImage *m_pimage;
    QImage *m_scaled_pimage;
    const QImage *m_draw_ptr;
    int m_fit_wd;
    int m_fit_ht;
    double m_scale_k;	// scale factor
};

#endif // CCD_H
