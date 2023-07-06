#include "MultiSubjectWorker.h"

MultiSubjectWorker::MultiSubjectWorker(std::vector<SubjectFolder*> subjects, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& analysisOpt, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileType> filePriority, std::string ptsFilePath)
{
    m_Subjects = subjects;
    m_FrequencyBands = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(analysisOpt);
    m_filePriority = std::vector<InsermLibrary::FileType>(filePriority);
    m_Loca = new InsermLibrary::LOCA(m_FrequencyBands, new InsermLibrary::statOption(statOption), new InsermLibrary::picOption(picOption), ptsFilePath);
}

MultiSubjectWorker::~MultiSubjectWorker()
{
    //m_loca destroyed in base class
}

void MultiSubjectWorker::Process()
{
    SubjectFolder pat = *m_Subjects[m_CurrentProcessId];
    emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + pat.Path() + " ==="));

    InsermLibrary::eegContainer *myContainer = nullptr;
    int localizerCount = static_cast<int>(pat.ExperimentFolders().size());
    for (int i = 0; i < localizerCount; i++)
    {
        emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + pat.ExperimentFolders()[i].Path() + " ==="));
        bool extractData = m_FrequencyBands.size() > 0 ? m_FrequencyBands[0].analysisParameters.eeg2env2 : false; //for now it's the same analysus choice for each band , might change in the future
        myContainer = ExtractData(pat.ExperimentFolders()[i], extractData);

        if (myContainer != nullptr)
        {
            emit sendLogInfo("Number of Bipole for analysis : " + QString::number(myContainer->BipoleCount()));

            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("Begin time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");
            m_Loca->Localize(myContainer, i, pat.GetEegFileInfo(i), pat.ExperimentFolders()[i].ExperimentLabel());
            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("End time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");

            emit sendLogInfo("End of processing for experiment " + QString::number(i+1) + " out of " + QString::number(localizerCount) + "\n");
            deleteAndNullify1D(myContainer);
        }
    }

    m_CurrentProcessId++;
    if(m_CurrentProcessId < static_cast<int>(m_Subjects.size()))
    {
        ExtractElectrodeList();
    }
    else
    {
        emit finished();
    }
}

void MultiSubjectWorker::ExtractElectrodeList()
{
    if(m_CurrentProcessId >= static_cast<int>(m_Subjects.size()))
    {
        throw new std::runtime_error("Error ExtractElectrodeList : ProcessID is greater than the number of subjects to process");
    }

    SubjectFolder pat = *m_Subjects[m_CurrentProcessId];
    if (pat.ExperimentFolders().size() == 0)
    {
        emit sendLogInfo("Error, there is no localizer folder in this patient, aborting analysis.\n");
        emit finished();
        return;
    }

    for(int k = 0; k < pat.ExperimentFolders().size(); k++)
    {
        int filePriorityCount = static_cast<int>(m_filePriority.size());
        for (int i = 0; i < filePriorityCount; i++)
        {
            InsermLibrary::IEegFileInfo* ifileInfo = pat.ExperimentFolders()[k].GetEegFileInfo(m_filePriority[i]);
            if(ifileInfo != nullptr)
            {
                if(ifileInfo->CheckForErrors() == 0)
                {
                    std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(ifileInfo->GetFilesString());
                    std::string connectCleanerFilePath = pat.Path() + "/" + pat.SubjectLabel() + ".ccf";
                    emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
                    return;
                }
            }
        }
    }

    emit sendLogInfo("No Compatible file format detected, aborting analysis.\n");
    //emit finished();
}

InsermLibrary::eegContainer* MultiSubjectWorker::ExtractData(ExperimentFolder currentLoca, bool extractOriginalData)
{
    int filePriorityCount = static_cast<int>(m_filePriority.size());
    for (int i = 0; i < filePriorityCount; i++)
    {
        InsermLibrary::IEegFileInfo* ifileInfo = currentLoca.GetEegFileInfo(m_filePriority[i]);
        if(ifileInfo != nullptr)
        {
            if(ifileInfo->CheckForErrors() == 0)
            {
                InsermLibrary::eegContainer *myContainer = GetEegContainer(ifileInfo->GetFilesString(), extractOriginalData);
                myContainer->DeleteElectrodes(m_IndexToDelete);
                myContainer->GetElectrodes();

                switch(m_ElectrodeOperation)
                {
                    case 0: //mono
                    {
                        myContainer->MonoElectrodes();
                        emit sendLogInfo(QString::fromStdString("Single channel created !"));
                        break;
                    }
                    case 1: //bipo
                    {
                        myContainer->BipolarizeElectrodes();
                        emit sendLogInfo(QString::fromStdString("Bipole created !"));
                        break;
                    }
                    default:
                    {
                        emit sendLogInfo(QString::fromStdString("Error, operation unknow, aborting"));
                        return nullptr;
                    }
                }
                return myContainer;
            }
        }
    }

    //if we arrive at this point, no compatible file has been detected, aborting
    emit sendLogInfo(QString::fromStdString("No Compatible file format detected, aborting file extraction"));
    return nullptr;
}
