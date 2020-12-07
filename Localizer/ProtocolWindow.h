#ifndef PROTOCOLWINDOW_H
#define PROTOCOLWINDOW_H

#include "ui_ProtocolWindow.h"

#include <QtWidgets/QDialog>
#include <QDir>
#include <QMenu>
#include <QList>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>
#include <QModelIndex>
#include <QInputDialog>
#include <QTableWidget>
#include <QAbstractItemModel>
#include <QStandardItemModel>

#include <iostream>
#include "Utility.h"
#include "ProtocolFile.h"

class ProtocolWindow : public QDialog
{
	Q_OBJECT

public:
    ProtocolWindow(QWidget *parent = nullptr);
    ~ProtocolWindow();

private:
	void ConnectSignals();
	void InitUiParameters();
	QStringList GetFilesFromRootFolder(QString fileExt);
    void LoadProtocolsInUI(QStringList protocols);

private slots:
    void ShowLocaListContextMenu(QPoint point);
    void UpdateProtocolOnRowChanged(const QModelIndex current, const QModelIndex previous);
    void ShowProvTabContextMenu(QPoint point);
    void AddConditionToExperiment(const QModelIndex index);
    void RemoveConditionFromExperiment();
    void ManageChangeItem(QStandardItem* item);
    void SaveAllProtocols();

private:
    QList<ProtocolFile*> files;
	QStringList m_listHeaderProv;
    bool m_dataChanged = false;
    QString m_provFolder = QCoreApplication::applicationDirPath() + "/Resources/Config/Prov";
	Ui::FormLoca ui;
};

#endif
