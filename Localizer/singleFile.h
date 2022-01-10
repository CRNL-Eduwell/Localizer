#ifndef SINGLEFILE_H
#define SINGLEFILE_H

#include <iostream>
#include <vector>
#include "Utility.h"

#include <QDir>
#include <QString>
#include <QObject>
#include <QRegularExpression>

struct singleFileFreq
{
	std::string sm0eeg;
	std::string sm250eeg;
	std::string sm500eeg;
	std::string sm1000eeg;
	std::string sm2500eeg;
	std::string sm5000eeg;
};

class singleFile
{
public:
	singleFile(std::string path, int nbFreqBands);
	~singleFile();
	inline std::vector<singleFileFreq> frequencyFolders() const { return m_freqFiles; };
	inline std::string patientName() const { return m_patientName; };
	std::string filePath(InsermLibrary::FileExt wantedFile);
	inline std::string rootFolder() const { return m_rootFolder; };
	InsermLibrary::FileExt fileExtention();

private :
	void getFileData(std::string path);

private :
	std::string m_trcFile = "";
	std::string m_eegFile = "";
	std::string m_eegEntFile = "";
	std::string m_posFile = "";
	std::string m_dsPosFile = "";
	std::string m_edfFile = "";
	std::vector<singleFileFreq> m_freqFiles;
	std::string m_rootFolder = "";
	std::string m_patientName = "";
	std::string m_fileExtention = "";
};

#endif
