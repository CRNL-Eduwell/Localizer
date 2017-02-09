#include "locagui.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>
//#include "vld.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LocaGUI w;
	w.show();
	QApplication::setStyle(QStyleFactory::create("fusion"));

	return a.exec();
}