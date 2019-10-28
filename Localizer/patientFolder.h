#ifndef PATIENTFOLDER_H
#define PATIENTFOLDER_H

#include <iostream>
#include <vector>
#include "Utility.h"
#include "../../EEGFormat/EEGFormat/Utility.h"

#include <QDir>
#include <QString>
#include <QObject>

class patientFolder;
class locaFolder;
class frequencyFolder;

struct picData
{
	std::string pathToPic;
	int sortingWeight;
};

class analyzedDataFolder
{
public :
	analyzedDataFolder(frequencyFolder *freq, std::string rootPath);
	~analyzedDataFolder();
	inline std::string typeDrawing() const { return m_typeDrawing; };

private :
	void getStatsInfo(std::string rootDataFolder);
	void getPicturesFolder(std::string rootDataFolder);
	int strToWeightInt(std::string myStr);

private :
	std::string m_typeDrawing;
	std::string m_StatAnalysis;
	std::vector<picData> m_pictures;
	//===
	std::string m_rootDataFolder = "";
	frequencyFolder *parent = nullptr;
};

class frequencyFolder
{
public:
	frequencyFolder(locaFolder *loca, std::string rootPath);
	~frequencyFolder();
	inline std::vector<analyzedDataFolder> dataFolders() const { return m_dataFolders; };
	inline std::string frequencyName() const { return m_frequencyName; };
	std::string fullFrequencyName();
	std::string filePath(InsermLibrary::FileExt wantedFile);
	std::vector<std::string> FilePaths(InsermLibrary::FileExt wantedFile);
	inline std::string rootFrequencyFolder() const { return m_rootFrequencyFolder; };
	bool hasTrialMap();
	bool hasEnvBar();

private :
	void getFreqBandName(std::string rootFreqFolder);
	void retrieveSMFile2(std::string rootFreqFolder);
	void retrieveSMFile(std::string rootFreqFolder);
	void retrieveDataFolders(std::string rootFreqFolder);
	std::vector<std::string> GetFirstFullDataSet(const std::vector<std::string>& rawFilePaths);

private:
	std::string m_sm0eeg;
	std::string m_sm250eeg;
	std::string m_sm500eeg;
	std::string m_sm1000eeg;
	std::string m_sm2500eeg;
	std::string m_sm5000eeg;
	std::vector<std::vector<std::string>> m_smXFiles;
	std::vector<analyzedDataFolder> m_dataFolders;
	//===
	std::string m_frequencyName = "";
	std::string m_rootFrequencyFolder = "";
	locaFolder* parent = nullptr;
};

class locaFolder
{
public:
	locaFolder(patientFolder *pat, std::string rootLocaFolder);
	~locaFolder();
	inline std::vector<frequencyFolder> frequencyFolders() const { return m_freqFolder; };
	inline std::string localizerName() const { return m_locaName; };
	std::string fullLocalizerName();
	InsermLibrary::FileExt fileExtention();
	std::string filePath(InsermLibrary::FileExt wantedFile);
	inline std::string rootLocaFolder() const { return m_rootLocaFolder; };

private :
	void getLocaName(std::string rootLocaFolder);
	void retrieveFiles(std::string rootLocaFolder);
	void retrieveFrequencyFolders(std::string rootLocaFolder);

private:
	std::string m_trcFile = "";
	std::string m_eegFile = "";
	std::string m_eegEntFile = "";
	std::string m_posFile = "";
	std::string m_dsPosFile = "";
	std::string m_edfFile = "";
    std::vector<frequencyFolder> m_freqFolder;
	//===
	std::string m_locaName = "";
	std::string m_rootLocaFolder = "";
	patientFolder* parent = nullptr;
};

class patientFolder
{
public:
	patientFolder(patientFolder *pat);
	patientFolder(std::string rootPath);
	~patientFolder();
	inline std::vector<locaFolder> localizerFolder() const { return m_localizerFolder; };
	inline std::vector<locaFolder> & localizerFolder() { return m_localizerFolder; }
	std::string patientName();
	inline std::string rootFolder() const { return m_rootFolder; };

private :
	void getPatientInfo(std::string rootPath);
	void findLocaFolders(std::string rootPath);

private:
	std::string m_rootFolder = "";
	std::string m_hospital = "";
	std::string m_year = "";
	std::string m_patientName = "";
	std::vector<locaFolder> m_localizerFolder;
};

#endif
