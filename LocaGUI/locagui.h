#ifndef LOCAGUI_H
#define LOCAGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qcheckbox.h>

#include <vector>

#include "ui_locagui.h"
#include "LOCA.h"
#include "Worker.h"

class LocaGUI : public QMainWindow
{
	Q_OBJECT

public:
	LocaGUI(QWidget *parent = 0);
	~LocaGUI();
	void setUpGUI(QMainWindow* QTGUIClass);
	void readFreqFile(std::string p_pathFreq);
	template<typename T> std::vector<T> split(const T & str, const T & delimiters);
	void connectAllBordel();
	void loadTRCFromFolder(std::string p_pathFolder);
	void loadTRCListWidget(std::vector<std::vector<std::string>> p_trcList);
	void addProv2List(std::string p_locaName);
	void removeProv2List(int p_index);

public slots:
	void launchAnalysis();
	void browsePatient();
	void FreqBandCheck(bool isChecked);
	void addTRC2List();
	void removeTRC2List();
	void provClicked(QListWidgetItem *provItem);
	void displayLog(QString info);
	void receiveElanPointer(InsermLibrary::ELAN *p_elan);
	void receiveOptionPointer(InsermLibrary::OptionLOCA *optionLOCA);

	void upDateProgressBar(int value);
private slots :
	void openOptions();
	void openAbout();

signals:
	void bipDone(bool);

private:
	std::vector<std::string> freqBandName;
	std::vector<std::vector<double>> freqBandValue;
	QWidget **freqTAB = nullptr;
	QCheckBox ***freqCheckBox = nullptr;
	QListWidgetItem **listTRCWidget = nullptr;
	std::vector<int> indexTRCList;
	std::vector<std::string> directoryList;
	std::vector<std::vector<std::string>> trcList;
	Ui::LocaGUIClass ui;
	QThread* thread = nullptr;
	Worker* worker = nullptr;
	InsermLibrary::OptionLOCA *optionLOCAGUI = nullptr;
};

#endif // LOCAGUI_H
