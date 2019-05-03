#ifndef _LOCA_H
#define _LOCA_H

#include <QtWidgets/QMainWindow>	
#include <iostream>	
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
#include "optionsParameters.h"
#include "TriggerContainer.h"

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
		LOCA(userOption *userOpt);
		~LOCA();
		void eeg2erp(eegContainer *myeegContainer, PROV *myprovFile);
		void LocaSauron(eegContainer *myeegContainer, int idCurrentLoca, locaFolder *currentLoca);
		void LocaFrequency(eegContainer *myeegContainer, int idCurrentLoca);
		static void checkShannonCompliance(int p_samplingFrequency, frequency & p_freq);
	private:
		void toBeNamedCorrectlyFunction(eegContainer *myeegContainer, int idCurrentFreq, string freqFolder, analysisOption a);
		//==
		void CreateEventsFile(eegContainer *myeegContainer, TriggerContainer *triggerContainer, PROV *myprovFile);
		void createPosFile(std::string filePath, std::vector<Trigger> & triggers);
		void createConfFile(eegContainer *myeegContainer);
		//==
		string createIfFreqFolderExistNot(eegContainer *myeegContainer, frequency currentFreq);
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
		//TRIGGINFO *triggCatEla = nullptr, *triggCatEla2 = nullptr;
		locaFolder *currentLoca = nullptr;
		TriggerContainer *m_triggerContainer = nullptr;
		int idCurrentLoca = -1;
		userOption *userOpt = nullptr;
	};
}

#endif