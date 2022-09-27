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

//EN COURS
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
#include "StatisticalFilesProcessor.h"


namespace InsermLibrary
{
	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOptionn, std::string ptsFilePath = "");
		~LOCA();
        void Eeg2erp(eegContainer *myeegContainer, PROV *myprovFile);
        void Localize(eegContainer *myeegContainer, int idCurrentLoca, locaFolder *currentLoca);
        void LocalizeMapsOnly(eegContainer *myeegContainer, int idCurrentLoca);

	private:
        void GenerateMapsAndFigures(eegContainer *myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a);
		//==
        void CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer *myeegContainer, TriggerContainer *triggerContainer, ProvFile *myprovFile);
		void CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger> & triggers, std::string extraFilePath = "");
		void CreateConfFile(eegContainer *myeegContainer);
		void RelinkAnalysisFileAnUglyWay(const std::string& rootPath, const std::string& fileNameBase, const std::string& frequencySuffix, const std::string& downsamplingFactor);

		//==
        std::string CreateFrequencyFolder(eegContainer *myeegContainer, FrequencyBand currentFreq);
		PROV* LoadProvForTask();
		std::vector<PROV> LoadAllProvForTask();

        ProvFile* LoadProvForTask(std::string taskName, std::string analysisName = "");

        bool ShouldPerformBarPlot(std::string locaName);
        bool IsBarPlot(std::string provFile);
		//==
        void Barplot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
        std::string GetBarplotMapsFolder(std::string freqFolder, PROV *myprovFile);
        std::string PrepareFolderAndPathsBar(std::string freqFolder, int dsSampFreq);
        std::vector<PVALUECOORD> ProcessKruskallStatistic(vec3<float> &bigData, eegContainer *myeegContainer, ProvFile* myprovFile, std::string freqFolder);
		//==
        void Env2plot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder);
        std::string GetEnv2PlotMapsFolder(std::string freqFolder, PROV *myprovFile);
        std::string PrepareFolderAndPathsPlot(std::string freqFolder, int dsSampFreq);

		//==
        void TimeTrialMatrices(eegContainer *myeegContainer, ProvFile* myprovFile, std::string freqFolder);
        std::string GetTrialmatFolder(ProvFile* myprovFile, std::string freqFolder);
        std::string PrepareFolderAndPathsTrial(std::string freqFolder, int dsSampFreq);
        bool ShouldPerformTrialmatStats(std::string locaName);
        std::vector<PVALUECOORD> ProcessWilcoxonStatistic(vec3<float> &bigData, eegContainer *myeegContainer, ProvFile* myprovFile, std::string freqFolder);

		//==
		void CorrelationMaps(eegContainer* myeegContainer, std::string freqFolder);
        std::string DefineMapPath(std::string freqFolder, int dsSampFreq, int windowSizeInSec);
        std::vector<int> DefineCorrelationWindowsCenter(int halfWindowSizeInSample, int fileSizeInSample);
        std::vector<std::vector<float>> ComputeElectrodesDistances(eegContainer* myeegContainer);
        std::vector<std::vector<float>> ComputeElectrodesDistancesFromPts(eegContainer* myeegContainer);
        float ComputeSurrogate(int electrodeCount, int triggerCount, int surrogateCount, vec2<float> distances, vec3<float> eegData);
		void DrawCorrelationCircle(QPainter* painterChanel, eegContainer* myeegContainer, int halfwidth, int halfheight, int offset);
		void DrawCorrelationOnCircle(QPainter* painterChanel, int halfheight, int offset, std::vector<std::vector<float>> dist, std::vector<std::vector<float>> corre);
		int GetIndexFromElectrodeLabel(std::string myString);
		QColor GetColorFromLabel(std::string label, std::string& memoryLabel);

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
		std::string m_PtsFilePath = "";
		int m_colorId = -1;
	};
}

#endif
