#ifndef FREQWIDGET_H
#define FREQWIDGET_H

#include <QtWidgets/QDialog>
#include "ui_freqwidget.h"

class freqwidget : public QDialog
{
	Q_OBJECT

public:
	freqwidget(QWidget *parent = 0);
	~freqwidget();
private:
	//void connectSignals();

private:
	Ui::Form ui;
};


#endif