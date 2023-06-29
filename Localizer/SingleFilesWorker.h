#ifndef SINGLEFILESWORKER_H
#define SINGLEFILESWORKER_H

#include <QCoreApplication>

#include <iostream>
#include <ctime>
#include "IWorker.h"
#include "singleFile.h"
#include "FrequencyBandAnalysisOpt.h"

class SingleFilesWorker : public IWorker
{
	Q_OBJECT

public:
    SingleFilesWorker(std::vector<singleFile>& singleFiles, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands);
	~SingleFilesWorker();
	void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(singleFile currentFile, bool extractOriginalData);

private:
    int m_CurrentProcessId = 0;
	std::vector<singleFile> m_currentFiles;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_frequencyBands;
};

#endif
