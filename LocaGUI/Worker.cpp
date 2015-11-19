#include "Worker.h"
#include "QCoreApplication"
#include "Windows.h"

// --- CONSTRUCTOR ---
Worker::Worker(std::vector<std::vector<double>> p_freqBandValue, std::vector <std::vector<bool>> p_anaDetails, std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks)
{
	// you could copy data from constructor arguments to internal variables here.

	numberFiles = p_trc.size();
	locaAnaOpt = new InsermLibrary::LOCAANALYSISOPTION*[numberFiles];
	for (int i = 0; i < numberFiles; i++)
	{
		locaAnaOpt[i] = new InsermLibrary::LOCAANALYSISOPTION(p_freqBandValue, p_anaDetails, p_trc[i], p_prov[i], folderPatient, p_tasks[i], p_exptasks[i]);
	}

	qRegisterMetaType<InsermLibrary::ELAN*>("InsermLibrary::ELAN*");
	
	//get every pointer to connect when creation worker 
	loca = new InsermLibrary::LOCA();

}

// --- DECONSTRUCTOR ---
Worker::~Worker() {
	// free resources
	for (int i = 0; i < numberFiles; i++)
	{
		delete locaAnaOpt[i];
	}
	delete locaAnaOpt;
}

InsermLibrary::LOCA * Worker::returnLoca()
{
	return loca;
}

//==========================================  Methods  =========================================== 
//===========================================  Slots  =========================================== 
//Start processing data.
void Worker::process() 
{
	// allocate resources using new here
	//qDebug("Hello World!");

	std::stringstream TRCfilePath, LOCAfilePath, displayText, TimeDisp;
	SYSTEMTIME LocalTime;

	for (int i = 0; i < numberFiles; i++)
	{
		//To get the local time
		GetLocalTime(&LocalTime);
		TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond;
		emit sendLogInfo(QString::fromStdString(TimeDisp.str()));

		std::stringstream().swap(TRCfilePath);
		TRCfilePath << locaAnaOpt[i]->patientFolder << "/" << locaAnaOpt[i]->expTask << "/" << locaAnaOpt[i]->trcPath;

		displayText << "Processing : " << TRCfilePath.str();
		emit sendLogInfo(QString::fromStdString(displayText.str()));

		// 1) Faire l'analyse du TRC pour récupérer les data
		MicromedLibrary::TRC *trc = new MicromedLibrary::TRC();
		trc->HeaderInformations(TRCfilePath.str());

		/************************/
		/* Extraction des Notes */
		/************************/
		trc->DescriptorNote(TRCfilePath.str(), 208);
		trc->NoteOperator(TRCfilePath.str(), trc->noteStart, trc->noteLength);

		/***************************/
		/* Extraction des Triggers */
		/***************************/
		trc->DescriptorTrigger(TRCfilePath.str(), 400);
		trc->DigitalTriggers(TRCfilePath.str(), trc->triggerStart, trc->triggerLength);

		//Extract chanel names and reorder them according their position in TRC file																									  
		trc->DescriptorElectrode(TRCfilePath.str(), 192);
		trc->ElectrodePresence(TRCfilePath.str(), trc->electrodeStart, trc->electrodeLength);
		trc->SortElectrodeFromFile();
		trc->ExtractAllChanels(TRCfilePath.str());

		emit sendLogInfo(QString::fromStdString("Data extracted ! "));

		if (i == 0)
		{
			elan = new InsermLibrary::ELAN(trc, locaAnaOpt[i]->frequencys.size());

			emit sendElanPointer(elan); //send Elan pointer to pop up window to choose Elec to analyse
			while (bip == false) //While bipole not created 
			{
				QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
			}

			emit sendLogInfo(QString::fromStdString("Bipole created !"));
		}
		else
		{
			delete elan->trc;
			elan->trc = nullptr;
			elan->trc = trc;
		}

		std::stringstream().swap(displayText);
		displayText << "Number of Bip : " << elan->m_bipole.size();
		emit sendLogInfo(QString::fromStdString(displayText.str()));

		emit sendLogInfo(QString::fromStdString("Beginning Analysis ..."));

		InsermLibrary::PROV *p_provVISU = new InsermLibrary::PROV(locaAnaOpt[i]->provPath);
		LOCAfilePath << locaAnaOpt[i]->patientFolder << "/" << locaAnaOpt[i]->expTask;

		if (locaAnaOpt[i]->task == "VISU")
		{
			loca->LocaVISU(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "LEC1")
		{
			loca->LocaLEC1(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "MCSE")
		{
			loca->LocaMCSE(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "MVIS")
		{
			loca->LocaMVIS(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "MVEB")
		{
			loca->LocaMVEB(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "MASS")
		{
			loca->LocaMASS(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "LEC2")
		{
			loca->LocaLEC2(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "MOTO")
		{
			loca->LocaMOTO(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "AUDI")
		{
			loca->LocaAUDI(elan, p_provVISU, locaAnaOpt[i]);
		}
		else if (locaAnaOpt[i]->task == "ARFA")
		{
			loca->LocaARFA(elan, p_provVISU, locaAnaOpt[i]);
		}

		std::stringstream().swap(TimeDisp);
		GetLocalTime(&LocalTime);
		TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond;
		emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
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
