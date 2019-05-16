#include "ConcatenationWorker.h"

ConcatenationWorker::ConcatenationWorker(std::vector<std::string>& trcFiles, std::string& directoryPath, std::string& fileName)
{
	m_eegFiles = std::vector<std::string>(trcFiles);
	m_directoryPath = directoryPath;
	m_fileName = fileName;
}

ConcatenationWorker::~ConcatenationWorker()
{

}

void ConcatenationWorker::Process()
{
	if (m_eegFiles.size() < 2)
	{
		emit(sendLogInfo("Error : You need at least to file to perform concatenation."));
		emit finished();
		return;
	}

	if (EEGFormat::Utility::IsValidFile(m_directoryPath + m_fileName))
	{
		emit(sendLogInfo("Error : You need to input a valid file path for the output file."));
		emit finished();
		return;
	}

	EEGFormat::MicromedFile* firstTRC = nullptr;
	bool firstFileFound = false;
	int concatenationCount = 0;
	for (int i = 0; i < m_eegFiles.size(); i++)
	{
		if (!firstFileFound && EEGFormat::Utility::IsValidFile(m_eegFiles[i]))
		{
			std::string firstFile = m_eegFiles[i];
			firstTRC = new EEGFormat::MicromedFile(firstFile);
			firstFileFound = true;
			continue;
		}

		if (EEGFormat::Utility::IsValidFile(m_eegFiles[i]))
		{	
			//Do the stuff
			std::string secondFile = m_eegFiles[i];
			EEGFormat::MicromedFile* secondTRC = new EEGFormat::MicromedFile(secondFile);
			//EEGFormat::MicromedFile::ConcatenateFiles("", firstTRC, secondTRC);
			//EEGFormat::MicromedFile::StapleFiles("", firstTRC, secondTRC);
			concatenationCount++;
			EEGFormat::Utility::DeleteAndNullify(secondTRC);

			QString dd = QString::fromStdString("Processing " + firstTRC->FilePath() + " et " + secondTRC->FilePath());
			emit(sendLogInfo(dd));
		}
		else
		{
			emit(sendLogInfo(QString::fromStdString(m_eegFiles[i]) + "is not valid, going to the next file."));
		}
	}

	if (firstFileFound && concatenationCount > 0)
	{
		//firstTRC->SaveAs(m_directoryPath, m_fileName);
		EEGFormat::Utility::DeleteAndNullify(firstTRC);
	}

	emit(sendLogInfo("Concatenation Process is over."));
}