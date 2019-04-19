#ifndef CONCATENATOR_H
#define CONCATENATOR_H

#include <QtWidgets/QDialog>
#include <QDir>
#include <QTableWidget>
#include <QMessageBox>
#include <iostream>
#include "Worker.h"
#include <QThread>
#include "ui_concatenator.h"

class concatenator : public QDialog
{
	Q_OBJECT

public:
	concatenator(std::string rootFolder, QWidget *parent = 0);
	~concatenator();

private slots:
	void checkOnEnter(QListWidgetItem * item);
	void concatenateFiles();

signals:
	void sendLogInfo(QString);

private :
	void updateTRCList();

private:
	Ui::FormConcat ui;
	std::string m_rootFolder;
	QThread* thread = nullptr;
	Worker* worker = nullptr;
	bool isAlreadyRunning = false;
};

#endif