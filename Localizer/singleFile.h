#ifndef SINGLEFILE_H
#define SINGLEFILE_H

#include <iostream>
#include "Utility.h"

#include <QDir>
#include <QString>
#include <QObject>

using namespace std;
using namespace InsermLibrary;

struct singleFileFreq
{
	string sm0eeg;
	string sm250eeg;
	string sm500eeg;
	string sm1000eeg;
	string sm2500eeg;
	string sm5000eeg;
};

class singleFile
{
public:
	singleFile(string path, int numFreq);
	~singleFile();
	string filePath();
private :
	void getFileData(string path);
public :
	string trcFile = "";
	string eegFile = "";
	string eegEntFile = "";
	string posFile = "";
	string dsPosFile = "";
	vector<singleFileFreq> freqFiles;
	string rootFolder = "";
	string shortName = "";
	string fileExtention = "";
};

#endif