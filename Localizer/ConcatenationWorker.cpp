#include "ConcatenationWorker.h"
#include "../../EEGFormat/EEGFormat/MicromedFile.h"

ConcatenationWorker::ConcatenationWorker(std::vector<std::string>& trcFiles, std::string& directoryPath, std::string& fileName)
{
    m_EegFiles = std::vector<std::string>(trcFiles);
    m_DirectoryPath = std::string(directoryPath);
    m_FileName = std::string(fileName);
}

ConcatenationWorker::~ConcatenationWorker()
{

}

void ConcatenationWorker::Process()
{
	if (m_EegFiles.size() < 2)
	{
		emit sendLogInfo("Error : You need at least two file to perform concatenation.");
		emit finished();
		return;
	}

	std::vector<EEGFormat::MicromedFile*> filesToConcatenate;
	int fileCount = static_cast<int>(m_EegFiles.size());
	for (int i = 0; i < fileCount; i++)
	{
		filesToConcatenate.push_back(new EEGFormat::MicromedFile(m_EegFiles[i]));
	}

	EEGFormat::MicromedFile::ConcatenateFiles(m_DirectoryPath, m_FileName, filesToConcatenate);

	//then we cleanup
	for (int i = 0; i < fileCount; i++)
	{
		if (filesToConcatenate[i] != nullptr)
		{
			delete filesToConcatenate[i];
			filesToConcatenate[i] = nullptr;
		}
	}
	filesToConcatenate.clear();

	emit sendLogInfo("Concatenation Process is over.");
	emit finished();
}

void ConcatenationWorker::ExtractElectrodeList()
{
    if(m_EegFiles.size() == 0)
    {
        throw new std::runtime_error("Error, there should be at least one file to concatenateÒ");
    }
    std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(m_EegFiles[0]);
    //std::string connectCleanerFilePath = m_Patient->rootFolder() + "/" + m_Patient->patientName() + ".ccf";
    std::string connectCleanerFilePath = ""; //TODO : demander a Benoit si on fait un fichier de nettoyage lors de la conversion aussi
    emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
}
