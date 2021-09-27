#include "SingleFilesWorker.h"

SingleFilesWorker::SingleFilesWorker(std::vector<singleFile>& singleFiles, std::vector<InsermLibrary::FrequencyBandAnalysisOpt>& FrequencyBands)
{
    m_currentFiles = std::vector<singleFile>(singleFiles);
    m_frequencyBands = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(FrequencyBands);

    m_Loca = new InsermLibrary::LOCA(m_frequencyBands, nullptr, nullptr);
}

SingleFilesWorker::~SingleFilesWorker()
{
    //m_loca destroyed in base class
}

void SingleFilesWorker::Process()
{
    singleFile file = m_currentFiles[m_CurrentProcessId];
    emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + file.rootFolder() + " ==="));
    InsermLibrary::eegContainer *myContainer = ExtractData(file, true);

    if (myContainer != nullptr)
    {
        emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
        //==
        emit sendLogInfo(GetCurrentTime().c_str());
        //==
        m_Loca->LocalizeMapsOnly(myContainer, m_CurrentProcessId);
        //==
        emit sendLogInfo(GetCurrentTime().c_str());
        //==
        sendLogInfo("End of File number " + QString::number(m_CurrentProcessId) + "\n");
        deleteAndNullify1D(myContainer);
    }

    m_CurrentProcessId++;
    if(m_CurrentProcessId < static_cast<int>(m_currentFiles.size()))
    {
        ExtractElectrodeList();
    }
    else
    {
        emit finished();
    }
}

void SingleFilesWorker::ExtractElectrodeList()
{
    if(m_CurrentProcessId >= static_cast<int>(m_currentFiles.size()))
    {
        throw new std::runtime_error("Error ExtractElectrodeList : ProcessID is greater than the number of file to process");
    }

    singleFile file = m_currentFiles[m_CurrentProcessId];
    InsermLibrary::FileExt currentExtention = file.fileExtention();
    if (currentExtention == InsermLibrary::NO_EXT)
    {
        throw new std::runtime_error("Error ExtractElectrodeList : FileExtention is unknown");
    }

    std::string currentFilePath = file.filePath(currentExtention);
    std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(currentFilePath);
    std::string connectCleanerFilePath = "";
    emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
}

InsermLibrary::eegContainer* SingleFilesWorker::ExtractData(singleFile currentFile, bool extractOriginalData)
{
    InsermLibrary::FileExt currentExtention = currentFile.fileExtention();
    if (currentExtention == InsermLibrary::NO_EXT)
        return nullptr;
    std::string currentFilePath = currentFile.filePath(currentExtention);

    InsermLibrary::eegContainer *myContainer = GetEegContainer(currentFilePath, extractOriginalData);
    myContainer->DeleteElectrodes(m_IndexToDelete);
    myContainer->GetElectrodes();
    myContainer->BipolarizeElectrodes();

    emit sendLogInfo(QString::fromStdString("Bipole created !"));
    return myContainer;
}
