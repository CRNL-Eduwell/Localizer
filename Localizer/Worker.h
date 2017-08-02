#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QThread>

#include "patientFolder.h"
#include "singleFile.h"
#include "Utility.h"
#include "optionsStats.h"
#include "optionsPerf.h"

#include "eegContainer.h"

#include"LOCA.h"
#include "Windows.h"


using namespace std;
using namespace MicromedLibrary;
using namespace InsermLibrary;

class Worker : public QObject
{
	Q_OBJECT

public:
	Worker(patientFolder *currentPatient, userOption *userOpt);
	Worker(locaFolder *loca, userOption *userOpt);
	Worker(vector<singleFile> currentFiles, userOption *userOpt, int idFile);
	~Worker();
	LOCA* getLoca();
private:
	void analysePatientFolder(patientFolder *currentPatient);
	void analyseSingleFiles(vector<singleFile> currentFiles);
	eegContainer *extractEEGData(locaFolder currentLoca, int idFile, int nbFreqBand);
	eegContainer *extractEEGData(locaFolder currentLoca);
	eegContainer *extractEEGData(singleFile currentFile, int idFile, int nbFreqBand);
	eegContainer *extractEEGData(singleFile currentFile);
	void extractOriginalData(TRCFile *myTRC, locaAnalysisOption anaOpt);
	void extractOriginalData(ELANFile *myElan, locaAnalysisOption anaOpt);

public slots:
	void processAnalysis();
	void processERP();
	void processToELAN();

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
	int idFile = -1;
	userOption *optionUser = nullptr;
};

#endif