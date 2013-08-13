#ifndef CAM_UI_QHY5LII_H
#define CAM_UI_QHY5LII_H

#include <QtGui/QWidget>
#include "ui_cam_ui_qhy5lii.h"
#include "ui_device_base.h"
#include "cam_qhy5lii.h"


class cam_ui_qhy5lii : public ui_device_base
{
    Q_OBJECT

public:
    cam_ui_qhy5lii( ccamera_base *pcam, QWidget *parent = 0 );
    virtual ~cam_ui_qhy5lii();

    virtual bool apply( void );

protected slots:
        void onHwOkClick();

protected:
        virtual bool is_valid( void );
        virtual void fill_interface( void );

private:
    Ui::cam_ui_qhy5liiClass ui;

    cam_qhy5lii_params *m_params;
};

#endif // CAM_UI_QHY5LII_H
