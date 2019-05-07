#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QThread>

#include "eegContainer.h"
#include "Wrapper.h"
#include "LOCA.h"
#include "Utility.h"
#include "patientFolder.h"
#include "singleFile.h"
#include "optionsStats.h"
#include "optionsPerf.h"
#include "Windows.h"

using namespace std;
using namespace InsermLibrary;

class Worker : public QObject
{
	Q_OBJECT

public:
	Worker(patientFolder *currentPatient, userOption *userOpt);
	Worker(locaFolder *loca, userOption *userOpt);
	Worker(vector<singleFile> currentFiles, userOption *userOpt, int idFile = -1);
	Worker(std::string myFirstTRC, std::string mySecondTRC, std::string myOutputTRC);
	~Worker();
	LOCA* getLoca();
private:
	void analysePatientFolder(patientFolder *currentPatient);
	void analyseSingleFiles(vector<singleFile> currentFiles);
	eegContainer* ExtractExamData(locaFolder currentLoca, int idFile = -1, int nbFreqBand = 0);
	eegContainer* ExtractSingleFileData(singleFile currentFile, int idFile = -1, int nbFreqBand = 0);
	eegContainer* GetEegContainer(std::string currentFilePath, int idFile, int nbFreqBand);
	bool extractOriginalData(locaAnalysisOption anaOpt);

public slots:
	void processAnalysis();
	void processERP();
	void processToELAN();
	void processConcatenation();

signals:
	void sendLogInfo(QString);
	void finished();
	void sendContainerPointer(eegContainer *eegCont);

public:
	int bipCreated = -1;

private:
	LOCA *loca = nullptr;
	vector<int> elecToDeleteMem;
	patientFolder *patient = nullptr;
	locaFolder *locaFold = nullptr;
	vector<singleFile> files;
	vector<std::string> trcFiles;
	int idFile = -1;
	userOption *optionUser = nullptr;
};

#endif