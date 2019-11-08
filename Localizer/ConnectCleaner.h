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
    ConnectCleaner(InsermLibrary::eegContainer* eegCont, QString connectCleanerFilePath, QWidget *parent = 0);
	~ConnectCleaner();

private:
    void FillList(const std::vector<std::string> & labels);
    void keyPressEvent(QKeyEvent *);

private slots:
    /**
     * @brief When Multiple elements are selected, change all of their state according
     * to the one being modified either by mouse click or key press
     */
    void CheckMultipleItems(QStandardItem *item);
    /**
     * @brief Get the id of unwanted electrodes and correct the labeling of wanted one
     */
    void ValidateConnect();

private:
    InsermLibrary::eegContainer *containerEeg = nullptr;
	std::vector<std::string> m_ElectrodesLabel;
	Ui::ConnectCleanerForm ui;
    bool m_lockMultiple = false;
    QString m_connectCleanerFilePath = "";
    CleanConnectFile *m_cleanConnectFile = nullptr;
};

#endif
