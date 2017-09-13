#ifndef DISPLAYPROV_H
#define DISPLAYPROV_H

#include "ui_displayprov.h"

#include <QtWidgets/QDialog>
#include <QString>
#include <QList>
#include <QDir>

#include <iostream>
#include <vector>

using namespace std;

class displayProv : public QDialog
{
	Q_OBJECT

public:
	displayProv(vector<QString> currentList, QWidget *parent = 0);
	~displayProv();
private:
	void getProvList();
	void displayProvList(vector<QString> currentList);
	void connectSignals();

private slots:
	void checkMultipleItems(QListWidgetItem * item);
	void checkOnEnter(QListWidgetItem * item);
	void sendListAndClose();
signals:
	void sendProvList(vector<QString> truc);

private:
	vector<QString> prov;
	Ui::FormDispProv ui;
};


#endif