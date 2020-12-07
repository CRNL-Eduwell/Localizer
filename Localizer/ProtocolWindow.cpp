#include "ProtocolWindow.h"

using namespace InsermLibrary;

ProtocolWindow::ProtocolWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    ConnectSignals();
    InitUiParameters();
    QStringList provFiles = GetFilesFromRootFolder("*.prov");
    LoadProtocolsInUI(provFiles);
}

ProtocolWindow::~ProtocolWindow()
{

}

void ProtocolWindow::ConnectSignals()
{
    connect(ui.listWidget_loca, &QHeaderView::customContextMenuRequested, this, &ProtocolWindow::ShowLocaListContextMenu);
    connect(ui.listWidget_loca->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ProtocolWindow::UpdateProtocolOnRowChanged);
    connect(ui.tableView, &QHeaderView::customContextMenuRequested, this, &ProtocolWindow::ShowProvTabContextMenu);
    connect(ui.tableView->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &ProtocolWindow::ShowProvTabContextMenu);
    connect(ui.tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &ProtocolWindow::ShowProvTabContextMenu);
    connect(ui.buttonBox_save, &QDialogButtonBox::accepted, this, &ProtocolWindow::SaveAllProtocols);
    connect(ui.buttonBox_save, &QDialogButtonBox::rejected, this, [&] { close(); });
}

void ProtocolWindow::InitUiParameters()
{
    ui.listWidget_loca->setContextMenuPolicy(Qt::CustomContextMenu);

    ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui.tableView->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    ui.tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);
}

QStringList ProtocolWindow::GetFilesFromRootFolder(QString fileExt)
{
    QDir currentDir(m_provFolder);
    currentDir.setFilter(QDir::Files);
    currentDir.setNameFilters(QStringList() << fileExt);
    return currentDir.entryList();
}

void ProtocolWindow::LoadProtocolsInUI(QStringList protocols)
{
    files = QList<ProtocolFile*>();
    for(int i = 0; i < protocols.size(); i++)
    {
        QString protocolLabel = protocols[i];
        QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
        currentPROV->setText(protocolLabel.replace(".prov", ""));
        currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);

        QString currentPath = m_provFolder + "/" + protocols[i];
        files.push_back(new ProtocolFile(currentPath));
    }

    //Put Model in view
    ui.tableView->setModel(files[0]->Model());
    ui.tableView->resizeColumnsToContents();
    ui.tableView->resizeRowsToContents();

    connect(dynamic_cast<QStandardItemModel*>(ui.tableView->model()), &QStandardItemModel::itemChanged, this, &ProtocolWindow::ManageChangeItem);
}

void ProtocolWindow::ShowLocaListContextMenu(QPoint point)
{
    QModelIndex index = ui.listWidget_loca->indexAt(point);
    QMenu* contextMenu = new QMenu();
    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
    QObject* senderObject = sender();

    QAction* addLocalizerAction = contextMenu->addAction("Add localizer", [this]
    {
        bool ok;
        QString localizerName = QInputDialog::getText(ui.listWidget_loca, "New Name", "Choose Localizer Name", QLineEdit::Normal, "New Localizer", &ok);
        if (ok && !localizerName.isEmpty())
        {
            QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
            currentPROV->setText(localizerName);
            currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);

            QString currentPath = m_provFolder + "/" + localizerName + ".prov";
            files.push_back(new ProtocolFile(currentPath));
        }
    });
    QAction* removeLocalizerAction = contextMenu->addAction("Remove localizer", [this]
    {
        QModelIndexList indexes = ui.listWidget_loca->selectionModel()->selectedIndexes();
        while (!indexes.isEmpty())
        {
            ui.listWidget_loca->model()->removeRows(indexes.last().row(), 1);
            indexes.removeLast();
        }
    });

    // Position
    if (senderObject == ui.listWidget_loca)
        contextMenu->exec(ui.listWidget_loca->viewport()->mapToGlobal(point));
}

