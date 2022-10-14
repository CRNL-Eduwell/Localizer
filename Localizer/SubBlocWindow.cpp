#include "SubBlocWindow.h"
#include "EventWindow.h"

SubBlocWindow::SubBlocWindow(InsermLibrary::SubBloc& subbloc, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    //Event related
    connect(ui.EventsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnEventDoubleClicked);
    //Icon related
    connect(ui.IconsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnIconDoubleClicked);
    //===
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &SubBlocWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_subbloc = &subbloc;

    ui.SubBlocNameLineEdit->setText(m_subbloc->Name().c_str());
    ui.OrderLineEdit->setText(QString::number(m_subbloc->Order()));
    //dropdown type
    ui.WindowStartLineEdit->setText(QString::number(m_subbloc->MainWindow().Start()));
    ui.WindowEndLineEdit->setText(QString::number(m_subbloc->MainWindow().End()));
    ui.BaselineStartLineEdit->setText(QString::number(m_subbloc->Baseline().Start()));
    ui.BaselineEndLineEdit->setText(QString::number(m_subbloc->Baseline().End()));

    LoadEvents();
    LoadIcons();
}

SubBlocWindow::~SubBlocWindow()
{

}

void SubBlocWindow::LoadInUi()
{

}

void SubBlocWindow::UpdateEventDisplay(int index, std::string name)
{
    ui.EventsListWidget->item(index)->setText(name.c_str());
}

void SubBlocWindow::LoadEvents()
{
    for (int i = 0; i < m_subbloc->Events().size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.EventsListWidget);
        currentPROV->setText(m_subbloc->Events()[i].Name().c_str());
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);
    }
}

void SubBlocWindow::LoadIcons()
{

}

void SubBlocWindow::OnEventDoubleClicked()
{
    QModelIndexList indexes = ui.EventsListWidget->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty())
    {
        m_IndexOfEvent = indexes[0].row();

        m_memoryEvent = InsermLibrary::Event(m_subbloc->Events()[m_IndexOfEvent]);
        EventWindow* blocWindow = new EventWindow(m_subbloc->Events()[m_IndexOfEvent], this);
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &EventWindow::accepted, this, &SubBlocWindow::OnEventWindowAccepted);
        connect(blocWindow, &EventWindow::rejected, this, &SubBlocWindow::OnEventWindowRejected);
    }
}

void SubBlocWindow::OnIconDoubleClicked()
{

}

void SubBlocWindow::OnEventWindowAccepted()
{
    std::cout << "OnEventWindowAccepted" << std::endl;
    UpdateEventDisplay(m_IndexOfEvent, m_subbloc->Events()[m_IndexOfEvent].Name());
}

void SubBlocWindow::OnEventWindowRejected()
{
    std::cout << "OnEventWindowRejected" << std::endl;
    m_subbloc->Events()[m_IndexOfEvent] = InsermLibrary::Event(m_memoryEvent);
}

void SubBlocWindow::ValidateModifications()
{    
    m_subbloc->Name(ui.SubBlocNameLineEdit->text().toStdString());
    m_subbloc->Order(ui.OrderLineEdit->text().toInt());
    InsermLibrary::MainSecondaryEnum type = ui.TypeComboBox->currentIndex() == 0 ? InsermLibrary::MainSecondaryEnum::Main : InsermLibrary::MainSecondaryEnum::Secondary;
    m_subbloc->Type(type);
    m_subbloc->MainWindow().Start(ui.WindowStartLineEdit->text().toInt());
    m_subbloc->MainWindow().End(ui.WindowEndLineEdit->text().toInt());
    m_subbloc->Baseline().Start(ui.BaselineStartLineEdit->text().toInt());
    m_subbloc->Baseline().End(ui.BaselineEndLineEdit->text().toInt());

    done(1);
    close();
}
