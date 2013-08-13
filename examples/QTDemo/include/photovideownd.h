#ifndef PHOTOVIDEOWND_H
#define PHOTOVIDEOWND_H

#include <QtGui/QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QHideEvent>
#include "camera.h"
#include "ui_photovideownd.h"


class ccd;
class cam_progress_Event;

class photovideownd : public QDialog
{
    Q_OBJECT

public:
    photovideownd(ccd *parent = 0);
    ~photovideownd();

    void set_mode( int val );
    int  get_mode( void );
    bool is_session( void );

    void lock_ui( bool set );

protected:
	void showEvent ( QShowEvent * event );
	void closeEvent( QCloseEvent *event );
	void hideEvent ( QHideEvent * event );

	void cameraEvent( cam_progress_Event *event );

	bool event(QEvent *event);


protected slots:
	void onSessionStateChange( int );
	void onStartButtonClick();
	void onStopButtonClick();

private:
	int  mode;
	bool is_sess;
	ccd *pmain_wnd;

private:
    Ui::photovideowndClass ui;
};

#endif // PHOTOVIDEOWND_H
