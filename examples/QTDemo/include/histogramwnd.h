#ifndef HISTOGRAMWND_H
#define HISTOGRAMWND_H

#include <QtGui>
#include <QtGui/QDialog>
//#include <QLayout>
//#include <QPainter>
#include "ui_histogramwnd.h"
#include "graph.h"
#include "image.h"

class CDrawWidget;
class ccd;


class CMarkerMove : public CMarkerWrapper
{
public:
	CMarkerMove( Ui::histogramwndClass *pUI ){ui = pUI;};
	virtual ~CMarkerMove() {};

	virtual void onpressfunc( const QString &name, double x, double, int scr_x, int scr_y );
	virtual void onmovefunc( const QString &name, double x, double y );
	virtual void onreleasefunc( const QString &name, double x, double y );

private:
	Ui::histogramwndClass *ui;
};


class histogramwnd : public QDialog
{
    Q_OBJECT

public:
    histogramwnd(ccd *parent = 0);
    ~histogramwnd();

    bool setup( cimage_base * const pimg );
    void preview( void );


protected slots:
	void onPreviewButtonClick();
	void onLowRangeChange( int );
	void onHighRangeChange( int );

protected:
	void showEvent( QShowEvent *event );

private:
	ccd *pmain_wnd;
	CDrawWidget *m_drawer;
	QGridLayout *m_drawlayout;
	CGraph *m_histo_view;

	cimage_base *m_image;
	int  m_bpp;
	int  m_channel_cnt;
	int  m_channel_bpp;
	bool m_applyLUT;
	CMarkerMove *m_on_marker_move;

	void lock_ui( bool lock );

private:
    Ui::histogramwndClass ui;
};



class CDrawWidget : public QWidget
{
    Q_OBJECT

public:
	CGraph *pGraph;
	bool isDragging;

	CDrawWidget(QWidget *parent = NULL ) : QWidget(parent)
	{
		pGraph = NULL;
		isDragging = false;

	};
    ~CDrawWidget()
    {
    };
    void SetSource( CGraph *pG = NULL )
    {
    	pGraph = pG;
    }

protected:
    void paintEvent(QPaintEvent *)
    {
    	if( pGraph == NULL )
    		return;

    	pGraph->OnPaint();

    	QPainter painter( this );
    	painter.drawImage( 0, 0, *pGraph->GetBuffer() );
    	painter.end();
    };


    void mouseMoveEvent ( QMouseEvent *event )
    {
    	if( !pGraph )
    		return;

    	if( !isDragging )
    		return;

    	pGraph->MoveMarker( event->x(), event->y() );
    	update();

    }
    void mousePressEvent ( QMouseEvent *event )
    {
    	if( !pGraph )
    	    return;

    	if( event->button() != Qt::LeftButton || !pGraph->ActivateMarker( event->x(), event->y() ) )
    		return;

    	isDragging = true;

    }
    void mouseReleaseEvent ( QMouseEvent * )
    {
    	if( !pGraph )
    	    return;

    	pGraph->DeActivateMarker();
    }

};





#endif // HISTOGRAMWND_H
