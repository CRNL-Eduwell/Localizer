#include "singleFile.h"

singleFile::singleFile(std::string path, int nbFreqBands)
{
	getFileData(path);
	for (int i = 0; i < nbFreqBands; i++)
		m_freqFiles.push_back(singleFileFreq());
}

singleFile::~singleFile()
{

}

std::string singleFile::filePath(InsermLibrary::FileExt wantedFile)
{
	switch (wantedFile)
	{
    case InsermLibrary::FileExt::TRC:
		return m_trcFile;
		break;
    case InsermLibrary::FileExt::EEG_ELAN:
		return m_eegFile;
		break;
    case InsermLibrary::FileExt::ENT_ELAN:
		return m_eegEntFile;
		break;
    case InsermLibrary::FileExt::POS_ELAN:
		return m_posFile;
		break;
    case InsermLibrary::FileExt::POS_DS_ELAN:
		return m_dsPosFile;
		break;
    case InsermLibrary::FileExt::EDF:
		return m_edfFile;
		break;
    case InsermLibrary::FileExt::NO_EXT:
		return "";
		break;
	}
	return "";
}

InsermLibrary::FileExt singleFile::fileExtention()
{
	if (m_trcFile != "")
        return InsermLibrary::FileExt::TRC;
	else if (m_eegFile != "")
        return InsermLibrary::FileExt::EEG_ELAN;
	else if (m_edfFile != "")
        return InsermLibrary::FileExt::EDF;
	else
        return InsermLibrary::FileExt::NO_EXT;
}

void singleFile::getFileData(std::string path)
{
    std::vector<std::string> splitExt = InsermLibrary::split<std::string>(path, "\\/.");
	if (path[0] == '/' && path[1] == '/') //In case this is a newtork ressource
	{
		m_rootFolder = "//";
	}
	else
	{
		if (path[0] == '/') //In case this is a linux path
			m_rootFolder = "/";
	}

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
