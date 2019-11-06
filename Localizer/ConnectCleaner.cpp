#include "ConnectCleaner.h"

ConnectCleaner::ConnectCleaner(InsermLibrary::eegContainer* eegCont, QWidget *parent) : QDialog(parent)
{
    containerEeg = eegCont;
	m_ElectrodesLabel = std::vector<std::string>(eegCont->flatElectrodes);
	ui.setupUi(this);
    connect(ui.ValidateButton, &QPushButton::clicked, this, &ConnectCleaner::CreateBipoles);
    FillList(m_ElectrodesLabel);
}

ConnectCleaner::~ConnectCleaner()
{

}

void ConnectCleaner::FillList(const std::vector<std::string> & labels)
{
    //Create Electrode list from file and/or labels
    CleanConnectFile file(labels);

    //Connect model to detect click
    connect(file.Model(), &QStandardItemModel::itemChanged, this, &ConnectCleaner::CheckMultipleItems);

    //Put Model in view
    ui.SelectElectrodeList->setModel(file.Model());
    ui.SelectElectrodeList->resizeColumnsToContents();
    ui.SelectElectrodeList->resizeRowsToContents();
    ui.SelectElectrodeList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui.SelectElectrodeList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    ui.SelectElectrodeList->horizontalHeader()->setHidden(true);
    ui.SelectElectrodeList->verticalHeader()->setHidden(true);
}

void ConnectCleaner::keyPressEvent(QKeyEvent *key)
{
    if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) )
    {
        QModelIndex index = ui.SelectElectrodeList->selectionModel()->currentIndex();
        QStandardItem *item = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model())->itemFromIndex(index);
        if(item != nullptr)
        {
            if (item->checkState() == Qt::CheckState::Checked)
            {
                item->setCheckState(Qt::CheckState::Unchecked);
            }
            else
            {
                item->setCheckState(Qt::CheckState::Checked);
            }
        }
    }
}

void ConnectCleaner::CheckMultipleItems(QStandardItem *item)
{
    if(m_lockMultiple == true)
        return;

    m_lockMultiple = true;

    QModelIndexList selection = ui.SelectElectrodeList->selectionModel()->selectedIndexes();
    QStandardItemModel *qsim = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model());
    for (int i = 0; i < selection.size(); i++)
    {
        qsim->item(selection[i].row(), 0)->setCheckState(item->checkState());
    }

    m_lockMultiple = false;
}

void ConnectCleaner::CreateBipoles()
{
    QStandardItemModel *qsim = dynamic_cast<QStandardItemModel*>(ui.SelectElectrodeList->model());
    for (int i = 0; i < qsim->rowCount(); i++)
    {
        if (qsim->item(i, 0)->checkState() == Qt::CheckState::Unchecked)
        {
            containerEeg->idElecToDelete.push_back(i);
        }
    }
    containerEeg = nullptr;
    done(1);
}
