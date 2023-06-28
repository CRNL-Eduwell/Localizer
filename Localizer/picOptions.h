#ifndef PICOPTIONS_H
#define PICOPTIONS_H

#include <QtWidgets/QDialog>
#include "ui_picOptions.h"
#include "optionsParameters.h"
#include <cmath>

class picOptions : public QDialog
{
	Q_OBJECT

public:
	picOptions(QWidget *parent = 0);
	~picOptions();
	void connectSignals();
    InsermLibrary::picOption getPicOption();

private slots:
	void width2HeigthTrialRatio();
	void heigth2WidthTrialRatio();
	void width2HeigthPlotRatio();
	void heigth2WidthPlotRatio();

private:
	QSize memTrialmap, memPlot;
	Ui::FormOptPic ui;
};

#endif
