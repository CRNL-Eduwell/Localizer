#ifndef OPTIONSTAT_H
#define OPTIONSTAT_H

#include <QtWidgets/QDialog>
#include <QString>

#include "displayProv.h"
#include "Utility.h"
#include "ui_optionsStats.h"
#include <QMouseEvent>
#include "optionsParameters.h"

using namespace std;
using namespace InsermLibrary;

class optionsStats : public QDialog
{
	Q_OBJECT

public :
	optionsStats(QWidget *parent = 0);
	~optionsStats();
	statOption getStatOption();

private:
	void connectSignals();
	void displayLoca(QListWidget *uiList, vector<QString> wantedLoca);
	void readList();

private slots:
	void openProvWindow();
	void deleteProvFromList();
	void getProvList(vector<QString> provList);
	void saveListsandClose();
	void pValueKruskall();
	void pValueWilcoxon();
	void updateWilOpt();
	void updateKWOpt();
private:
	vector<QString> wantedLocaKW;
	vector<QString> wantedLocaWil;
	bool toggle = false;
	displayProv *prov = nullptr;
	QString kwFilePath = "./Resources/Config/statKW.txt";
	QString wilFilePath = "./Resources/Config/statWil.txt";
	Ui::FormStat ui;
};


#endif