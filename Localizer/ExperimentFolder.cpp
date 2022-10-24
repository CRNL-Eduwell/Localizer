#include "ExperimentFolder.h"
#include "Utility.h"
#include <QDir>
#include <QRegularExpression>

ExperimentFolder::ExperimentFolder(std::string path)
{
    m_Path = path;
    GetExperimentNameFromPath(m_Path);
    GetEegFiles(m_Path);
    GetFrequencyFolders(m_Path);
}

ExperimentFolder::~ExperimentFolder()
{

}

InsermLibrary::IEegFileInfo* ExperimentFolder::GetEegFileInfo(InsermLibrary::FileType fileType)
{
    switch(fileType)
    {
    case InsermLibrary::FileType::Micromed:
        {
            return &m_MicromedFileInfo;
        }
    case InsermLibrary::FileType::Elan:
        {
            return &m_ElanFileInfo;
        }
    case InsermLibrary::FileType::Brainvision:
        {
            return &m_BvFileInfo;
        }
    case InsermLibrary::FileType::EuropeanDataFormat:
        {
            return &m_EdfFileInfo;
        }
    default:
        {
            return nullptr;
        }
    }
}

void ExperimentFolder::GetExperimentNameFromPath(std::string path)
{
    std::vector<std::string> SplitFolderPath = InsermLibrary::split<std::string>(path, "\\/");
    m_FolderName = SplitFolderPath[SplitFolderPath.size() - 1];

    std::vector<std::string> SplitFolderInfo = InsermLibrary::split<std::string>(m_FolderName, "_");
    m_ExperimentLabel = SplitFolderInfo[SplitFolderInfo.size() - 1];
}

void ExperimentFolder::GetEegFiles(std::string path)
{
    QRegularExpression rxTRC((m_FolderName + ".trc").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpression rxEeg((m_FolderName + ".eeg").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    //QRegularExpression rxEnt((m_FolderName + ".eeg.ent").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpression rxPos((m_FolderName + "_raw.pos").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpression rxDsPos((m_FolderName + "_ds" + "(\\d+)" + ".pos").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpression rxEdf((m_FolderName + ".edf").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpression rxBrainVision((m_FolderName + ".vhdr").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);

    QDir currentDir(path.c_str());
    currentDir.setNameFilters(QStringList() << "*.trc" << "*.eeg" /*<< "*.eeg.ent"*/ << "*.pos" << "*.edf" << "*.vhdr");

    std::string trc = "", eeg = "", eegent = "", pos = "", dspos = "", bv = "", edf = "";
    QStringList fileFound = currentDir.entryList();
    for (int i = 0; i < fileFound.size(); i++)
    {
        QRegularExpressionMatch trcMatch = rxTRC.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (trcMatch.hasMatch())
            trc = path + fileFound[i].toStdString();

        QRegularExpressionMatch eegMatch = rxEeg.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (eegMatch.hasMatch())
            eeg = path + fileFound[i].toStdString();

//        QRegularExpressionMatch entMatch = rxEnt.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
//        if (entMatch.hasMatch())
//            eegent = path + fileFound[i].toStdString();

        QRegularExpressionMatch posMatch = rxPos.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (posMatch.hasMatch())
            pos = path + fileFound[i].toStdString();

        QRegularExpressionMatch dsPosMatch = rxDsPos.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (dsPosMatch.hasMatch())
            dspos = path + fileFound[i].toStdString();

        QRegularExpressionMatch edfMatch = rxEdf.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (edfMatch.hasMatch())
            edf = path + fileFound[i].toStdString();

        QRegularExpressionMatch bvMatch = rxBrainVision.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
        if (bvMatch.hasMatch())
            bv = path + fileFound[i].toStdString();
    }

    m_MicromedFileInfo = InsermLibrary::MicromedFileInfo(trc);

    m_ElanFileInfo = InsermLibrary::ElanFileInfo(eeg, pos, "");
    m_BvFileInfo = InsermLibrary::BrainVisionFileInfo(bv);
    m_EdfFileInfo = InsermLibrary::EdfFileInfo(edf);
    m_DsPosFile = dspos;
}

void ExperimentFolder::GetFrequencyFolders(std::string path)
{
    QRegularExpression rxFreqFold((m_ExperimentLabel + "_f(\\d+)f(\\d+)").c_str());

    //Get every folder corresponding to one LOCALIZER exam
    QDir currentDir(path.c_str());
    currentDir.setFilter(QDir::Dirs);

    QStringList entries = currentDir.entryList();
    for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
    {

        QString dirname = *entry;
        QRegularExpressionMatch dirMatch = rxFreqFold.match(dirname);
        if (dirname != QObject::tr(".") && dirname != QObject::tr("..") && dirMatch.hasMatch())
        {
            m_FrequencyFolders.push_back(FrequencyFolder(path + dirname.toStdString() + "/"));
        }
    }
}
