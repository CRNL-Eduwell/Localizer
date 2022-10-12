#include "SubBlocWindow.h"

SubBlocWindow::SubBlocWindow(InsermLibrary::SubBloc subbloc, QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    //Event related
    connect(ui.EventsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnEventDoubleClicked);

    //Icon related
    connect(ui.IconsListWidget, &QListWidget::itemDoubleClicked, this, &SubBlocWindow::OnIconDoubleClicked);

    //===
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::accepted, this, &SubBlocWindow::ValidateModifications);
    connect(ui.OkCancelButtonBox, &QDialogButtonBox::rejected, this, [&] { close(); });

    m_subbloc = new InsermLibrary::SubBloc(subbloc);
    LoadEvents();
    LoadIcons();
}

SubBlocWindow::~SubBlocWindow()
{

}

void SubBlocWindow::LoadEvents()
{
    QListWidgetItem* currentPROV = new QListWidgetItem(ui.EventsListWidget);
    currentPROV->setText(m_subbloc->MainEvent().Name().c_str());
    currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsSelectable);

    for (int i = 0; i < m_subbloc->SecondaryEvents().size(); i++)
    {
        QListWidgetItem* currentPROV = new QListWidgetItem(ui.EventsListWidget);
        currentPROV->setText(m_subbloc->SecondaryEvents()[i].Name().c_str());
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
        EventWindow* blocWindow = nullptr;

        int eventIndex = indexes[0].row();
        if (eventIndex == 0)
        {
            blocWindow = new EventWindow(m_subbloc->MainEvent(), this);
        }
        else
        {
            blocWindow = new EventWindow(m_subbloc->SecondaryEvents()[eventIndex - 1], this);
        }
        blocWindow->setAttribute(Qt::WA_DeleteOnClose);
        blocWindow->show();

        connect(blocWindow, &EventWindow::SendModifiedData, this, &SubBlocWindow::OnEventWindowAccepted);
        connect(blocWindow, &EventWindow::rejected, this, &SubBlocWindow::OnEventWindowRejected);
    }
}

void SubBlocWindow::OnIconDoubleClicked()
{

}

void SubBlocWindow::OnEventWindowAccepted(std::string name, std::vector<int> codes, InsermLibrary::MainSecondaryEnum type)
{

}

void SubBlocWindow::OnEventWindowRejected()
{

}

void SubBlocWindow::ValidateModifications()
{
    done(1);
    close();
}