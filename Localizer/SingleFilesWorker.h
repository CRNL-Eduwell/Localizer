#ifndef SINGLEFILESWORKER_H
#define SINGLEFILESWORKER_H

#include <QCoreApplication>

#include <iostream>
#include <ctime>
#include "IWorker.h"
#include "singleFile.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class SingleFilesWorker : public IWorker
{
	Q_OBJECT

public:
    SingleFilesWorker(std::vector<singleFile>& singleFiles, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands);
	~SingleFilesWorker();
	void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(singleFile currentFile, bool extractOriginalData, int nbFreqBand);

private:
    int m_CurrentProcessId = 0;
	std::vector<singleFile> m_currentFiles;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_frequencyBands;
};

#endif
