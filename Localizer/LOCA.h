#ifndef _LOCA_H
#define _LOCA_H

#include <QtWidgets/QMainWindow>	
#include <QCoreApplication>
#include <iostream>	
#include <vector>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <filesystem>
#include "../../Framework/Framework/Measure.h"
#include "Utility.h"
#include "eegContainer.h"
//#include "patientFolder.h"
#include "ExperimentFolder.h"
#include "ProvFile.h"
#include "Stats.h"
#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h"
#include "TriggerContainer.h"
#include "FrequencyBand.h"
#include "AlgorithmCalculator.h"

#include "../../Framework/Framework/Pearson.h"

#include <filesystem>
#include "EnvplotProcessor.h"
#include "BarplotProcessor.h"
#include "TrialMatricesProcessor.h"
#include "CorrelationMapsProcessor.h"
#include "StatisticalFilesProcessor.h"

#include "./../../EEGFormat/EEGFormat/Wrapper.h"

namespace InsermLibrary
{
	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOptionn, std::string ptsFilePath = "");
		~LOCA();
        void Eeg2erp(eegContainer *myeegContainer, ProvFile* myprovFile);
        void Localize(eegContainer* myeegContainer, int idCurrentLoca,  InsermLibrary::IEegFileInfo* currentLoca, std::string taskName);
        void LocalizeMapsOnly(eegContainer *myeegContainer, int idCurrentLoca);

	private:
        int LoadData(eegContainer* myeegContainer, std::pair<std::string,std::string> kvp, std::string frequency, SmoothingWindow smoothingWindow, int index);
        InsermLibrary::IEegFileInfo* GetEegFileInfo(eegContainer* myeegContainer, std::pair<std::string,std::string> kvp, SmoothingWindow smoothingWindow, int index, InsermLibrary::FileType fileType);
        int LoadProcessedData(eegContainer* myeegContainer, InsermLibrary::IEegFileInfo* requestedFile, int index);
        void GenerateMapsAndFigures(eegContainer *myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a);
		//==
        void CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer *myeegContainer, TriggerContainer *triggerContainer, ProvFile *myprovFile);
		void CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger> & triggers, std::string extraFilePath = "");
		void CreateConfFile(eegContainer *myeegContainer);
        void RelinkAnalysisFileAnUglyWay(eegContainer *myeegContainer, const std::string& frequencySuffix, const std::string& downsamplingFactor);
		//==
        std::string CreateFrequencyFolder(eegContainer *myeegContainer, FrequencyBand currentFreq);
        ProvFile* LoadProvForTask(std::string taskName, std::string analysisName = "");

	signals:
		void sendLogInfo(QString);
		void incrementAdavnce(int divider);

	private:
        std::string m_currentLocaName = "";
		int m_idCurrentLoca = -1;
		TriggerContainer *m_triggerContainer = nullptr;
		std::vector<FrequencyBandAnalysisOpt> m_analysisOpt;
		statOption* m_statOption;
		picOption* m_picOption;
		std::string m_PtsFilePath = "";
	};
}

#endif
