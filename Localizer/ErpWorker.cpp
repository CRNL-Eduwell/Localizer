#include "ErpWorker.h"

using namespace InsermLibrary;

ErpWorker::ErpWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& provFiles, picOption picOption)
{
	m_eegFiles = std::vector<std::string>(eegFiles);
	m_provFiles = std::vector<std::string>(provFiles);

    std::vector<FrequencyBandAnalysisOpt> vec;
    m_loca = new LOCA(vec, nullptr, new InsermLibrary::picOption(picOption));
}

ErpWorker::~ErpWorker()
{

}

void ErpWorker::Process()
{
    std::string provFolderPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/";
	for (int i = 0; i < m_eegFiles.size(); i++)
	{
		eegContainer *myContainer = ExtractData(m_eegFiles[i], i);
        PROV *myprovFile = new PROV(provFolderPath + m_provFiles[i]);
		if (myprovFile != nullptr)
			m_loca->eeg2erp(myContainer, myprovFile);
		deleteAndNullify1D(myContainer);
		deleteAndNullify1D(myprovFile);
		emit(incrementAdavnce(1));
	}


	emit sendLogInfo("End of ERP Analysis");
	emit finished();
}

//TODO
eegContainer* ErpWorker::ExtractData(std::string currentFile, int idFile)
{
	eegContainer *myContainer = GetEegContainer(currentFile, true, 0);

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	if (idFile > -1)
		bipCreated = -1; //Since we loop one or multiple file we need to recheck each time the good/bad elec

    m_electrodeToDeleteMemory = std::vector<int>(myContainer->idElecToDelete);

	myContainer->DeleteElectrodes(m_electrodeToDeleteMemory);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}
