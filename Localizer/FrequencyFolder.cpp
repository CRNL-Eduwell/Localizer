#include "FrequencyFolder.h"
#include "Utility.h"
#include "BrainvisionFileInfo.h"
#include "ElanFileInfo.h"

#include <QDir>
#include <QRegularExpression>

FrequencyFolder::FrequencyFolder(std::string path)
{
    m_Path = path;
    GetFrequencyBandFromPath(path);
}

FrequencyFolder::~FrequencyFolder()
{
    for(int i = 0; i < m_SmoothingXFiles.size(); i++)
    {
        InsermLibrary::deleteAndNullify1D(m_SmoothingXFiles[i]);
    }
}

void FrequencyFolder::GetFrequencyBandFromPath(std::string path)
{
    std::vector<std::string> SplitFolderPath = InsermLibrary::split<std::string>(path, "\\/");
    m_FolderName = SplitFolderPath[SplitFolderPath.size() - 1];

    std::vector<std::string> SplitFolderInfo = InsermLibrary::split<std::string>(m_FolderName, "_");
    m_FrequencyBandLabel = SplitFolderInfo[SplitFolderInfo.size() - 1];
}

void FrequencyFolder::GetFrequencyEegFiles(std::string path)
{
    std::vector<QRegularExpression> rxEegSmX;
    QString rxBaseName = QString::fromStdString(m_FolderName + "_ds" + "(\\d+)");
    QString rxExtensions = "(.eeg|.eeg.ent|.vhdr)";
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm0" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm250" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm500" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm1000" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm2500" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm5000" + rxExtensions));

    QDir currentDir(path.c_str());
    currentDir.setNameFilters(QStringList() << "*.eeg" /*<< "*.eeg.ent"*/ << "*.vhdr");
    QStringList fileFound = currentDir.entryList();

    m_SmoothingXFiles.resize(6);
    for (int i = 0; i < fileFound.size(); i++)
    {
        for (int j = 0; j < 6; j++)
        {
            QRegularExpressionMatch smxMatch = rxEegSmX[j].match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
            if (smxMatch.hasMatch())
            {
                QString filePath = QString::fromStdString(m_FolderName) + fileFound[i];
                QFileInfo f(filePath);
                if(f.suffix() == "vhdr")
                {
                    m_SmoothingXFiles[j] = new InsermLibrary::BrainVisionFileInfo(filePath.toStdString());
                }
                else
                {
                    //This way we either take Brainvision data or Elan (since bv data file is also a .eeg file)
                    if(f.suffix() == "eeg")
                    {
                        m_SmoothingXFiles[j] = new InsermLibrary::ElanFileInfo(filePath.toStdString());
                    }
                }
            }
        }
    }
}

void FrequencyFolder::GetDataFolders(std::string path)
{
    QDir currentDir(path.c_str());
    currentDir.setFilter(QDir::Dirs);

    QStringList entries = currentDir.entryList();
    for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
    {
        QString dirname = *entry;
        if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
        {
            m_DataFolder.push_back(AnalysisFolder(path + dirname.toStdString() + "/"));
        }
    }
}
