#ifndef CLEANCONNECTFILE_H
#define CLEANCONNECTFILE_H

#include <string>
#include <vector>
#include <QFileInfo>
#include <QStandardItemModel>

class CleanConnectFile
{
public:
    CleanConnectFile(QFileInfo fileInfo);
    CleanConnectFile(const std::vector<std::string> & labels);
    ~CleanConnectFile();
    QStandardItemModel* Model() const { return m_ItemModel; }
    QFileInfo FileInfo() const { return m_FileInfo; }
    void Save();

private:
    QFileInfo m_FileInfo;
    QStandardItemModel* m_ItemModel;
};

#endif // CLEANCONNECTFILE_H
