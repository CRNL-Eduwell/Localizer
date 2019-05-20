#ifndef _LOCA_H
#define _LOCA_H

#include <QtWidgets/QMainWindow>	
#include <iostream>	
#include <vector>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
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

using namespace std;
using namespace InsermLibrary;
using namespace InsermLibrary::DrawCard;
using namespace InsermLibrary::DrawbarsPlots;

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
		void toBeNamedCorrectlyFunction(eegContainer *myeegContainer, int idCurrentFreq, string freqFolder, FrequencyBandAnalysisOpt a);
		//==
		void CreateEventsFile(eegContainer *myeegContainer, TriggerContainer *triggerContainer, PROV *myprovFile);
		void CreatePosFile(std::string filePath, std::vector<Trigger> & triggers);
		void CreateConfFile(eegContainer *myeegContainer);
		//==
		string createIfFreqFolderExistNot(eegContainer *myeegContainer, FrequencyBand currentFreq);
		vector<PROV> loadProvCurrentLoca();
		bool shouldPerformBarPlot(string locaName);
		bool isBarPlot(string provFile);
		//==
		void barplot(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, string freqFolder);
		string getMapsFolderBar(string freqFolder, PROV *myprovFile);
		string prepareFolderAndPathsBar(string freqFolder, int dsSampFreq);
		vector<PVALUECOORD> calculateStatisticKruskall(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, string freqFolder);
		//==
		void env2plot(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, string freqFolder);
		string getMapsFolderPlot(string freqFolder, PROV *myprovFile);
		string prepareFolderAndPathsPlot(string freqFolder, int dsSampFreq);

		//==
		void timeTrialmatrices(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, string freqFolder);
		string getMapsFolderTrial(PROV *myprovFile, string freqFolder);
		string prepareFolderAndPathsTrial(string freqFolder, int dsSampFreq);
		bool shouldPerformStatTrial(string locaName);
		vector<PVALUECOORD> calculateStatisticWilcoxon(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, string freqFolder);
		float stdMean(vec2<float> eegDataChanel, int windowSam[2]);

	signals:
		void sendLogInfo(QString);
		void incrementAdavnce(int divider);

	private:
		locaFolder *m_currentLoca = nullptr;
		int m_idCurrentLoca = -1;
		TriggerContainer *m_triggerContainer = nullptr;
		//userOption *m_userOpt = nullptr;

		std::vector<FrequencyBandAnalysisOpt> m_analysisOpt;
		statOption* m_statOption;
		picOption* m_picOption;
	};
}

#endif