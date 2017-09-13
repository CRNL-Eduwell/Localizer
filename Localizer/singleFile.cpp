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
	shortName = splitNameShort[splitNameShort.size() - 2];

	vector<string> splitExt = split<string>(path, "\\/.");
	fileExtention = splitExt[splitExt.size() - 1];

	vector<string> splitName = split<string>(path, "\\/");
	QStringList splitRoot = QString(path.c_str()).split(splitName[splitName.size() - 1].c_str(), QString::SplitBehavior::SkipEmptyParts);
	rootFolder = splitRoot[0].toStdString();

	QDir currentDir(rootFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent" << "*.pos" << "*.trc");

	QRegExp rxTrc(QString::fromStdString(shortName) + ".trc", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEeg(QString::fromStdString(shortName) + ".eeg", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEnt(QString::fromStdString(shortName) + ".eeg.ent", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxPos(QString::fromStdString(shortName) + "_raw.pos", Qt::CaseSensitivity::CaseInsensitive);

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		if (rxTrc.exactMatch(fileFound[i]))
			trcFile = rootFolder + fileFound[i].toStdString();
		if (rxEeg.exactMatch(fileFound[i]))
			eegFile = rootFolder + fileFound[i].toStdString();
		if (rxEnt.exactMatch(fileFound[i]))
			eegEntFile = rootFolder + fileFound[i].toStdString();
		if (rxPos.exactMatch(fileFound[i]))
			posFile = rootFolder + fileFound[i].toStdString();
	}
}