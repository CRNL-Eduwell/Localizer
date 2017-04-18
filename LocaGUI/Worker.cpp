#include "Worker.h"
#include "QCoreApplication"
#include "Windows.h"

// --- CONSTRUCTOR ---
Worker::Worker(OptionLOCA *p_LOCAOpt, vector<vector<double>> p_freqBandValue, vector <vector<bool>> p_anaDetails, vector<string> p_trc, vector<string> p_prov, string folderPatient, vector<string> p_tasks, vector<string> p_exptasks)
{
	// you could copy data from constructor arguments to internal variables here.
	LOCAOpt = p_LOCAOpt;
	if (LOCAOpt == nullptr)
	{
		LOCAOpt = new OptionLOCA();
	}

	numberFiles = p_trc.size();
	locaAnaOpt = new LOCAANALYSISOPTION*[numberFiles];
	for (int i = 0; i < numberFiles; i++)
	{
		locaAnaOpt[i] = new LOCAANALYSISOPTION(p_freqBandValue, p_anaDetails, p_trc[i], p_prov[i], folderPatient, p_tasks[i], p_exptasks[i]);
	}

	//qRegisterMetaType<InsermLibrary::ELAN*>("InsermLibrary::ELAN*");
	qRegisterMetaType<ELAN*>("ELAN*");

	//get every pointer to connect when creation worker 
	loca = new LOCA(LOCAOpt);

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

LOCA * Worker::returnLoca()
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
	stringstream TRCfilePath, LOCAfilePath, displayText, TimeDisp;
	SYSTEMTIME LocalTime;
	vector<string>ElecPosSave, ElecNegSave;
	vector<unsigned int>SignalPosSave,toDelete;

	//emit sendTest(); //send to win
	//while (wclo == false) //While win not closed 
	//{
	//	QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	//}

	//emit sendLogInfo(QString::fromStdString("Bipole created !"));



	int progressPerCent = 100 / numberFiles;
	int actualPerCent = 0;
	emit upScroll(actualPerCent);
	for (int i = 0; i < numberFiles; i++)
	{
		//To get the local time
		GetLocalTime(&LocalTime);
		TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond;
		emit sendLogInfo(QString::fromStdString(TimeDisp.str()));

		stringstream().swap(TRCfilePath);
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
			elan = new ELAN(trc, locaAnaOpt[i]->frequencys.size());

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
			deleteAndNullify1D(elan->trc);
			elan->trc = trc;

			/*On a changé de trc mais ce sont les mêmes index qui sont supprimés*/
			for (int i = elan->index_supp.size() - 1; i >= 0; i--)	
			{						
				trc->nameElectrodePositiv.erase(trc->nameElectrodePositiv.begin() + elan->index_supp[i]);	
				trc->signalPosition.erase(trc->signalPosition.begin() + elan->index_supp[i]);	
				trc->nameElectrodeNegativ.erase(trc->nameElectrodeNegativ.begin() + elan->index_supp[i]);	
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

		stringstream().swap(displayText);
		displayText << "Number of Bip : " << elan->m_bipole.size();
		emit sendLogInfo(QString::fromStdString(displayText.str()));

		emit sendLogInfo(QString::fromStdString("Beginning Analysis ..."));

		PROV *p_provVISU = new PROV(locaAnaOpt[i]->provPath);
		LOCAfilePath << locaAnaOpt[i]->patientFolder << "/" << locaAnaOpt[i]->expTask;

		loca->LocaSauron(elan, p_provVISU, locaAnaOpt[i]);

		stringstream().swap(TimeDisp);
		GetLocalTime(&LocalTime);
		TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond;
		emit sendLogInfo(QString::fromStdString(TimeDisp.str()));

		deleteAndNullify1D(p_provVISU);

		actualPerCent += progressPerCent;
		emit upScroll(actualPerCent);
	}

	emit sendLogInfo(QString::fromStdString("Generating Comportemental Performance Result"));
	//generate array comportemental performance
	//Get every folder corresponding to one LOCALIZER exam
	stringstream().swap(TRCfilePath);
	TRCfilePath << locaAnaOpt[0]->patientFolder << "/";
	vector<string> directoryList = CRperf::getAllFolderNames(TRCfilePath.str());

	CRData** files = new CRData*[5];
	files[0] = files[1] = files[2] = files[3] = files[4] = nullptr;
	for (int i = 0; i < directoryList.size(); i++)
	{
		stringstream().swap(TRCfilePath);
		TRCfilePath << locaAnaOpt[0]->patientFolder << "/" << directoryList[i] << "/" << directoryList[i] << ".CR";
		QFile f(QString::fromStdString(TRCfilePath.str()));
		if (f.exists())
		{
			stringstream().swap(displayText);
			displayText << "Existing CR File : " << TRCfilePath.str();
			emit sendLogInfo(QString::fromStdString(displayText.str()));

			vector<string> dirSplit = split<string>(directoryList[i], "_");
			int numberOfConditions = CRperf::whichOneAmI(directoryList[i]);
			if (numberOfConditions != -1)
			{
				int numberOf = CRperf::whereAmI(directoryList[i]);
				files[numberOf] = CRperf::getCRInfo(TRCfilePath.str(), directoryList[i], numberOfConditions);
			}
		}
	}

	//Pass struct to create csv array
	stringstream().swap(TRCfilePath);
	TRCfilePath << locaAnaOpt[0]->patientFolder << "/RapportPerfComportementale.csv";
	CRperf::createCSVPerformanceReport(TRCfilePath.str(), files);

	emit sendLogInfo(QString::fromStdString("Done !"));
	emit sendLogInfo(QString::fromStdString("ByeBye"));

	deleteAndNullify1D(elan);
	deleteAndNullify1D(loca);
	deleteAndNullify2D(files, 4);

	emit finished();
}

void Worker::BipDoneeee(bool boo)
{
	if (boo == true)
	{
		bip = true;
	}
}

void Worker::winCloseeee(bool wClo)
{
	if (wClo == true)
	{
		wclo = true;
	}
}
