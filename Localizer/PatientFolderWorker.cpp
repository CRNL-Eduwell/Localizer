#include "PatientFolderWorker.h"

PatientFolderWorker::PatientFolderWorker(SubjectFolder currentPatient, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& analysisOpt, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileType> filePriority, std::string ptsFilePath)
{
    m_Patient = new SubjectFolder(currentPatient);
    m_FrequencyBands = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(analysisOpt);
    m_filePriority = std::vector<InsermLibrary::FileType>(filePriority);
    m_Loca = new InsermLibrary::LOCA(m_FrequencyBands, new InsermLibrary::statOption(statOption), new InsermLibrary::picOption(picOption), ptsFilePath);
}

PatientFolderWorker::~PatientFolderWorker()
{
    deleteAndNullify1D(m_Patient);
	//m_loca destroyed in base class
}

void PatientFolderWorker::Process()
{
    InsermLibrary::eegContainer *myContainer = nullptr;
    int localizerCount = static_cast<int>(m_Patient->ExperimentFolders().size());
    for (int i = 0; i < localizerCount; i++)
	{
        emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + m_Patient->ExperimentFolders()[i].Path() + " ==="));
        bool extractData = m_FrequencyBands.size() > 0 ? m_FrequencyBands[0].analysisParameters.eeg2env2 : false; //for now it's the same analysus choice for each band , might change in the future
        myContainer = ExtractData(m_Patient->ExperimentFolders()[i], extractData);

		if (myContainer != nullptr)
		{
            emit sendLogInfo("Number of Bipole for analysis : " + QString::number(myContainer->BipoleCount()));

            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("Begin time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");
            m_Loca->Localize(myContainer, i, &m_Patient->ExperimentFolders()[i]);
            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("End time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");

            emit sendLogInfo("End of processing for experiment " + QString::number(i+1) + " out of " + QString::number(localizerCount) + "\n");
			deleteAndNullify1D(myContainer);
		}
	}

	//Generate comportemental perf report

	emit finished();
}

void PatientFolderWorker::ExtractElectrodeList()
{
    if (m_Patient->ExperimentFolders().size() == 0)
	{
        emit sendLogInfo("Error, there does not seem to be any valid localizer folder for this patient, aborting analysis.\n");
		emit finished();
        return;
	}

    int filePriorityCount = static_cast<int>(m_filePriority.size());
    for (int i = 0; i < filePriorityCount; i++)
	{
        InsermLibrary::IEegFileInfo* ifileInfo = m_Patient->ExperimentFolders()[0].GetEegFileInfo(m_filePriority[i]);
        if(ifileInfo != nullptr)
        {
            if(ifileInfo->CheckForErrors() == 0)
            {
                std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(ifileInfo->GetFilesString());
                std::string connectCleanerFilePath = m_Patient->Path() + "/" + m_Patient->FolderLabel() + ".ccf";
                emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
                return;
            }
        }
	}

    emit sendLogInfo("No Compatible file format detected, aborting analysis.\n");
	emit finished();
}

InsermLibrary::eegContainer* PatientFolderWorker::ExtractData(ExperimentFolder currentLoca, bool extractOriginalData)
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
