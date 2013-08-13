/*
 * cam_ui_qhy6.h
 *
 *      Author: gm
 */

#ifndef CAM_UI_QHY6_H
#define CAM_UI_QHY6_H

#include <QtGui/QWidget>
#include "ui_cam_ui_qhy6.h"
#include "ui_device_base.h"
#include "cam_qhy6.h"


class cam_ui_qhy6 : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_qhy6( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_qhy6();

    virtual bool apply( void );

protected slots:
	void onHwOkClick();

protected:
	virtual bool is_valid( void );
	virtual void fill_interface( void );

private:
    Ui::cam_ui_qhy6Class ui;

    cam_qhy6_params *m_params;
};

#endif // CAM_UI_QHY6_H
