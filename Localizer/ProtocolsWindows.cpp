#include "ProtocolsWindow.h"

ProtocolsWindow::ProtocolsWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.ProtocolListWidget, &QListWidget::itemDoubleClicked, this, &ProtocolsWindow::OnProtocolDoubleClicked);
    connect(ui.AddProtocolPushButton, &QPushButton::clicked, this, &ProtocolsWindow::AddElement);
    connect(ui.RemoveProtocolPushButton, &QPushButton::clicked, this, &ProtocolsWindow::RemoveElement);
    //===
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &ProtocolsWindow::ValidateAndSave);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    // Get Data
    QStringList protocolList = GetProtocolsFileList();
    for(int i = 0; i < protocolList.size(); i++)
    {
        QString protocolPath = m_ProtocolFolder + "/" + protocolList[i];
        m_ProvFiles.push_back(InsermLibrary::ProvFile(protocolPath.toStdString()));
    }
    LoadProtocols(m_ProvFiles);
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

void ProtocolsWindow::LoadProtocols(std::vector<InsermLibrary::ProvFile> protocols)
{
    for (int i = 0; i < protocols.size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.ProtocolListWidget);
        currentPROV->setText(protocols[i].Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void ProtocolsWindow::OnProtocolDoubleClicked(QListWidgetItem* item)
{
    QModelIndexList indexes = item->listWidget()->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_fileIndex = indexes[0].row();

        m_memoryFile = InsermLibrary::ProvFile(m_ProvFiles[m_fileIndex]);
        ProtocolWindow* protocolsWindow = new ProtocolWindow(m_ProvFiles[m_fileIndex], this);
        protocolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        protocolsWindow->show();

        connect(protocolsWindow, &ProtocolWindow::accepted, this, &ProtocolsWindow::OnProtocolWindowAccepted);
        connect(protocolsWindow, &ProtocolWindow::rejected, this, &ProtocolsWindow::OnProtocolWindowRejected);

    }
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
    ui.ProtocolListWidget->item(m_fileIndex)->setText(m_ProvFiles[m_fileIndex].Name().c_str());
}

void ProtocolsWindow::OnProtocolWindowRejected()
{
    std::cout << "Rejected" << std::endl;
    m_ProvFiles[m_fileIndex] = InsermLibrary::ProvFile(m_memoryFile);
}

void ProtocolsWindow::ValidateAndSave()
{
    QStringList protocolList = GetProtocolsFileList();
    for(int i = 0; i < protocolList.size(); i++)
    {
        QString protocolPathFromFolder = m_ProtocolFolder + "/" + protocolList[i];
        auto it = std::find_if(m_ProvFiles.begin(), m_ProvFiles.end(), [&](InsermLibrary::ProvFile p)
        {
            QString pathFromFile = m_ProtocolFolder + "/" + p.Name().c_str() + ".prov";

            return pathFromFile == protocolPathFromFolder;
        });
        if(it == m_ProvFiles.end())
        {
            qDebug() << "Deleting " << protocolPathFromFolder;
            QFile::remove(protocolPathFromFolder);
        }
    }

    for(int i = 0; i < m_ProvFiles.size(); i++)
    {
        QString path = m_ProtocolFolder + "/" + m_ProvFiles[i].Name().c_str() + ".prov";
        m_ProvFiles[i].SaveAs(path.toStdString());
    }

    done(1);
    accept();
}
