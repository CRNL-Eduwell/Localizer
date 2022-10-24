#ifndef PATIENTFOLDERWORKER_H
#define PATIENTFOLDERWORKER_H

#include <QCoreApplication>
#include <iostream>
#include <ctime>
#include "IWorker.h"
//#include "patientFolder.h"
#include "SubjectFolder.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class PatientFolderWorker : public IWorker
{
	Q_OBJECT

public:
    PatientFolderWorker(SubjectFolder currentPatient, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileType> filePriority, std::string ptsFilePath);
	~PatientFolderWorker();
	void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(ExperimentFolder currentLoca, bool extractOriginalData);

private:
    SubjectFolder* m_Patient = nullptr;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_FrequencyBands;
    std::vector<InsermLibrary::FileType> m_filePriority;
};

#endif
