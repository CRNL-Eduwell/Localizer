#ifndef CONNECT_CLEANER_H
#define CONNECT_CLEANER_H

#include "ui_ConnectCleaner.h"
#include <QtWidgets/QDialog>
#include <QLineEdit>
#include <iostream>
#include <string>
#include <vector>
#include "eegContainer.h"
#include "CleanConnectFile.h"
#include <QTableView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QKeyEvent>

class ConnectCleaner : public QDialog
{
	Q_OBJECT

public:
	ConnectCleaner(InsermLibrary::eegContainer* eegCont, QWidget *parent = 0);
	~ConnectCleaner();

private:
    void FillList(const std::vector<std::string> & labels);
    void keyPressEvent(QKeyEvent *);

private slots:
    void CheckMultipleItems(QStandardItem *item);
    void CreateBipoles();

private:
    InsermLibrary::eegContainer *containerEeg = nullptr;
	std::vector<std::string> m_ElectrodesLabel;
	Ui::ConnectCleanerForm ui;
    bool m_lockMultiple = false;
};

#endif
