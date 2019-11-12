#include "IWorker.h"

IWorker::IWorker()
{

}

IWorker::~IWorker()
{
    deleteAndNullify1D(m_Loca);
}

InsermLibrary::eegContainer* IWorker::GetEegContainer(std::string currentFilePath, bool shouldExtractData, int nbFreqBand)
{	
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFilePath));
	EEGFormat::IFile* file = CreateGenericFile(currentFilePath.c_str(), shouldExtractData);
    return new InsermLibrary::eegContainer(file, 64, nbFreqBand);;
}

void IWorker::SetExternalParameters(std::vector<int> IndexToDelete, std::vector<std::string> CorrectedLabels )
{
    m_IndexToDelete = std::vector<int>(IndexToDelete);
    m_CorrectedLabels = std::vector<std::string>(CorrectedLabels);
}

void IWorker::ExtractElectrodeList(std::string currentFilePath)
{
    emit sendLogInfo(QString::fromStdString("  => Extracting Electrodes from : " + currentFilePath));
    EEGFormat::IFile* file = CreateGenericFile(currentFilePath.c_str(), false);
    std::vector<std::string> ElectrodesList = std::vector<std::string>(file->ElectrodeCount());
    for(size_t i = 0; i < ElectrodesList.size(); i++)
    {
        ElectrodesList[i] = std::string(file->Electrode(i)->Label());
    }
    DeleteGenericFile(file);
    sendElectrodeList(ElectrodesList);
}
