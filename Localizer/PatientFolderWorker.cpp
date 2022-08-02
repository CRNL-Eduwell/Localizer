#include "PatientFolderWorker.h"

PatientFolderWorker::PatientFolderWorker(patientFolder currentPatient, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& analysisOpt, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileExt> filePriority, std::string ptsFilePath)
{
    m_Patient = new patientFolder(currentPatient);
    m_FrequencyBands = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(analysisOpt);
	m_filePriority = std::vector<InsermLibrary::FileExt>(filePriority);
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
    int localizerCount = static_cast<int>(m_Patient->localizerFolder().size());
    for (int i = 0; i < localizerCount; i++)
	{
        emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + m_Patient->localizerFolder()[i].rootLocaFolder() + " ==="));
        bool extractData = m_FrequencyBands.size() > 0 ? m_FrequencyBands[0].analysisParameters.eeg2env2 : false; //for now it's the same analysus choice for each band , might change in the future
        myContainer = ExtractData(m_Patient->localizerFolder()[i], extractData);

		if (myContainer != nullptr)
		{
            emit sendLogInfo("Number of Bipole for analysis : " + QString::number(myContainer->BipoleCount()));

            emit sendLogInfo("");
            emit sendLogInfo(QString::fromStdString("Begin time : ") + GetCurrentTime().c_str());
            emit sendLogInfo("");
            m_Loca->Localize(myContainer, i, &m_Patient->localizerFolder()[i]);
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
	if (m_Patient->localizerFolder().size() == 0)
	{
        emit sendLogInfo("Error, there is no localizer folder in this patient, aborting analysis.\n");
		emit finished();
	}

    int filePriorityCount = static_cast<int>(m_filePriority.size());
    for (int i = 0; i < filePriorityCount; i++)
	{
		std::string currentFilePath = m_Patient->localizerFolder()[0].filePath(m_filePriority[i]);
		if (EEGFormat::Utility::DoesFileExist(currentFilePath))
		{
			std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(currentFilePath);
			std::string connectCleanerFilePath = m_Patient->rootFolder() + "/" + m_Patient->patientName() + ".ccf";
			emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
			return;
		}
	}

    emit sendLogInfo("No Compatible file format detected, aborting analysis.\n");
	emit finished();
}

InsermLibrary::eegContainer* PatientFolderWorker::ExtractData(locaFolder currentLoca, bool extractOriginalData)
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
