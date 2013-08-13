#ifndef CAM_UI_QHY8L_H
#define CAM_UI_QHY8L_H

#include <QtGui/QWidget>
#include <QTimer>
#include "ui_cam_ui_qhy8l.h"
#include "ui_device_base.h"
#include "cam_qhy8l.h"


class cam_ui_qhy8l : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_qhy8l( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_qhy8l();

    virtual bool apply( void );

protected slots:
	void onHwOkClick();
	void onTimer();
	void onShow();
	void onClose();

protected:
	virtual bool is_valid( void );
	virtual void fill_interface( void );

private:
    Ui::cam_ui_qhy8lClass ui;

    cam_qhy8l_params *m_params;
	QTimer m_timer;
};

#endif // CAM_UI_QHY8L_H
