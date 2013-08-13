/*
 * cam_ui_null.h
 *
 *      Author: gm
 */

#ifndef CAM_UI_NULL_H
#define CAM_UI_NULL_H

#include <QtGui/QWidget>
#include "ui_cam_ui_null.h"
#include "ui_device_base.h"
#include "cam_null.h"


class cam_ui_null : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_null( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_null();

    virtual bool apply( void );

protected slots:
	void onHwOkClick();

protected:
	virtual bool is_valid( void );
	virtual void fill_interface( void );

private:
    Ui::cam_ui_nullClass ui;

    cam_null_params *m_params;
};

#endif // CAM_UI_NULL_H
