#ifndef PATIENTFOLDERWORKER_H
#define PATIENTFOLDERWORKER_H

#include <QCoreApplication>
#include <iostream>
#include <ctime>
#include "IWorker.h"
#include "patientFolder.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class PatientFolderWorker : public IWorker
{
	Q_OBJECT

public:
	PatientFolderWorker(patientFolder currentPatient, std::vector<FrequencyBandAnalysisOpt>& FrequencyBands, statOption statOption, picOption picOption);
	~PatientFolderWorker();
	void Process();

private:
	eegContainer* ExtractData(locaFolder currentLoca, bool extractOriginalData, int idFile, int nbFreqBand);

private:
	patientFolder* m_patient = nullptr;
	std::vector<FrequencyBandAnalysisOpt> m_frequencyBands;
};

#endif