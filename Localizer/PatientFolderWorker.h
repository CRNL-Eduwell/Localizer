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
    PatientFolderWorker(patientFolder currentPatient, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption);
	~PatientFolderWorker();
	void Process();

private:
    InsermLibrary::eegContainer* ExtractData(locaFolder currentLoca, bool extractOriginalData, int idFile, int nbFreqBand);

private:
	patientFolder* m_patient = nullptr;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_frequencyBands;
};

#endif
