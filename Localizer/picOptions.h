#ifndef PICOPTIONS_H
#define PICOPTIONS_H

#include <QtWidgets/QDialog>
#include "ui_picOptions.h"
#include "Utility.h"
#include "optionsParameters.h"

using namespace std;
using namespace InsermLibrary;

class picOptions : public QDialog
{
	Q_OBJECT

public:
	picOptions(QWidget *parent = 0);
	~picOptions();
	void connectSignals();
	picOption getPicOption();

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
