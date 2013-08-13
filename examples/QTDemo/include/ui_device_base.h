/*
 * ui_abstract.h
 *
 *  Created on: 19.04.2011
 *      Author: gm
 */

#ifndef UI_ABSTRACT_H_
#define UI_ABSTRACT_H_

#include <QtGui/QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QHideEvent>

#include "camera.h"
#include "ui_devicewnd.h"


class ui_hw_dialog_wrapper;


class ui_device_base : public QDialog
{
	Q_OBJECT
public:
	ui_device_base( ccamera_base *p, QWidget *parent = 0 );
	virtual ~ui_device_base();

	virtual bool apply( void ) = 0;

private slots:
	void onHardwareParamsClick();
	void onApplyParamsClick();

protected:
	virtual void showEvent ( QShowEvent * event );

protected:
	virtual bool is_valid( void ) = 0;
	virtual void fill_interface( void ) = 0;

	ccamera_base *m_camera;

	QDialog *m_hw_ui;

	Ui::devicewndClass ui;
};


class ui_hw_dialog_wrapper : public QDialog
{
	Q_OBJECT
public:
	ui_hw_dialog_wrapper( QWidget *parent = 0 );
	virtual ~ui_hw_dialog_wrapper();

protected:
	virtual void showEvent ( QShowEvent * ev );
	virtual void closeEvent( QCloseEvent * ev );
	virtual void hideEvent( QHideEvent * ev );
signals:
	void onShowSignal();
	void onCloseSignal();
};

#endif /* UI_ABSTRACT_H_ */
