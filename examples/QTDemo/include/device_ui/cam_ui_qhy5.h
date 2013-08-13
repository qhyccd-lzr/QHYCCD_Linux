/*
 * cam_ui_qhy5.h
 *
 *      Author: gm
 */

#ifndef CAM_UI_QHY5_H
#define CAM_UI_QHY5_H

#include <QtGui/QWidget>
#include "ui_cam_ui_qhy5.h"
#include "ui_device_base.h"
#include "cam_qhy5.h"


class cam_ui_qhy5 : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_qhy5( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_qhy5();

    virtual bool apply( void );

protected slots:
	void onHwOkClick();

protected:
	virtual bool is_valid( void );
	virtual void fill_interface( void );

private:
    Ui::cam_ui_qhy5Class ui;

    cam_qhy5_params *m_params;
};

#endif // CAM_UI_QHY5_H
