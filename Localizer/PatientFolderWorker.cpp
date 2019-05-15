#include "PatientFolderWorker.h"

//at this point, patient folder has had the non wanted folder removed 

PatientFolderWorker::PatientFolderWorker(patientFolder currentPatient, std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption statOption, picOption picOption)
{
	m_patient = new patientFolder(currentPatient);
	m_frequencyBands = std::vector<FrequencyBandAnalysisOpt>(analysisOpt);

	m_loca = new LOCA(m_frequencyBands, new InsermLibrary::statOption(statOption), new InsermLibrary::picOption(picOption));
}

PatientFolderWorker::~PatientFolderWorker()
{
	deleteAndNullify1D(m_patient);
	//m_loca destroyed in base class
}

void PatientFolderWorker::Process()
{
	stringstream TimeDisp;
	SYSTEMTIME LocalTime;
	eegContainer *myContainer = nullptr;

	for (int i = 0; i < m_patient->localizerFolder().size(); i++)
	{
		emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + m_patient->localizerFolder()[i].rootLocaFolder() + " ==="));
		myContainer = ExtractData(m_patient->localizerFolder()[i], m_frequencyBands[i].eeg2env2, i, m_frequencyBands.size());

		if (myContainer != nullptr)
		{
			emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
			//==
			stringstream().swap(TimeDisp);
			GetLocalTime(&LocalTime);
			TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			m_loca->LocaSauron(myContainer, i, &m_patient->localizerFolder()[i]);
			//==
			stringstream().swap(TimeDisp);
			GetLocalTime(&LocalTime);
			TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			sendLogInfo("End of Loca number " + QString::number(i) + "\n");
			deleteAndNullify1D(myContainer);
		}
	}

	//Generate comportemental perf report

	emit finished();
}

eegContainer* PatientFolderWorker::ExtractData(locaFolder currentLoca, bool extractOriginalData, int idFile, int nbFreqBand)
{
	FileExt currentExtention = currentLoca.fileExtention();
	if (currentExtention == NO_EXT)
		return nullptr;
	std::string currentFilePath = currentLoca.filePath(currentExtention);
	eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData, nbFreqBand);

	bool promptToDelete = (idFile == -1) || (idFile == 0);
	if (promptToDelete)
	{
		emit sendContainerPointer(myContainer);
		while (bipCreated == -1) //While bipole not created 
		{
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
		}
		if (bipCreated == 0)
			return nullptr;

		m_electrodeToDeleteMemory = vector<int>(myContainer->idElecToDelete);
	}

	myContainer->DeleteElectrodes(m_electrodeToDeleteMemory);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}