#ifndef FILECONVERTERPROCESSOR_H
#define FILECONVERTERPROCESSOR_H

#include "ui_FileConverterProcessor.h"
#include "../../EEGFormat/EEGFormat/Utility.h"
#include <QDir>
#include <QList>
#include <QComboBox>
#include <QFileInfo>
#include <QtWidgets/QDialog>

class FileConverterProcessor : public QDialog
{
	Q_OBJECT

public:
    FileConverterProcessor(QList<QString> fileList, QWidget *parent = nullptr);
	~FileConverterProcessor();

private slots:
	void CancelProcess();
	void ValidateFiles();

signals:
	void SendExamCorrespondance(QList<QString>);

private:
	void LoadFileList(QList<QString> fileList);
    void LoadFileTypeList(QStringList fileTypeName);
	void ResizeColumns();

private:
	Ui::FileConvertDialog ui;
};

#endif
