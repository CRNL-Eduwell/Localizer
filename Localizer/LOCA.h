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
	private:
		void toBeNamedCorrectlyFunction(eegContainer *myeegContainer, int idCurrentFreq, string freqFolder, analysisOption a);
		//==
		void createPosFile(eegContainer *myeegContainer, PROV *myprovFile);
		void createConfFile(eegContainer *myeegContainer);
		void renameTriggers(TRIGGINFO *eegTriggers, TRIGGINFO *downsampledEegTriggers, PROV *myprovFile);
		//==
		void processEvents(eegContainer *myeegContainer, PROV *myprovFile);
		void processEventsDown(eegContainer *myeegContainer, PROV *myprovFile);
		void pairStimResp(TRIGGINFO *downsampledEegTriggers, PROV *myprovFile);
		void deleteUnsignificativEvents(TRIGGINFO *downsampledEegTriggers, PROV *myprovFile);
		void sortTrials(TRIGGINFO *eegTriggersTemp, PROV *myprovFile, int downSampFreq);
		void swapStimResp(TRIGGINFO *eegTriggers, PROV *myprovFile);
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

	signals:
		void sendLogInfo(QString);
	private:
		TRIGGINFO *triggCatEla = nullptr, *triggCatEla2 = nullptr;
		locaFolder *currentLoca = nullptr;
		int idCurrentLoca = -1;
		userOption *userOpt = nullptr;
	};
}

#endif