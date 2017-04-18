#ifndef PATIENTFOLDER_H
#define PATIENTFOLDER_H

#include <iostream>
#include <vector>
#include "Utility.h"

#include <QDir>
#include <QString>
#include <QObject>

using namespace std;
using namespace InsermLibrary;

class patientFolder;
class locaFolder;
class frequencyFolder;

struct picData
{
	string pathToPic;
	int sortingWeight;
};

class analyzedDataFolder
{
public :
	analyzedDataFolder(frequencyFolder *freq, string rootPath);
	~analyzedDataFolder();

private :
	void getStatsInfo(string path);
	void getPicturesFolder(string path);
	int strToWeightInt(string myStr);

public :
	string typeDrawing;
	string StatAnalysis;
	vector<picData> pictures;
	//===
	frequencyFolder *parent = nullptr;
	string pathToFolder = "";
};

class frequencyFolder
{
public:
	frequencyFolder(locaFolder *loca, string rootPath);
	~frequencyFolder();
	QString freqFullName();
	bool hasTrialMap();
	bool hasEnvBar();

private :
	void getFreqBandName(string path);
	void retrieveSMFile(string path);
	void retrieveDataFolders(string path);


public:
	string sm0eeg;
	string sm250eeg;
	string sm500eeg;
	string sm1000eeg;
	string sm2500eeg;
	string sm5000eeg;
	vector<analyzedDataFolder> dataFolders;
	//===
	locaFolder* parent = nullptr;
	string pathToFolder = "";
	string frequencyName = "";
};

class locaFolder
{
public:
	locaFolder(patientFolder *pat, string rootPath);
	~locaFolder();
	QString locaFullName();

private :
	void getLocaName(string path);
	void retrieveFiles(string path);
	void retrieveFrequencyFolders(string path);

public:
	string trcFile = "";
	string eegFile = "";
	string eegEntFile = "";
	string posFile = "";
	string dsPosFile = "";
	vector<frequencyFolder> freqFolder;
	//===
	patientFolder* parent = nullptr;
	string pathToFolder = "";
	string locaName = "";
	bool analyseMe = false;
};

class patientFolder
{
public:
	patientFolder(patientFolder *pat);
	patientFolder(string rootPath);
	~patientFolder();
	QString patientFullName();

private :
	void getPatientInfo(string path);
	void findLocaFolders(string pathToBrowse);

public:
	string pathToFolder = "";
	string hospital = "";
	string year = "";
	string patientName = "";
	vector<locaFolder> localizerFolder;
};

#endif