#include "ProtocolsWindow.h"

ProtocolsWindow::ProtocolsWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.ProtocolListWidget, &QListWidget::itemDoubleClicked, this, &ProtocolsWindow::OnProtocolDoubleClicked);
    connect(ui.AddProtocolPushButton, &QPushButton::clicked, this, &ProtocolsWindow::AddElement);
    connect(ui.RemoveProtocolPushButton, &QPushButton::clicked, this, &ProtocolsWindow::RemoveElement);

    // Get Data
    QStringList protocolList = GetProtocolsFileList();
    LoadProtocols(protocolList);
}

ProtocolsWindow::~ProtocolsWindow()
{

}

QStringList ProtocolsWindow::GetProtocolsFileList()
{
    QDir currentDir(m_ProtocolFolder);
    currentDir.setFilter(QDir::Files);
    currentDir.setNameFilters(QStringList() << "*.prov");
    return currentDir.entryList();
}

void ProtocolsWindow::LoadProtocols(QStringList protocols)
{
    for (int i = 0; i < protocols.size(); i++)
    {
        QString protocolLabel = protocols[i];
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.ProtocolListWidget);
        currentPROV->setText(protocolLabel.replace(".prov", ""));
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void ProtocolsWindow::OnProtocolDoubleClicked(QListWidgetItem* item)
{
    QString protocolPath = m_ProtocolFolder + "/" + item->text() + ".prov";

    ProtocolWindow* protocolsWindow = new ProtocolWindow(protocolPath, this);
    protocolsWindow->setAttribute(Qt::WA_DeleteOnClose);
    protocolsWindow->show();

    connect(protocolsWindow, &ProtocolWindow::accepted, this, &ProtocolsWindow::OnProtocolWindowAccepted);
    connect(protocolsWindow, &ProtocolWindow::rejected, this, &ProtocolsWindow::OnProtocolWindowRejected);
}

void ProtocolsWindow::AddElement()
{
    QModelIndexList indexes = ui.ProtocolListWidget->selectionModel()->selectedIndexes();
    int insertionIndex = !indexes.isEmpty() ? indexes[0].row() + 1 : ui.ProtocolListWidget->count();
    ui.ProtocolListWidget->insertItem(insertionIndex, "New Protocol");
    //add new logic file to list 
}

void ProtocolsWindow::RemoveElement()
{
    QModelIndexList indexes = ui.ProtocolListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int indexToDelete = indexes[0].row();
        ui.ProtocolListWidget->item(indexToDelete)->~QListWidgetItem();
        //remove logic file from list
        //wantedLocaKW.erase(wantedLocaKW.begin() + rowToDelete);
    }
}

void ProtocolsWindow::OnProtocolWindowAccepted()
{
    std::cout << "Accepted" << std::endl;
}

void ProtocolsWindow::OnProtocolWindowRejected()
{
    std::cout << "Rejected" << std::endl;
}
