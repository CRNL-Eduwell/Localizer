#include "locaOops.h"

locaOops::locaOops(QWidget *parent) : QDialog(parent)
{
	d.setupUi(this);
	//QPixmap image("C:\\System98\\ExternalProgram\\Localizer\\Config\\Pictures\\wookie2.jpg");
	QPixmap image("Config\\Pictures\\wookie2.jpg");
	d.label->setPixmap(image);
}

locaOops::~locaOops()
{

}