#include "GeneralOptionsWindow.h"

using namespace InsermLibrary;

GeneralOptionsWindow::GeneralOptionsWindow(GeneralOptionsFile *file, QWidget *parent) : QDialog(parent)
{
	m_GeneralOptionsFile = file;

    ui.setupUi(this);
	if (m_GeneralOptionsFile != nullptr)
	{
		LoadFileTypeInUi(m_GeneralOptionsFile->FileExtensionsFavorite());
		ConnectSignals();
	}
	else
	{
		QMessageBox::critical(this, "File Missing", "There should be an option file at in /Resources/Config/generalOptions.txt");
		close();
	}

}

GeneralOptionsWindow::~GeneralOptionsWindow()
{

}

void GeneralOptionsWindow::ConnectSignals()
{
	connect(ui.MoveUpButton, &QPushButton::clicked, this, &GeneralOptionsWindow::MoveElementUp);
	connect(ui.MoveDownButton, &QPushButton::clicked, this, &GeneralOptionsWindow::MoveElementDown);
	connect(ui.SaveCancelButtonbox, &QDialogButtonBox::accepted, this, [&] {m_GeneralOptionsFile->Save(); close(); });
    connect(ui.SaveCancelButtonbox, &QDialogButtonBox::rejected, this, [&] { close(); });
}

void GeneralOptionsWindow::LoadFileTypeInUi(std::vector<InsermLibrary::FileExt> fileTypes)
{
    for(int i = 0; i < fileTypes.size(); i++)
    {
        QListWidgetItem *currentFileType = new QListWidgetItem(ui.listWidget_fileTypes);

		switch (fileTypes[i])
		{
			case InsermLibrary::FileExt::TRC:
			{
				currentFileType->setText("Micromed");
				break;
			}
			case InsermLibrary::FileExt::EEG_ELAN:
			{				
				currentFileType->setText("Elan");
				break;
			}
			case InsermLibrary::FileExt::BRAINVISION:
			{
				currentFileType->setText("BrainVision");
				break;
			}
			case InsermLibrary::FileExt::EDF:
			{
				currentFileType->setText("European Data Format");
				break;
			}
		}
		currentFileType->setFlags(currentFileType->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    }
}

void GeneralOptionsWindow::MoveElementUp()
{
	QModelIndexList selectedIndexes = ui.listWidget_fileTypes->selectionModel()->selectedRows();
	if (selectedIndexes.size() == 1)
	{
		int currentIndex = selectedIndexes[0].row();
		if (currentIndex - 1 >= 0)
		{
			QListWidgetItem *currentItem = ui.listWidget_fileTypes->takeItem(currentIndex);
			ui.listWidget_fileTypes->insertItem(currentIndex - 1, currentItem);
			ui.listWidget_fileTypes->setCurrentRow(currentIndex - 1);
			std::swap(m_GeneralOptionsFile->FileExtensionsFavorite()[currentIndex], m_GeneralOptionsFile->FileExtensionsFavorite()[currentIndex - 1]);
		}
	}
}

void GeneralOptionsWindow::MoveElementDown()
{
	QModelIndexList selectedIndexes = ui.listWidget_fileTypes->selectionModel()->selectedRows();
	if (selectedIndexes.size() == 1)
	{
		int currentIndex = selectedIndexes[0].row();
		if (currentIndex + 1 <= ui.listWidget_fileTypes->count())
		{
			QListWidgetItem *currentItem = ui.listWidget_fileTypes->takeItem(currentIndex);
			ui.listWidget_fileTypes->insertItem(currentIndex + 1, currentItem);
			ui.listWidget_fileTypes->setCurrentRow(currentIndex + 1);
			std::swap(m_GeneralOptionsFile->FileExtensionsFavorite()[currentIndex], m_GeneralOptionsFile->FileExtensionsFavorite()[currentIndex + 1]);
		}
	}
}