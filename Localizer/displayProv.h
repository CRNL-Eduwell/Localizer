#ifndef DISPLAYPROV_H
#define DISPLAYPROV_H

#include "ui_displayprov.h"

#include <QCoreApplication>
#include <QtWidgets/QDialog>
#include <QString>
#include <QList>
#include <QDir>
#include <QRegularExpression>

#include <iostream>
#include <vector>

class displayProv : public QDialog
{
	Q_OBJECT

public:
	displayProv(std::vector<QString> currentList, QWidget *parent = 0);
	~displayProv();
private:
	void getProvList();
	void displayProvList(std::vector<QString> currentList);
	void connectSignals();

private slots:
	void checkMultipleItems(QListWidgetItem * item);
	void checkOnEnter(QListWidgetItem * item);
	void sendListAndClose();
signals:
	void sendProvList(std::vector<QString> truc);

private:
	std::vector<QString> prov;
	Ui::FormDispProv ui;
};


#endif
