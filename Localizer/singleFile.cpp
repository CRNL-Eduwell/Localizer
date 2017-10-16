#include "singleFile.h"

singleFile::singleFile(string path, int nbFreqBands)
{
	getFileData(path);
	for (int i = 0; i < nbFreqBands; i++)
		m_freqFiles.push_back(singleFileFreq());
}

singleFile::~singleFile()
{

}

string singleFile::filePath(FileExt wantedFile)
{
	switch (wantedFile)
	{
	case TRC:
		return m_trcFile;
		break;
	case EEG_ELAN:
		return m_eegFile;
		break;
	case ENT_ELAN:
		return m_eegEntFile;
		break;
	case POS_ELAN:
		return m_posFile;
		break;
	case POS_DS_ELAN:
		return m_dsPosFile;
		break;
	case EDF:
		return m_edfFile;
		break;
	case NO_EXT:
		return "";
		break;
	}
	return "";
}

FileExt singleFile::fileExtention()
{
	if (m_trcFile != "")
		return TRC;
	else if (m_eegFile != "")
		return EEG_ELAN;
	else if (m_edfFile != "")
		return EDF;
	else
		return NO_EXT;
}

void singleFile::getFileData(string path)
{
	vector<string> splitExt = split<string>(path, "\\/.");
	for (int i = 0; i < splitExt.size() - 2; i++)
		m_rootFolder += splitExt[i] + "/";
	m_patientName = splitExt[splitExt.size() - 2];
	m_fileExtention = splitExt[splitExt.size() - 1];

	QDir currentDir(m_rootFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.trc" << "*.eeg" << "*.eeg.ent" << "*.pos" << "*.edf");

	QRegExp rxTrc(QString::fromStdString(m_patientName) + ".trc", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEeg(QString::fromStdString(m_patientName) + ".eeg", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEnt(QString::fromStdString(m_patientName) + ".eeg.ent", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxPos(QString::fromStdString(m_patientName) + "_raw.pos", Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEdf(QString::fromStdString(m_patientName) + ".edf", Qt::CaseSensitivity::CaseInsensitive);

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		if (rxTrc.exactMatch(fileFound[i]))
			m_trcFile = m_rootFolder + fileFound[i].toStdString();
		if (rxEeg.exactMatch(fileFound[i]))
			m_eegFile = m_rootFolder + fileFound[i].toStdString();
		if (rxEnt.exactMatch(fileFound[i]))
			m_eegEntFile = m_rootFolder + fileFound[i].toStdString();
		if (rxPos.exactMatch(fileFound[i]))
			m_posFile = m_rootFolder + fileFound[i].toStdString();
		if (rxEdf.exactMatch(fileFound[i]))
			m_edfFile = m_rootFolder + fileFound[i].toStdString();
	}
}