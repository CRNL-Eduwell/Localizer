#ifndef GENERALOPTIONSWINDOW_H
#define GENERALOPTIONSWINDOW_H

#include "ui_GeneralOptionsWindow.h"

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
#include "GeneralOptionsFile.h"

class GeneralOptionsWindow : public QDialog
{
	Q_OBJECT

public:
	GeneralOptionsWindow(InsermLibrary::GeneralOptionsFile *file, QWidget *parent = nullptr);
    ~GeneralOptionsWindow();

private:
	void ConnectSignals();
    void LoadFileTypeInUi(std::vector<InsermLibrary::FileType> fileTypes);
	void SaveFromModel();

private slots:
	void MoveElementUp();
	void MoveElementDown();

private:
	InsermLibrary::GeneralOptionsFile *m_GeneralOptionsFile = nullptr;
	QString generalOptionsPath = QCoreApplication::applicationDirPath() + "/Resources/Config/generalOptions.txt";
	Ui::FormFileType ui;
};

#endif
