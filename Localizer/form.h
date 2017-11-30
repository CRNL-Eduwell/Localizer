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

class form : public QDialog
{
	Q_OBJECT

public:
	form(QWidget *parent = 0);
	~form();

	void load();

private:
	void defineHorizontalHeader(QTableWidget *modele);
	QStringList getFilesFromRootFolder(QString fileExt);
	void loadProvListUI(QStringList provList);

private slots:
	void displaySelectedProv(QListWidgetItem* item);
	void manageChangeItem(QTableWidgetItem* item);
	void addLoca();
	void removeLoca();
	void addBloc();
	void removeBloc();
	void save();

private:
	QStringList listeHeaderLoca;
	vector<InsermLibrary::PROV> listprov;
	QString provFolder = QDir::currentPath() + "/Resources/Config/Prov";
	Ui::FormLoca ui;
	int initsize = 0;
};


#endif