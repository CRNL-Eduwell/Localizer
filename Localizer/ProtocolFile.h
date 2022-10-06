#ifndef PROTOCOLFILE_H
#define PROTOCOLFILE_H

#include <QRegularExpression>
#include <QString>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardItemModel>

#include <string>
#include <iostream>
//#include "PROV.h"
#include "../../EEGFormat/EEGFormat/Utility.h"

class ProtocolFile
{
public:
    ProtocolFile(const QString & filePath);
    ~ProtocolFile();
    inline QStandardItemModel* Model() const { return m_ItemModel; }
    inline QFileInfo FileInfo() const { return m_FileInfo; }
    void Save();

private:
    QStringList GetHeaderList();
    void LoadEmptyModel();
    //void LoadProtocolInModel(InsermLibrary::PROV protocol);

private:
    QFileInfo m_FileInfo;
    QStandardItemModel* m_ItemModel = nullptr;
    //InsermLibrary::PROV *m_Prov = nullptr;
};

#endif // PROTOCOLFILE_H
