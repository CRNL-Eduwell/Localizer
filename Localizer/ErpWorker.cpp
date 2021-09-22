#include "ErpWorker.h"

ErpWorker::ErpWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& provFiles, InsermLibrary::picOption picOption)
{
    m_EegFiles = std::vector<std::string>(eegFiles);
    m_ProvFiles = std::vector<std::string>(provFiles);

    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> vec;
    m_Loca = new InsermLibrary::LOCA(vec, nullptr, new InsermLibrary::picOption(picOption));
}

ErpWorker::~ErpWorker()
{

}

void ErpWorker::Process()
{
    std::string provFolderPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/";
    for (size_t i = 0; i < m_EegFiles.size(); i++)
	{
        InsermLibrary::eegContainer *myContainer = ExtractData(m_EegFiles[i]);
        InsermLibrary::PROV *myprovFile = new InsermLibrary::PROV(provFolderPath + m_ProvFiles[i]);
		if (myprovFile != nullptr)
            m_Loca->eeg2erp(myContainer, myprovFile);
		deleteAndNullify1D(myContainer);
		deleteAndNullify1D(myprovFile);
        emit incrementAdavnce(1);
	}

	emit sendLogInfo("End of ERP Analysis");
	emit finished();
}

InsermLibrary::eegContainer* ErpWorker::ExtractData(std::string currentFile)
{
    InsermLibrary::eegContainer *myContainer = GetEegContainer(currentFile, true, 0);
    myContainer->DeleteElectrodes(m_IndexToDelete);
    myContainer->GetElectrodes();
    myContainer->BipolarizeElectrodes();

    emit sendLogInfo(QString::fromStdString("Bipole created !"));
    return myContainer;
}

void ErpWorker::ExtractElectrodeList()
{
    std::vector<std::string> ElectrodeList = ExtractElectrodeListFromFile(m_EegFiles[0]);
    std::string connectCleanerFilePath = "";
    emit sendElectrodeList(ElectrodeList, connectCleanerFilePath);
}

