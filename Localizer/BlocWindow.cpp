#include "BlocWindow.h"

BlocWindow::BlocWindow(InsermLibrary::Bloc bloc, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.SubBlocsListWidget, &QListWidget::itemDoubleClicked, this, &BlocWindow::OnSubBlocDoubleClicked);
    connect(ui.AddSubBlocPushButton, &QPushButton::clicked, this, &BlocWindow::AddElement);
    connect(ui.RemoveSubBlocPushButton, &QPushButton::clicked, this, &BlocWindow::RemoveElement);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &BlocWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_bloc = new InsermLibrary::Bloc(bloc);
    LoadSubBlocs();
}

BlocWindow::~BlocWindow()
{

}

void BlocWindow::LoadSubBlocs()
{
    for (int i = 0; i < m_bloc->SubBlocs().size(); i++)
    {
        InsermLibrary::SubBloc subbloc = m_bloc->SubBlocs()[i];

        QListWidgetItem* currentPROV = new QListWidgetItem(ui.SubBlocsListWidget);
        currentPROV->setText(subbloc.Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void BlocWindow::OnSubBlocDoubleClicked()
{
    QModelIndexList indexes = ui.SubBlocsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        int subBlocIndex = indexes[0].row();

        SubBlocWindow* blocWindow = new SubBlocWindow(m_bloc->SubBlocs()[subBlocIndex], this);
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &SubBlocWindow::accepted, this, &BlocWindow::OnSubBlocWindowAccepted);
        connect(blocWindow, &SubBlocWindow::rejected, this, &BlocWindow::OnSubBlocWindowRejected);
    }
}

void BlocWindow::AddElement()
{

}

void BlocWindow::RemoveElement()
{

}

void BlocWindow::OnSubBlocWindowAccepted()
{
    std::cout << "OnSubBlocWindowAccepted" << std::endl;
}

void BlocWindow::OnSubBlocWindowRejected()
{
    std::cout << "OnSubBlocWindowRejected" << std::endl;
}

void BlocWindow::ValidateModifications()
{
    done(1);
    close();
}