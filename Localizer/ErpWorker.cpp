#include "ErpWorker.h"

using namespace InsermLibrary;

ErpWorker::ErpWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& provFiles, picOption picOption)
{
    m_EegFiles = std::vector<std::string>(eegFiles);
    m_ProvFiles = std::vector<std::string>(provFiles);

    std::vector<FrequencyBandAnalysisOpt> vec;
    m_Loca = new LOCA(vec, nullptr, new InsermLibrary::picOption(picOption));
}

ErpWorker::~ErpWorker()
{

}

void ErpWorker::Process()
{
    std::string provFolderPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/";
    for (int i = 0; i < m_EegFiles.size(); i++)
	{
        eegContainer *myContainer = ExtractData(m_EegFiles[i]);
        PROV *myprovFile = new PROV(provFolderPath + m_ProvFiles[i]);
		if (myprovFile != nullptr)
            m_Loca->eeg2erp(myContainer, myprovFile);
		deleteAndNullify1D(myContainer);
		deleteAndNullify1D(myprovFile);
        emit incrementAdavnce(1);
	}


	emit sendLogInfo("End of ERP Analysis");
	emit finished();
}

eegContainer* ErpWorker::ExtractData(std::string currentFile)
{
    eegContainer *myContainer = GetEegContainer(currentFile, true, 0);
    myContainer->DeleteElectrodes(m_IndexToDelete);
    myContainer->GetElectrodes();
    myContainer->BipolarizeElectrodes();

    emit sendLogInfo(QString::fromStdString("Bipole created !"));
    return myContainer;
}

