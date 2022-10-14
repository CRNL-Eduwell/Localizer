#include "BlocWindow.h"

BlocWindow::BlocWindow(InsermLibrary::Bloc& bloc, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.SubBlocsListWidget, &QListWidget::itemDoubleClicked, this, &BlocWindow::OnSubBlocDoubleClicked);
    connect(ui.AddSubBlocPushButton, &QPushButton::clicked, this, &BlocWindow::AddElement);
    connect(ui.RemoveSubBlocPushButton, &QPushButton::clicked, this, &BlocWindow::RemoveElement);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &BlocWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_bloc = &bloc;

    ui.BlocNameLineEdit->setText(m_bloc->Name().c_str());
    ui.OrderLineEdit->setText(QString::number(m_bloc->Order()));
    ui.SortLineEdit->setText(m_bloc->Sort().c_str());
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

void BlocWindow::UpdateSubBlocsDisplay(int index)
{

}

void BlocWindow::OnSubBlocDoubleClicked()
{
    QModelIndexList indexes = ui.SubBlocsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_subBlocIndex = indexes[0].row();

        m_memorySubbloc = InsermLibrary::SubBloc(m_bloc->SubBlocs()[m_subBlocIndex]);
        SubBlocWindow* blocWindow = new SubBlocWindow(m_bloc->SubBlocs()[m_subBlocIndex], this);
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
    ui.SubBlocsListWidget->item(m_subBlocIndex)->setText(m_bloc->SubBlocs()[m_subBlocIndex].Name().c_str());
}

void BlocWindow::OnSubBlocWindowRejected()
{
    std::cout << "OnSubBlocWindowRejected" << std::endl;
    m_bloc->SubBlocs()[m_subBlocIndex] = InsermLibrary::SubBloc(m_memorySubbloc);
}

void BlocWindow::ValidateModifications()
{
    m_bloc->Name(ui.BlocNameLineEdit->text().toStdString());
    m_bloc->Order(ui.OrderLineEdit->text().toInt());
    m_bloc->Sort(ui.SortLineEdit->text().toStdString());

    done(1);
    close();
}
