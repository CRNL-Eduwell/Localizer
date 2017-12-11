#ifndef OPTIONPERF_H
#define OPTIONPERF_H

#include <QtWidgets/QDialog>
#include <QString>
#include "ui_optionsPerf.h"
#include <iostream>
#include <fstream>
#include "Utility.h"
#include "displayProv.h"

using namespace std;
using namespace InsermLibrary;

class optionsPerf : public QDialog
{
	Q_OBJECT

public:
	optionsPerf(QWidget *parent = 0);
	~optionsPerf();
	void getPerfLoca(vector<QString> &locaPerf);

private:
	void connectSignals();
	void displayLoca(vector<QString> wantedLoca);
	void readList();

private slots:
	void openProvWindow();
	void deleteProvFromList();
	void getProvList(vector<QString> provList);
	void saveListandClose();

private:
	vector<QString> wantedLoca;
	displayProv *prov = nullptr;
	QString perfFilePath = "./Resources/Config/perfComp.txt";
	Ui::FormPerf ui;
};


#endif