#include "Worker.h"
#include "QCoreApplication"

// --- CONSTRUCTOR ---
Worker::Worker(std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks)
{
	// you could copy data from constructor arguments to internal variables here.
	trcFiles = p_trc;
	provFiles = p_prov;
	patientFolder = folderPatient;
	tasks = p_tasks;
	expTasks = p_exptasks;

	qRegisterMetaType<InsermLibrary::ELAN*>("InsermLibrary::ELAN*");

}

// --- DECONSTRUCTOR ---
Worker::~Worker() {
	// free resources
}

//===========================================  Slots  =========================================== 
// Start processing data.
void Worker::process() 
{
	// allocate resources using new here
	//qDebug("Hello World!");

	std::stringstream filePath, displayText;
	for (int i = 0; i < trcFiles.size(); i++)
	{
		std::stringstream().swap(filePath);
		filePath << patientFolder << "/" << expTasks[i] << "/" << trcFiles[i];

		displayText << "Processing : " << filePath.str();
		emit sendLogInfo(QString::fromStdString(displayText.str()));

		// 1) Faire l'analyse du TRC pour récupérer les data
		MicromedLibrary::TRC *trc = new MicromedLibrary::TRC();
		trc->HeaderInformations(filePath.str());
		
		/************************/
		/* Extraction des Notes */
		/************************/
		trc->DescriptorNote(filePath.str(), 208);
		trc->NoteOperator(filePath.str(), trc->noteStart, trc->noteLength);
		
		/***************************/
		/* Extraction des Triggers */
		/***************************/
		trc->DescriptorTrigger(filePath.str(), 400);
		trc->DigitalTriggers(filePath.str(), trc->triggerStart, trc->triggerLength);
		
		//Extract chanel names and reorder them according their position in TRC file																									  
		trc->DescriptorElectrode(filePath.str(), 192);
		trc->ElectrodePresence(filePath.str(), trc->electrodeStart, trc->electrodeLength);
		trc->SortElectrodeFromFile();
		trc->ExtractAllChanels(filePath.str());

		emit sendLogInfo(QString::fromStdString("Data extracted ! "));

		InsermLibrary::ELAN *elan = new InsermLibrary::ELAN(trc, 2);

		emit sendElanPointer(elan);
		while (bip == false)
		{
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
		}
		
		emit sendLogInfo(QString::fromStdString("Bipole created !"));
		emit sendLogInfo(QString::fromStdString("Beginning Analysis ..."));

		InsermLibrary::LOCA *loca = new InsermLibrary::LOCA();
		InsermLibrary::PROV *p_provVISU = new InsermLibrary::PROV(provFiles[i]);
		std::stringstream locaPath;
		locaPath << patientFolder << "/" << expTasks[i];

		loca->LocaVISU(elan, p_provVISU, locaPath.str(), expTasks[i], tasks[i]);
		emit sendLogInfo(QString::fromStdString("End of Loca \n"));
	}
	emit sendLogInfo(QString::fromStdString("ByeBye"));
	emit finished();
}

void Worker::BipDoneeee(bool boo)
{
	if (boo == true)
	{
		bip = true;
	}
}
