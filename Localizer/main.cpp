#include <QtWidgets/QApplication>
#include "localizer.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Localizer/Resources/localizer.ico"));
    QDir::setCurrent(QCoreApplication::applicationDirPath());
	Localizer w;
	w.show();
	return a.exec();
}
