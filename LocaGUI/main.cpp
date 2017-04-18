#include <QtWidgets/QApplication>
#include "localizer.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Localizer w;
	w.show();
	return a.exec();
}