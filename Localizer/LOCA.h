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
#include "../../Framework/Framework/Measure.h"
#include "Utility.h"
#include "eegContainer.h"
#include "patientFolder.h"
#include "PROV.h"
#include "Stats.h"
#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h"
#include "TriggerContainer.h"
#include "FrequencyBand.h"
#include "AlgorithmCalculator.h"

namespace InsermLibrary
{
	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOption);
		~LOCA();
		void eeg2erp(eegContainer *myeegContainer, PROV *myprovFile);
		void LocaSauron(eegContainer *myeegContainer, int idCurrentLoca, locaFolder *currentLoca);
		void LocaFrequency(eegContainer *myeegContainer, int idCurrentLoca);

	private:
		void toBeNamedCorrectlyFunction(eegContainer *myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a);
		//==
		void CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer *myeegContainer, TriggerContainer *triggerContainer, PROV *myprovFile);
		void CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger> & triggers);
		void CreateConfFile(eegContainer *myeegContainer);
		void RelinkAnalysisFileAnUglyWay(const std::string& rootPath, const std::string& fileNameBase, const std::string& frequencySuffix, const std::string& downsamplingFactor);

		//==
		std::string createIfFreqFolderExistNot(eegContainer *myeegContainer, FrequencyBand currentFreq);
		PROV* LoadProvForTask();
		std::vector<PROV> LoadAllProvForTask();
		bool shouldPerformBarPlot(std::string locaName);
		bool isBarPlot(std::string provFile);
		//==
		void barplot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
		std::string getMapsFolderBar(std::string freqFolder, PROV *myprovFile);
		std::string prepareFolderAndPathsBar(std::string freqFolder, int dsSampFreq);
        std::vector<PVALUECOORD> calculateStatisticKruskall(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
		//==
		void env2plot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
		std::string getMapsFolderPlot(std::string freqFolder, PROV *myprovFile);
		std::string prepareFolderAndPathsPlot(std::string freqFolder, int dsSampFreq);

		//==
		void timeTrialmatrices(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
		std::string getMapsFolderTrial(PROV *myprovFile, std::string freqFolder);
		std::string prepareFolderAndPathsTrial(std::string freqFolder, int dsSampFreq);
		bool shouldPerformStatTrial(std::string locaName);
        std::vector<PVALUECOORD> calculateStatisticWilcoxon(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);

	signals:
		void sendLogInfo(QString);
		void incrementAdavnce(int divider);

	private:
		locaFolder *m_currentLoca = nullptr; //Only contains a link to filesystem information about current localizer, do not delete in destructor
		int m_idCurrentLoca = -1;
		TriggerContainer *m_triggerContainer = nullptr;
		std::vector<FrequencyBandAnalysisOpt> m_analysisOpt;
		statOption* m_statOption;
		picOption* m_picOption;
	};
}

#endif
