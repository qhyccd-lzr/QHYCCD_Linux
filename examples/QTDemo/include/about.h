#ifndef ABOUT_H
#define ABOUT_H

#include <QtGui/QDialog>
#include "ui_about.h"

class about : public QDialog
{
    Q_OBJECT

public:
    about(QWidget *parent = 0);
    ~about();

private:
    Ui::aboutClass ui;
protected slots:
	void onOk_ButtonPressed();
};

#endif // ABOUT_H
