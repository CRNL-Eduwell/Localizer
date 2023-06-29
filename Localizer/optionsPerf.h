#ifndef OPTIONPERF_H
#define OPTIONPERF_H

#include <QCoreApplication>
#include <QtWidgets/QDialog>
#include <QString>
#include "ui_optionsPerf.h"
#include <iostream>
#include <fstream>
#include "displayProv.h"

class optionsPerf : public QDialog
{
	Q_OBJECT

public:
	optionsPerf(QWidget *parent = 0);
	~optionsPerf();
	void getPerfLoca(std::vector<QString> &locaPerf);

private:
	void connectSignals();
	void displayLoca(std::vector<QString> wantedLoca);
	void readList();

private slots:
	void openProvWindow();
	void deleteProvFromList();
	void getProvList(std::vector<QString> provList);
	void saveListandClose();

private:
	std::vector<QString> wantedLoca;
	displayProv *prov = nullptr;
    QString perfFilePath = QCoreApplication::applicationDirPath() + "/Resources/Config/perfComp.txt";
	Ui::FormPerf ui;
};


#endif
