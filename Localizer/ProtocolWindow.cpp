#include "ProtocolWindow.h"

ProtocolWindow::ProtocolWindow(InsermLibrary::ProvFile& prov, QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.BlocsListWidget, &QListWidget::itemDoubleClicked, this, &ProtocolWindow::OnBlocDoubleClicked);
    connect(ui.AddBlocPushButton, &QPushButton::clicked, this, &ProtocolWindow::AddElement);
    connect(ui.RemoveBlocPushButton, &QPushButton::clicked, this, &ProtocolWindow::RemoveElement);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &ProtocolWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_file = &prov;//new InsermLibrary::ProvFile(protocolPath.toStdString());

    ui.ProtocolNameLineEdit->setText(m_file->Name().c_str());
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
        m_BlocIndex = indexes[0].row();

        m_memoryBloc = InsermLibrary::Bloc(m_file->Blocs()[m_BlocIndex]);
        BlocWindow* blocWindow = new BlocWindow(m_file->Blocs()[m_BlocIndex], this);
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
    std::cout << "OnProtocolWindowAccepted" << std::endl;
    //Update UI
}

void ProtocolWindow::OnProtocolWindowRejected()
{
    std::cout << "OnProtocolWindowRejected" << std::endl;
    m_file->Blocs()[m_BlocIndex] = InsermLibrary::Bloc(m_memoryBloc);
}

void ProtocolWindow::ValidateModifications()
{
    m_file->Name(ui.ProtocolNameLineEdit->text().toStdString());

    done(1);
    close();
}
