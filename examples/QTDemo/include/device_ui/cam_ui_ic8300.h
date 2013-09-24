#ifndef CAM_UI_ic8300_H
#define CAM_UI_ic8300_H

#include <QtGui/QWidget>
#include <QTimer>
#include "ui_cam_ui_ic8300.h"
#include "ui_device_base.h"
#include "cam_ic8300.h"


class cam_ui_ic8300 : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_ic8300( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_ic8300();

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
    Ui::cam_ui_ic8300Class ui;

    cam_ic8300_params *m_params;
    QTimer m_timer;
};

#endif // CAM_UI_ic8300_H
