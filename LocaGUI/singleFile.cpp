#include "singleFile.h"

singleFile::singleFile(string path, int numFreq)
{
	getFileData(path);
	for (int i = 0; i < numFreq; i++)
		freqFiles.push_back(singleFileFreq());
}

singleFile::~singleFile()
{

}

string singleFile::filePath()
{
	if (trcFile != "")
		return trcFile;
	else if (eegFile != "")
		return eegFile;
	else
		return "";
}

void singleFile::getFileData(string path)
{
	vector<string> splitNameShort = split<string>(path, "\\//");
	shortName = splitNameShort[splitNameShort.size() - 1];

	vector<string> splitExt = split<string>(path, "\\/.");
	fileExtention = splitExt[splitExt.size() - 1];

	vector<string> splitName = split<string>(path, "\\/");
	QStringList splitRoot = QString(path.c_str()).split(splitName[splitName.size() - 1].c_str(), QString::SplitBehavior::SkipEmptyParts);
	rootFolder = splitRoot[0].toStdString();

	if (fileExtention.find("trc") != string::npos ||
		fileExtention.find("TRC") != string::npos)
	{
		trcFile = path;
	}
	else if (fileExtention.find("eeg") != string::npos ||
			 fileExtention.find("EEG") != string::npos)
	{
		eegFile = path;
	}
	else if (fileExtention.find("eeg.ent") != string::npos ||
		fileExtention.find("EEG.ENT") != string::npos)
	{
		eegEntFile = path;
	}
}