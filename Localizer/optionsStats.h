#ifndef OPTIONSTAT_H
#define OPTIONSTAT_H

#include <QCoreApplication>
#include <QtWidgets/QDialog>
#include <QString>
#include <QLocale>
#include <QDebug>

#include "Utility.h"
#include "ui_optionsStats.h"
#include <QMouseEvent>
#include "optionsParameters.h"

class optionsStats : public QDialog
{
	Q_OBJECT

public :
	optionsStats(QWidget *parent = 0);
	~optionsStats();
	InsermLibrary::statOption getStatOption();

private:
	void connectSignals();

private slots:
	void pValueKruskall();
	void pValueWilcoxon();
	void updateWilOpt();
	void updateKWOpt();

private:
	Ui::FormStat ui;
};


#endif
