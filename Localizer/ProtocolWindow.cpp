#include "ProtocolWindow.h"

ProtocolWindow::ProtocolWindow(QString protocolPath, QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.BlocsListWidget, &QListWidget::itemDoubleClicked, this, &ProtocolWindow::OnBlocDoubleClicked);
    connect(ui.AddBlocPushButton, &QPushButton::clicked, this, &ProtocolWindow::AddElement);
    connect(ui.RemoveBlocPushButton, &QPushButton::clicked, this, &ProtocolWindow::RemoveElement);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &ProtocolWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_file = new InsermLibrary::ProvFile(protocolPath.toStdString());
    LoadBlocs();
}

ProtocolWindow::~ProtocolWindow()
{

}

void ProtocolWindow::LoadBlocs()
{
    for (int i = 0; i < m_file->Blocs().size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.BlocsListWidget);
        currentPROV->setText(m_file->Blocs()[i].Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void ProtocolWindow::OnBlocDoubleClicked()
{
    QModelIndexList indexes = ui.BlocsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int blocIndex = indexes[0].row();

        BlocWindow* blocWindow = new BlocWindow(m_file->Blocs()[blocIndex], this);
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &BlocWindow::accepted, this, &ProtocolWindow::OnProtocolWindowAccepted);
        connect(blocWindow, &BlocWindow::rejected, this, &ProtocolWindow::OnProtocolWindowRejected);
    }
}

void ProtocolWindow::AddElement()
{

}

void ProtocolWindow::RemoveElement()
{

}

void ProtocolWindow::OnProtocolWindowAccepted()
{
    std::cout << "Accepted" << std::endl;
}

void ProtocolWindow::OnProtocolWindowRejected()
{
    std::cout << "Rejected" << std::endl;
}

void ProtocolWindow::ValidateModifications()
{
    done(1);
    close();
}