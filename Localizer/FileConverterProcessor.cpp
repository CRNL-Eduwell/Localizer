#include "FileConverterProcessor.h"

FileConverterProcessor::FileConverterProcessor(QList<QString> fileList, QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.cancelButton, &QPushButton::clicked, this, &FileConverterProcessor::CancelProcess);
	connect(ui.okButton, &QPushButton::clicked, this, &FileConverterProcessor::ValidateFiles);

	QStringList fileTypeName;
	fileTypeName << "Elan" << "BrainVision";

	LoadFileList(fileList);
    LoadFileTypeList(fileTypeName);
	ResizeColumns();
}

FileConverterProcessor::~FileConverterProcessor()
{

}

void FileConverterProcessor::CancelProcess()
{
	done(0);
	close();
}

void FileConverterProcessor::ValidateFiles()
{
	QStringList newFileTypes;
	for (int i = 0; i < ui.NewFileListWidget->count(); i++)
	{
		QComboBox* currentbox = (QComboBox*)ui.NewFileListWidget->itemWidget(ui.NewFileListWidget->item(i));
		if (currentbox->currentText().compare("Elan") == 0)
		{
			newFileTypes.push_back("eeg");
		}
		else if (currentbox->currentText().compare("BrainVision") == 0)
		{
			newFileTypes.push_back("vhdr");
		}
		else
		{
			newFileTypes.push_back("");
		}
	}
	SendExamCorrespondance(newFileTypes);

	done(1);
	close();
}

void FileConverterProcessor::LoadFileList(QList<QString> fileList)
{
	int fileCount = fileList.size();
	for (int i = 0; i < fileCount; i++)
	{
		QFileInfo fileInfo = QFileInfo(fileList[i]);

		QListWidgetItem *item = new QListWidgetItem(ui.FileListWidget);
		item->setText(fileInfo.fileName());
		item->setFlags(item->flags() | Qt::NoItemFlags);
	}
}

void FileConverterProcessor::LoadFileTypeList(QStringList examList)
{
	examList.insert(0, "UNKNOWN");
	int examFileCount = examList.size();
	int fileCount = ui.FileListWidget->count();

	for (int i = 0; i < fileCount; i++)
	{
		QComboBox *comboBox = new QComboBox(ui.NewFileListWidget);
		for (int j = 0; j < examFileCount; j++)
		{
			comboBox->addItem(examList[j]);
		}
		comboBox->setCurrentIndex(0);

		QListWidgetItem *item = new QListWidgetItem(ui.NewFileListWidget);
		ui.NewFileListWidget->setItemWidget(item, comboBox);
	}
}

void FileConverterProcessor::ResizeColumns()
{
	QFontMetrics * fm = new QFontMetrics(ui.FileListWidget->item(0)->font());
	int width = fm->width(ui.FileListWidget->item(0)->text()) + 25;
	ui.FileListWidget->setMinimumWidth(width);
	ui.NewFileListWidget->setMinimumWidth(width);
	delete fm;
}
