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
    PatientFolderWorker(patientFolder currentPatient, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileExt> filePriority);
	~PatientFolderWorker();
	void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(locaFolder currentLoca, bool extractOriginalData);

private:
    patientFolder* m_Patient = nullptr;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_FrequencyBands;
	std::vector<InsermLibrary::FileExt> m_filePriority;
};

#endif
