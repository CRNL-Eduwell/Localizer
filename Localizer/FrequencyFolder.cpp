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
    GetFrequencyEegFiles(path);
    GetDataFolders(path);
}

FrequencyFolder::~FrequencyFolder()
{

}

InsermLibrary::IEegFileInfo* FrequencyFolder::GetEegFileInfo(SmoothingWindow smoothingWindow, InsermLibrary::FileType fileType)
{
    switch(smoothingWindow)
    {
        case SmoothingWindow::SM0:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[0].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[0].second;
            }
            else
            {
                return nullptr;
            }
        }
        case SmoothingWindow::SM250:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[1].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[1].second;
            }
            else
            {
                return nullptr;
            }
        }
        case SmoothingWindow::SM500:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[2].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[2].second;
            }
            else
            {
                return nullptr;
            }
        }
        case SmoothingWindow::SM1000:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[3].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[3].second;
            }
            else
            {
                return nullptr;
            }
        }
        case SmoothingWindow::SM2500:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[4].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[4].second;
            }
            else
            {
                return nullptr;
            }
        }
        case SmoothingWindow::SM5000:
        {
            if(fileType == InsermLibrary::FileType::Elan)
            {
                return &m_ElanSmoothingXFiles[5].second;
            }
            else if(fileType == InsermLibrary::FileType::Brainvision)
            {
                return &m_BvSmoothingXFiles[5].second;
            }
            else
            {
                return nullptr;
            }
        }
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
    QString rxExtensions = "(.eeg|.vhdr)"; //|.eeg.ent
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm0" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm250" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm500" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm1000" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm2500" + rxExtensions));
    rxEegSmX.push_back(QRegularExpression(rxBaseName + "_sm5000" + rxExtensions));

    QDir currentDir(path.c_str());
    currentDir.setNameFilters(QStringList() << "*.eeg" /*<< "*.eeg.ent"*/ << "*.vhdr");
    QStringList fileFound = currentDir.entryList();

    m_ElanSmoothingXFiles.resize(6);
    m_BvSmoothingXFiles.resize(6);
    for (int i = 0; i < fileFound.size(); i++)
    {
        for (int j = 0; j < 6; j++)
        {
            QRegularExpressionMatch smxMatch = rxEegSmX[j].match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
            if (smxMatch.hasMatch())
            {
                QString filePath = QString::fromStdString(m_Path) + fileFound[i];
                QFileInfo f(filePath);
                std::cout << filePath.toStdString() << std::endl;
                if(f.suffix() == "vhdr")
                {
                    m_BvSmoothingXFiles[j] = std::make_pair(j, InsermLibrary::BrainVisionFileInfo(filePath.toStdString()));
                }
                else
                {
                    //This way we either take Brainvision data or Elan (since bv data file is also a .eeg file)
                    if(f.suffix() == "eeg")
                    {
                        m_ElanSmoothingXFiles[j] = std::make_pair(j, InsermLibrary::ElanFileInfo(filePath.toStdString()));
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