void ProtocolWindow::UpdateProtocolOnRowChanged(const QModelIndex current, const QModelIndex previous)
{
    if (m_dataChanged)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Data was modified", "Do you want to save the modifications to this experiment parameters ? ", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            files[previous.row()]->Save();
        }
        m_dataChanged = false;
    }

    QMap<int, QVariant> currentItem = ui.listWidget_loca->model()->itemData(current);
    if (currentItem.count() == 1)
    {
        //Set new model, connect to item change, and update view
        ui.tableView->setModel(files[current.row()]->Model());
        connect(dynamic_cast<QStandardItemModel*>(ui.tableView->model()), &QStandardItemModel::itemChanged, this, &ProtocolWindow::ManageChangeItem);
        ui.tableView->update();
    }
}

void ProtocolWindow::ShowProvTabContextMenu(QPoint point)
{
    QModelIndex index = ui.tableView->indexAt(point);
    QMenu* contextMenu = new QMenu();
    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
    QObject* senderObject = sender();

    // Position
    if (senderObject == ui.tableView)
    {
        QAction* addBlocAction = contextMenu->addAction("Add a condition to this experiment", [&] { AddConditionToExperiment(index); });
        QAction* removeBlocAction = contextMenu->addAction("Remove selected conditions from this experiment", [&] { RemoveConditionFromExperiment(); });
        contextMenu->exec(ui.tableView->viewport()->mapToGlobal(point));
    }
    else if (senderObject == ui.tableView->verticalHeader())
    {
        QAction* addBlocAction = contextMenu->addAction("Add a condition to this experiment", [&] { AddConditionToExperiment(index); });
        QAction* removeBlocAction = contextMenu->addAction("Remove selected conditions from this experiment", [&] { RemoveConditionFromExperiment(); });
        contextMenu->exec(ui.tableView->verticalHeader()->viewport()->mapToGlobal(point));
    }
    else if (senderObject == ui.tableView->horizontalHeader())
    {
        contextMenu->exec(ui.tableView->horizontalHeader()->viewport()->mapToGlobal(point));
    }
}

void ProtocolWindow::AddConditionToExperiment(const QModelIndex index)
{
    bool ok;
    QString conditionName = QInputDialog::getText(ui.tableView, "New Name", "Choose Condition Name", QLineEdit::Normal, "New Condition", &ok);
    if (ok && !conditionName.isEmpty())
    {
        QList<QStandardItem*> itemList;
        for(int i = 0; i < ui.tableView->model()->columnCount(); ++i)
        {
            if(i == 2)
            {
                QStandardItem *item = new QStandardItem(conditionName);
                itemList.append(item);
            }
            else
            {
                itemList.append(new QStandardItem);
            }
        }
        dynamic_cast<QStandardItemModel*>(ui.tableView->model())->insertRow(index.row(), itemList);

        m_dataChanged = true;
    }
}

void ProtocolWindow::RemoveConditionFromExperiment()
{
    QModelIndexList indexes = ui.tableView->selectionModel()->selectedRows();
    while (!indexes.isEmpty())
    {
        ui.tableView->model()->removeRows(indexes.last().row(), 1); //remove from tab
        indexes.removeLast();
    }
    m_dataChanged = true;
}

/**
 * @brief Used to keep element well formated, window information
 *        must be of the [min:max] format
 */
void ProtocolWindow::ManageChangeItem(QStandardItem* index)
{
    switch (index->column())
    {
    case 4:
    {
        QStringList splitData = index->text().split(":");
        if (splitData.count() != 2)
        {
            QMessageBox *error = new QMessageBox(QMessageBox::Warning, "ERROR", "The epoch window should be put as such : 0:500", QMessageBox::Ok);
            error->exec();
            index->setText("0:500"); //get the value from protocol data file ?
        }
        break;
    }
    case 5:
    {
        QStringList splitData = index->text().split(":");
        if (splitData.count() != 2)
        {
            QMessageBox *error = new QMessageBox(QMessageBox::Warning, "ERROR", "The baseline window should be put as such : 0:500", QMessageBox::Ok);
            error->exec();
            index->setText("0:500"); //get the value from protocol data file ?
        }
        break;
    }
    }
}

void ProtocolWindow::SaveAllProtocols()
{
    for(int i = 0; i < files.size(); i++)
    {
        files[i]->Save();
    }
    m_dataChanged = false;
}
