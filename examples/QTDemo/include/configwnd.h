#ifndef CONFIGWND_H
#define CONFIGWND_H

#include <QtGui/QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QHideEvent>
#include "ui_configwnd.h"
#include "params.h"


class configwnd : public QDialog
{
    Q_OBJECT

public:
    configwnd( params *param_ref, QWidget *parent = 0);
    ~configwnd();

protected slots:
	void onCameraListChanged( int index );
	void onPostprocessingEnabledChanged( int state );

	void onOkButtonClick();
	void onCancelButtonClick();

protected:
	void showEvent( QShowEvent * event );
	void closeEvent ( QCloseEvent * event );
	void hideEvent ( QHideEvent * event );

private:
	params *m_params_ref;

	void fill_interface( void );

private:
    Ui::configwndClass ui;
};

#endif // CONFIGWND_H
