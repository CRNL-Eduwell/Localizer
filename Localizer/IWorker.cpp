#include "IWorker.h"

IWorker::IWorker()
{

}

IWorker::~IWorker()
{
	deleteAndNullify1D(m_loca);
}

InsermLibrary::eegContainer* IWorker::GetEegContainer(std::string currentFilePath, bool shouldExtractData, int nbFreqBand)
{	
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFilePath));
	EEGFormat::IFile* file = CreateGenericFile(currentFilePath.c_str(), shouldExtractData);
    return new InsermLibrary::eegContainer(file, 64, nbFreqBand);;
}
