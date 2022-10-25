#ifndef MULTISUBJECTWORKER_H
#define MULTISUBJECTWORKER_H

#include <QCoreApplication>
#include <iostream>
#include <ctime>
#include "IWorker.h"
#include "patientFolder.h"
#include "SubjectFolder.h"
#include "FrequencyBandAnalysisOpt.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class MultiSubjectWorker : public IWorker
{
    Q_OBJECT

public:
    MultiSubjectWorker(std::vector<SubjectFolder> subjects, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& frequencyBands, InsermLibrary::statOption statOption, InsermLibrary::picOption picOption, std::vector<InsermLibrary::FileType> filePriority, std::string ptsFilePath);
    ~MultiSubjectWorker();
    void Process();
    void ExtractElectrodeList();

private:
    InsermLibrary::eegContainer* ExtractData(ExperimentFolder currentLoca, bool extractOriginalData);

private:
    int m_CurrentProcessId = 0;
    std::vector<SubjectFolder> m_Subjects;
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> m_FrequencyBands;
    std::vector<InsermLibrary::FileType> m_filePriority;
};

#endif // MULTISUBJECTWORKER_H
