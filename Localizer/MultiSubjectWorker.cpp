#include "MultiSubjectWorker.h"

MultiSubjectWorker::MultiSubjectWorker(std::vector<patientFolder> subjects, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& analysisOpt, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileExt> filePriority, std::string ptsFilePath)
{
    m_Subjects = std::vector<patientFolder>(subjects);
    m_FrequencyBands = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(analysisOpt);
    m_filePriority = std::vector<InsermLibrary::FileExt>(filePriority);
    m_Loca = new InsermLibrary::LOCA(m_FrequencyBands, new InsermLibrary::statOption(statOption), new InsermLibrary::picOption(picOption), ptsFilePath);
}

MultiSubjectWorker::~MultiSubjectWorker()
{
    //m_loca destroyed in base class
}

void MultiSubjectWorker::Process()
{
    patientFolder pat = m_Subjects[m_CurrentProcessId];
    emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + pat.rootFolder() + " ==="));

    InsermLibrary::eegContainer *myContainer = nullptr;
    int localizerCount = static_cast<int>(pat.localizerFolder().size());
    for (int i = 0; i < localizerCount; i++)
    {
        emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + pat.localizerFolder()[i].rootLocaFolder() + " ==="));
        bool extractData = m_FrequencyBands.size() > 0 ? m_FrequencyBands[0].analysisParameters.eeg2env2 : false; //for now it's the same analysus choice for each band , might change in the future
        myContainer = ExtractData(pat.localizerFolder()[i], extractData);

        if (myContainer != nullptr)
        {
            emit sendLogInfo("Number of Bipole for analysis : " + QString::number(myContainer->BipoleCount()));

            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("Begin time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");
            m_Loca->Localize(myContainer, i, &pat.localizerFolder()[i]);
            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("End time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");

            sendLogInfo("End of processing for experiment " + QString::number(i+1) + " out of " + QString::number(localizerCount) + "\n");
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

    patientFolder pat = m_Subjects[m_CurrentProcessId];
    if (pat.localizerFolder().size() == 0)
    {
        emit sendLogInfo("Error, there is no localizer folder in this patient, aborting analysis.\n");
        emit finished();
    }

    int filePriorityCount = static_cast<int>(m_filePriority.size());
    for (int i = 0; i < filePriorityCount; i++)
    {
        std::string currentFilePath = pat.localizerFolder()[0].filePath(m_filePriority[i]);
        if (EEGFormat::Utility::DoesFileExist(currentFilePath))
        {
            std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(currentFilePath);
            std::string connectCleanerFilePath = pat.rootFolder() + "/" + pat.patientName() + ".ccf";
            emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
            return;
        }
    }

    emit sendLogInfo("No Compatible file format detected, aborting analysis.\n");
    emit finished();
}

InsermLibrary::eegContainer* MultiSubjectWorker::ExtractData(locaFolder currentLoca, bool extractOriginalData)
{
    int filePriorityCount = static_cast<int>(m_filePriority.size());
    for (int i = 0; i < filePriorityCount; i++)
    {
        std::string currentFilePath = currentLoca.filePath(m_filePriority[i]);
        if (EEGFormat::Utility::DoesFileExist(currentFilePath))
        {
            InsermLibrary::eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData);
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

    //if we arrive at this point, no compatible file has been detected, aborting
    emit sendLogInfo(QString::fromStdString("No Compatible file format detected, aborting file extraction"));
    return nullptr;
}
