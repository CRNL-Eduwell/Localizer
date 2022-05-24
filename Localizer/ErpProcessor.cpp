#include "ErpProcessor.h"

ErpProcessor::ErpProcessor(QList<QString> fileList, QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.cancelButton, &QPushButton::clicked, this, &ErpProcessor::CancelProcess);
	connect(ui.okButton, &QPushButton::clicked, this, &ErpProcessor::ValidateFiles);

	QStringList provNames = GetProvList();
	LoadFileList(fileList);
	LoadProvList(fileList, provNames);
	ResizeColumns();
}

ErpProcessor::~ErpProcessor()
{

}

void ErpProcessor::CancelProcess()
{
	done(0);
	close();
}

void ErpProcessor::ValidateFiles()
{
	QStringList exams;
	for (int i = 0; i < ui.ExamListWidget->count(); i++)
	{
		QComboBox* currentbox = (QComboBox*)ui.ExamListWidget->itemWidget(ui.ExamListWidget->item(i));
		exams.push_back(currentbox->currentText());
	}
	SendExamCorrespondance(exams);
	done(1);
	close();
}


QStringList ErpProcessor::GetProvList()
{
    QString rootProvFolder(QCoreApplication::applicationDirPath() + "/Resources/Config/Prov");
	QDir currentDir(rootProvFolder);
	currentDir.setFilter(QDir::Files);
	currentDir.setNameFilters(QStringList() << "*.prov");

	return currentDir.entryList();
}

void ErpProcessor::LoadFileList(QList<QString> fileList)
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

void ErpProcessor::LoadProvList(QList<QString> fileList, QStringList examList)
{
	examList.insert(0, "UNKNOWN");
	int examFileCount = examList.size();
	int fileCount = ui.FileListWidget->count();

	for (int i = 0; i < fileCount; i++)
	{
		QComboBox *comboBox = new QComboBox(ui.ExamListWidget);
		for (int j = 0; j < examFileCount; j++)
		{
			comboBox->addItem(examList[j]);
		}

		std::string fileName = EEGFormat::Utility::GetFileName(fileList[i].toStdString(), false);
		std::vector<std::string> fileNameSplit = EEGFormat::Utility::Split<std::string>(fileName, "_");
		std::string examSuffix = fileNameSplit.size() > 0 ? fileNameSplit[fileNameSplit.size() - 1] : "UNKNOWN";
		
		QStringList matches = examList.filter(QString::fromStdString(examSuffix), Qt::CaseInsensitive);
		matches.size() > 0 ? comboBox->setCurrentIndex(examList.indexOf(matches[0])) : comboBox->setCurrentIndex(0);

		QListWidgetItem *item = new QListWidgetItem(ui.ExamListWidget);
		ui.ExamListWidget->setItemWidget(item, comboBox);
	}
}

void ErpProcessor::ResizeColumns()
{
	QFontMetrics * fm = new QFontMetrics(ui.FileListWidget->item(0)->font());
    int width = fm->horizontalAdvance(ui.FileListWidget->item(0)->text());// +25;
	ui.FileListWidget->setMinimumWidth(width);
	ui.ExamListWidget->setMinimumWidth(width);
	delete fm;
}
