#ifndef ERPPROCESSOR_H
#define ERPPROCESSOR_H

#include "ui_ErpProcessor.h"
#include "../../EEGFormat/EEGFormat/Utility.h"
#include <QDir>
#include <QList>
#include <QComboBox>
#include <QFileInfo>
#include <QtWidgets/QDialog>

class ErpProcessor : public QDialog
{
	Q_OBJECT

public:
	ErpProcessor(QList<QString> fileList, QWidget *parent = 0);
	~ErpProcessor();

private slots:
	void CancelProcess();
	void ValidateFiles();

signals:
	void SendExamCorrespondance(QList<QString>);

private:
	QStringList GetProvList();
	void LoadFileList(QList<QString> fileList);
	void LoadProvList(QList<QString> fileList, QStringList examList);

private:
	Ui::Dialog ui;
};

#endif