#include "Worker.h"
#include "QCoreApplication"
#include "Windows.h"

// --- CONSTRUCTOR ---
Worker::Worker(InsermLibrary::OptionLOCA *p_LOCAOpt, std::vector<std::vector<double>> p_freqBandValue, std::vector <std::vector<bool>> p_anaDetails, std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks)
{
	// you could copy data from constructor arguments to internal variables here.
	LOCAOpt = p_LOCAOpt;
	if (LOCAOpt == nullptr)
	{
		LOCAOpt = new InsermLibrary::OptionLOCA();
	}

	numberFiles = p_trc.size();
	locaAnaOpt = new InsermLibrary::LOCAANALYSISOPTION*[numberFiles];
	for (int i = 0; i < numberFiles; i++)
	{
		locaAnaOpt[i] = new InsermLibrary::LOCAANALYSISOPTION(p_freqBandValue, p_anaDetails, p_trc[i], p_prov[i], folderPatient, p_tasks[i], p_exptasks[i]);
	}

	qRegisterMetaType<InsermLibrary::ELAN*>("InsermLibrary::ELAN*");
	
	//get every pointer to connect when creation worker 
	loca = new InsermLibrary::LOCA(LOCAOpt);

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
	std::vector<std::string>ElecPosSave, ElecNegSave;
	std::vector<unsigned int>SignalPosSave,toDelete;

	int progressPerCent = 100 / numberFiles;
	int actualPerCent = 0;
	emit upScroll(actualPerCent);
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
			/*On remplace le TRC que l'on va analysé*/
			delete elan->trc;
			elan->trc = nullptr;
			elan->trc = trc;

			/*On a changé de trc mais ce sont les mêmes index qui sont supprimés*/
			for (int i = elan->index_supp.size() - 1; i >= 0; i--)																																												  //
			{																																																							  //
				trc->nameElectrodePositiv.erase(trc->nameElectrodePositiv.begin() + elan->index_supp[i]);																																		  //
				trc->signalPosition.erase(trc->signalPosition.begin() + elan->index_supp[i]);																																					  //
				trc->nameElectrodeNegativ.erase(trc->nameElectrodeNegativ.begin() + elan->index_supp[i]);																																		  //
				trc->eegData.erase(trc->eegData.begin() + elan->index_supp[i]);
			}

			for (int i = 0; i < elan->numberFrequencyBand; i++)
			{
				if (elan->elanFreqBand[i]->chan_nb > 0)
				{
					ef_free_data_array(elan->elanFreqBand[i]);
					ef_free_struct(elan->elanFreqBand[i]);
				}
			}
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

		delete p_provVISU;
		p_provVISU = nullptr;
		
		//delete elan->trc;
		//elan->trc = nullptr;

		actualPerCent += progressPerCent;
		emit upScroll(actualPerCent);
	}

	//emit sendLogInfo(QString::fromStdString("Generating Comportemental Performance Result"));
	////generate array comportemental performance
	////Get every folder corresponding to one LOCALIZER exam
	//std::vector<std::string> directoryList;
	//std::stringstream().swap(TRCfilePath);
	//TRCfilePath << locaAnaOpt[0]->patientFolder << "/";

	//QDir currentDir(QString::fromStdString(TRCfilePath.str()));
	//currentDir.setFilter(QDir::Dirs);

	//QStringList entries = currentDir.entryList();
	//for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	//{
	//	QString dirname = *entry;
	//	if (dirname != tr(".") && dirname != tr(".."))
	//	{
	//		directoryList.push_back(dirname.toStdString());
	//	}
	//}

	//for (int i = 0; i < directoryList.size(); i++)
	//{
	//	std::stringstream().swap(TRCfilePath);
	//	TRCfilePath << locaAnaOpt[0]->patientFolder << "/" << directoryList[i] << "/" << directoryList[i] << ".CR";
	//	QFile f(QString::fromStdString(TRCfilePath.str()));
	//	if (f.exists())
	//	{
	//		std::stringstream().swap(displayText);
	//		displayText << "Existing : " << TRCfilePath.str();
	//		emit sendLogInfo(QString::fromStdString(displayText.str()));

	//		std::ifstream crfile(TRCfilePath.str(), std::ios::beg);
	//		std::string line;
	//		while (std::getline(crfile, line))
	//		{
	//			std::vector<std::string> sortSplit = loca->split<std::string>(line, " : ");
	//			int a = 2;
	//		}
	//	}
	//}

	//emit sendLogInfo(QString::fromStdString("Done !"));

	emit sendLogInfo(QString::fromStdString("ByeBye"));

	delete elan;   
	elan = nullptr;
	delete loca;
	loca = nullptr;

	emit finished();
}

void Worker::BipDoneeee(bool boo)
{
	if (boo == true)
	{
		bip = true;
	}
}
