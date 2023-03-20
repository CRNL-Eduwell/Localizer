#include "ChooseLocaWindow.h"

ChooseLocaWindow::ChooseLocaWindow(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	QStringList provFiles = GetFilesFromRootFolder("*.prov");
	LoadProtocolsInUI(provFiles);

    connect(ui.SelectUnselectAllCheckBox, &QCheckBox::clicked, this, &ChooseLocaWindow::SelectUnselectAll);
    connect(ui.ValidatePushButton, &QPushButton::clicked, this, &ChooseLocaWindow::ValidateExperimentList);
}

ChooseLocaWindow::~ChooseLocaWindow()
{

}

QStringList ChooseLocaWindow::GetFilesFromRootFolder(QString fileExt)
{
    QDir currentDir(m_provFolder);
    currentDir.setFilter(QDir::Files);
    currentDir.setNameFilters(QStringList() << fileExt);
    QStringList allFiles = currentDir.entryList();

    QStringList matchingFiles;
    QRegularExpression rxProv("^[a-zA-Z]+[0-9]*.prov$");
    for (int i = 0; i < allFiles.size(); i++)
    {
        if (rxProv.match(allFiles[i]).hasMatch())
        {
            matchingFiles.push_back(allFiles[i]);
        }
    }

    return matchingFiles;
}

void ChooseLocaWindow::LoadProtocolsInUI(QStringList protocols)
{
    for (int i = 0; i < protocols.size(); i++)
    {
        QString protocolLabel = protocols[i];
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.listProv);
        currentPROV->setText(protocolLabel.replace(".prov", ""));
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);

        currentPROV->setCheckState(GetDefaultState(protocolLabel));
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsUserCheckable); //Allow to intercept enter key for modification
        currentPROV->setFlags(currentPROV->flags() ^ Qt::ItemIsEditable); // Item not editable since we have correctedLabel_item
    }
}

Qt::CheckState ChooseLocaWindow::GetDefaultState(QString label)
{
    if (label == "ARFA" || label == "AUDI" || label == "LEC1" || label == "LEC2" || label == "MCSE" || label == "MOTO" || label == "MVEB" || label == "MVIS" || label == "REST" || label == "VISU")
        return Qt::CheckState::Checked;
    else
        return Qt::CheckState::Unchecked;
}

void ChooseLocaWindow::SelectUnselectAll(bool checked)
{
    Qt::CheckState checkSate = checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    for(int i = 0; i < ui.listProv->count(); i++)
    {
        ui.listProv->item(i)->setCheckState(checkSate);
    }
}

void ChooseLocaWindow::ValidateExperimentList()
{
    for (int i = 0; i < ui.listProv->count();i++)
    {
        if (ui.listProv->item(i)->checkState() == Qt::CheckState::Checked)
        {
            m_ElementsToLookFor.push_back(ui.listProv->item(i)->text().toStdString());
        }
    }

    done(1);
    close();
}
