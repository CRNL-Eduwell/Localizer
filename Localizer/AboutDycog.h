#ifndef ABOUTDYCOG_H
#define ABOUTDYCOG_H

#include <QtWidgets/QDialog>
#include "ui_AboutDycog.h"

class AboutDycog : public QDialog
{
	Q_OBJECT

public:
	AboutDycog(QWidget *parent = 0);
	~AboutDycog();

private:
	Ui::FormAbout ui;
};
#endif