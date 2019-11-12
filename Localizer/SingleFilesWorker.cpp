#include "SingleFilesWorker.h"

using namespace InsermLibrary;

SingleFilesWorker::SingleFilesWorker(std::vector<singleFile>& singleFiles, std::vector<FrequencyBandAnalysisOpt>& FrequencyBands)
{
	m_currentFiles = std::vector<singleFile>(singleFiles);
	m_frequencyBands = std::vector<FrequencyBandAnalysisOpt>(FrequencyBands);

	m_Loca = new LOCA(m_frequencyBands, nullptr, nullptr);
}

SingleFilesWorker::~SingleFilesWorker()
{
	//m_loca destroyed in base class
}

void SingleFilesWorker::Process()
{
    std::stringstream TimeDisp;
	std::time_t t = std::time(nullptr);
	eegContainer *myContainer = nullptr;
	
	for (int i = 0; i < m_currentFiles.size(); i++)
	{
		emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + m_currentFiles[i].rootFolder() + " ==="));
		myContainer = ExtractData(m_currentFiles[i], true, i, m_frequencyBands.size());

		if (myContainer != nullptr)
		{
			emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
			//==
            std::stringstream().swap(TimeDisp);
			TimeDisp << std::put_time(std::localtime(&t), "%c") << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			m_Loca->LocaFrequency(myContainer, i);
			//==
            std::stringstream().swap(TimeDisp);
			TimeDisp << std::put_time(std::localtime(&t), "%c") << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			sendLogInfo("End of Loca number " + QString::number(i) + "\n");
			deleteAndNullify1D(myContainer);
		}
	}

	emit finished();
}

eegContainer* SingleFilesWorker::ExtractData(singleFile currentFile, bool extractOriginalData, int idFile, int nbFreqBand)
{
    FileExt currentExtention = currentFile.fileExtention();
    if (currentExtention == NO_EXT)
        return nullptr;
    std::string currentFilePath = currentFile.filePath(currentExtention);

    eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData, nbFreqBand);
    myContainer->DeleteElectrodes(m_IndexToDelete);
    myContainer->GetElectrodes();
    myContainer->BipolarizeElectrodes();

    emit sendLogInfo(QString::fromStdString("Bipole created !"));
    return myContainer;
}
