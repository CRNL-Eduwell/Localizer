#ifndef SINGLEFILESWORKER_H
#define SINGLEFILESWORKER_H

#include <QCoreApplication>

#include "Windows.h" //pour SYSTEMTIME , voir pour changer pour un truc cross platform
#include "IWorker.h"
#include "singleFile.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class SingleFilesWorker : public IWorker
{
	Q_OBJECT

public:
	SingleFilesWorker(std::vector<singleFile>& singleFiles, std::vector<FrequencyBandAnalysisOpt>& FrequencyBands);
	~SingleFilesWorker();
	void Process();

private:
	eegContainer* ExtractData(singleFile currentFile, bool extractOriginalData, int idFile, int nbFreqBand);

private:
	std::vector<singleFile> m_currentFiles;
	std::vector<FrequencyBandAnalysisOpt> m_frequencyBands;
};

#endif