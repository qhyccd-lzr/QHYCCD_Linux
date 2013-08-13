#include "about.h"
#include "maindef.h"


about::about(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	
	setWindowTitle( tr("About CCD") );
	
	QString str = ui.label_Info->text();
	str.replace( "*VERSION*", QString(CPY_RIGHT(VERSION)) );
	ui.label_Info->setText( str );

	// connect controls
	connect( ui.pushButton_Ok, SIGNAL( released() ), this, SLOT( onOk_ButtonPressed() ) );
}

about::~about()
{
}


void about::onOk_ButtonPressed()
{
	close();
}

