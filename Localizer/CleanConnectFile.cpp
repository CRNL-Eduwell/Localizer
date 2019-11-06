#include "CleanConnectFile.h"

CleanConnectFile::CleanConnectFile(QFileInfo fileInfo) : m_FileInfo(fileInfo)
{
    m_ItemModel = new QStandardItemModel();
}

CleanConnectFile::~CleanConnectFile()
{

}

CleanConnectFile::CleanConnectFile(const std::vector<std::string> & labels)
{
    m_ItemModel = new QStandardItemModel(static_cast<int>(labels.size()), 2);
    for(uint i = 0; i < labels.size(); i++)
    {
        QStandardItem *check_item = new QStandardItem(QString::fromStdString(labels[i]));
        check_item->setCheckable(true);
        check_item->setCheckState(Qt::CheckState::Checked);
        check_item->setFlags(check_item->flags() | Qt::ItemIsUserCheckable); //Allow to intercept enter key for modification
        check_item->setFlags(check_item->flags() ^ Qt::ItemIsEditable); // Item not editable since we have correctedLabel_item
        QStandardItem *correctedLabel_item = new QStandardItem(QString::fromStdString(labels[i]));

        m_ItemModel->setItem(static_cast<int>(i), 0, check_item);
        m_ItemModel->setItem(static_cast<int>(i), 1, correctedLabel_item);
    }
}
