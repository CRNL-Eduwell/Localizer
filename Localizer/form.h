#ifndef FORM_H
#define FORM_H

#include <QtWidgets/QDialog>
#include <QDir>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QkeyEvent>
#include "ui_form.h"
#include <iostream>
#include "Utility.h"
#include "PROV.h"

#include <QMenu>
#include <QList>
#include <QModelIndex>

#include <QInputDialog>
#include <QDebug>

class form : public QDialog
{
	Q_OBJECT

public:
	form(QWidget *parent = 0);
	~form();

private:
	void ConnectSignals();
	void InitUiParameters();
	void InitProvListUi(QStringList provList);
	void DefineHorizontalHeader(QTableWidget *modele);
	QStringList GetFilesFromRootFolder(QString fileExt);
	void LoadProvTabUi(QString provName);

private slots:
	void ShowLocaListContextMenu(QPoint);
	void ManageChangeItem(QTableWidgetItem* item);
	void ShowProvTabContextMenu(QPoint);
	void save();

private:
	InsermLibrary::PROV *m_currentProv = nullptr;
	QStringList m_listHeaderProv;
	QString m_provFolder = QDir::currentPath() + "/Resources/Config/Prov";
	Ui::FormLoca ui;
};


#endif