#include "PatientFolderWorker.h"

//at this point, patient folder has had the non wanted folder removed 

using namespace InsermLibrary;

PatientFolderWorker::PatientFolderWorker(patientFolder currentPatient, std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption statOption, picOption picOption)
{
    m_Patient = new patientFolder(currentPatient);
    m_FrequencyBands = std::vector<FrequencyBandAnalysisOpt>(analysisOpt);

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
        myContainer = ExtractData(m_Patient->localizerFolder()[i], extractData, i, m_FrequencyBands.size());

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

eegContainer* PatientFolderWorker::ExtractData(locaFolder currentLoca, bool extractOriginalData, int idFile, int nbFreqBand)
{
    FileExt currentExtention = currentLoca.fileExtention();
    if (currentExtention == NO_EXT)
        return nullptr;
    std::string currentFilePath = currentLoca.filePath(currentExtention);

    eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData, nbFreqBand);
    myContainer->DeleteElectrodes(m_IndexToDelete);
    myContainer->GetElectrodes();
    myContainer->BipolarizeElectrodes();

    emit sendLogInfo(QString::fromStdString("Bipole created !"));
    return myContainer;
}
