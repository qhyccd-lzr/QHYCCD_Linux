#include "ccd.h"
//#include <mcheck.h>

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
	//mtrace();
    QApplication a(argc, argv);
    ccd w;
    w.show();
    return a.exec();
}
