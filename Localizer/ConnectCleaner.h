#ifndef CONNECT_CLEANER_H
#define CONNECT_CLEANER_H

#include "ui_ConnectCleaner.h"
#include <QtWidgets/QDialog>
#include <QLineEdit>
#include "ElectrodeCleanerEdit.h"
#include <iostream>
#include <vector>
#include "eegContainer.h"

class ConnectCleaner : public QDialog
{
	Q_OBJECT

public:
	ConnectCleaner(InsermLibrary::eegContainer* eegCont, QWidget *parent = 0);
	~ConnectCleaner();


public slots:
	void CheckMultipleItems(QListWidgetItem * item);
	void CheckOnEnter(QListWidgetItem * item);
	//void createBipoles();

private:
	void FillSelectedList(const std::vector<std::string> & labels);
	void FillCorrectedList(const std::vector<std::string> & labels);
	void connectSignals();

private:
	std::vector<std::string> m_ElectrodesLabel;
	Ui::ConnectCleanerForm ui;
};

#endif
