#include "PatientFolderWorker.h"

//at this point, patient folder has had the non wanted folder removed 

using namespace InsermLibrary;

PatientFolderWorker::PatientFolderWorker(patientFolder currentPatient, std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption statOption, picOption picOption, std::vector<InsermLibrary::FileExt> filePriority)
{
    m_Patient = new patientFolder(currentPatient);
    m_FrequencyBands = std::vector<FrequencyBandAnalysisOpt>(analysisOpt);
	m_filePriority = std::vector<InsermLibrary::FileExt>(filePriority);
    m_Loca = new LOCA(m_FrequencyBands, new InsermLibrary::statOption(statOption), new InsermLibrary::picOption(picOption));
}

PatientFolderWorker::~PatientFolderWorker()
{
    deleteAndNullify1D(m_Patient);
	//m_loca destroyed in base class
}

void PatientFolderWorker::Process()
{
    std::stringstream TimeDisp;
	eegContainer *myContainer = nullptr;
	std::time_t t = std::time(nullptr);

    for (int i = 0; i < m_Patient->localizerFolder().size(); i++)
	{
        emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + m_Patient->localizerFolder()[i].rootLocaFolder() + " ==="));
        bool extractData = m_FrequencyBands.size() > 0 ? m_FrequencyBands[0].analysisParameters.eeg2env2 : false; //for now it's the same analysus choice for each band , might change in the future
        myContainer = ExtractData(m_Patient->localizerFolder()[i], extractData, m_FrequencyBands.size());

		if (myContainer != nullptr)
		{
			emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
			//==
            std::stringstream().swap(TimeDisp);
			TimeDisp << std::put_time(std::localtime(&t), "%c") << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
            m_Loca->LocaSauron(myContainer, i, &m_Patient->localizerFolder()[i]);
			//==
            std::stringstream().swap(TimeDisp);
			TimeDisp << std::put_time(std::localtime(&t), "%c") << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			sendLogInfo("End of Loca number " + QString::number(i) + "\n");
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
		sendLogInfo("Error, there is no localizer folder in this patient, aborting analysis.\n");
		emit finished();
	}

	for (int i = 0; i < m_filePriority.size(); i++)
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

	//if we arrive at this point, no compatible file has been detected, aborting loca 
	sendLogInfo("No Compatible file format detected, aborting analysis.\n");
	emit finished();
}

eegContainer* PatientFolderWorker::ExtractData(locaFolder currentLoca, bool extractOriginalData, int nbFreqBand)
{
	for (int i = 0; i < m_filePriority.size(); i++)
	{
		std::string currentFilePath = currentLoca.filePath(m_filePriority[i]);
		if (EEGFormat::Utility::DoesFileExist(currentFilePath))
		{
			eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData, nbFreqBand);
			myContainer->DeleteElectrodes(m_IndexToDelete);
			myContainer->GetElectrodes();
			myContainer->BipolarizeElectrodes();

			emit sendLogInfo(QString::fromStdString("Bipole created !"));
			return myContainer;
		}
	}

	//if we arrive at this point, no compatible file has been detected, aborting 
	emit sendLogInfo(QString::fromStdString("No Compatible file format detected, aborting file extraction"));
	return nullptr;
}
