#ifndef ELECTRODE_CLEANER_EDIT_H
#define ELECTRODE_CLEANER_EDIT_H

#include "ui_ElectrodeCleanerEdit.h"
#include <QtWidgets/QDialog>
#include <QDialog>
#include <QLineEdit>
#include <iostream>
#include <vector>
#include "eegContainer.h"


class ElectrodeCleanerEdit : public QWidget
{
	Q_OBJECT

public:
	ElectrodeCleanerEdit();
	~ElectrodeCleanerEdit();


//public slots:
//	void CheckMultipleItems(QListWidgetItem * item);
//	void CheckOnEnter(QListWidgetItem * item);
//	//void createBipoles();
//
//private:
//	void FillSelectedList(const std::vector<std::string> & labels);
//	void FillCorrectedList(const std::vector<std::string> & labels);
//	void connectSignals();
//
private:
//	std::vector<std::string> m_ElectrodesLabel;
	Ui::ElectrodeCleanerEditForm ui;
};

#endif
