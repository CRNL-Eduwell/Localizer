#ifndef LOCAGUI_H
#define LOCAGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qcheckbox.h>
#include <vector>

#include "ui_locagui.h"

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

	/*Slots*/
	public slots:
	void browsePatient();
	void FreqBandCheck(bool isChecked);
	void addTRC2List();
	void removeTRC2List();
	void testTextShell();
	/*Signaux*/
	//signals:

private:
	std::vector<std::string> freqBandName;
	std::vector<std::vector<int>> freqBandValue;
	/*Widget Freq*/
	QWidget **freqTAB;
	QCheckBox ***freqCheckBox;

	QListWidgetItem **listTRCWidget = nullptr;
	std::vector<int> indexTRCList;
	std::vector<std::string> directoryList;
	std::vector<std::vector<std::string>> trcList;

	Ui::LocaGUIClass ui;
};

#endif // LOCAGUI_H
