#ifndef LOCAGUI_H
#define LOCAGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qcheckbox.h>

#include <vector>
#include "ui_locagui.h"
#include "LOCA.h"
#include "Worker.h"


using namespace std;
using namespace InsermLibrary;

class LocaGUI : public QMainWindow
{
	Q_OBJECT

public:
	LocaGUI(QWidget *parent = 0);
	~LocaGUI();
	void setUpGUI(QMainWindow* QTGUIClass);
	void readFreqFile(string p_pathFreq);
	void connectAllBordel();
	void loadTRCFromFolder(string p_pathFolder);
	void loadTRCListWidget(vector<vector<string>> p_trcList);
	void addProv2List(string p_locaName);
	void removeProv2List(int p_index);

public slots:
	void launchAnalysis();
	void browsePatient();
	void FreqBandCheck(bool isChecked);
	void addTRC2List();
	void removeTRC2List();
	void provClicked(QListWidgetItem *provItem);
	void displayLog(QString info);
	void receiveElanPointer(ELAN *p_elan);
	void receiveOptionPointer(OptionLOCA *optionLOCA);

	void upDateProgressBar(int value);
	void finishedRuning();


	void receiveOpenSig();



private slots :
	void openOptions();
	void openAbout();

signals:
	void bipDone(bool);

	void winClose(bool);

private:
	vector<string> freqBandName;
	vector<vector<double>> freqBandValue;
	QWidget **freqTAB = nullptr;
	QCheckBox ***freqCheckBox = nullptr;
	QListWidgetItem **listTRCWidget = nullptr;
	vector<int> indexTRCList;
	vector<string> directoryList;
	vector<vector<string>> trcList;
	Ui::LocaGUIClass ui;
	QThread* thread = nullptr;
	Worker* worker = nullptr;
	OptionLOCA *optionLOCAGUI = nullptr;

	bool alreadyRunning = false;
};

#endif // LOCAGUI_H
