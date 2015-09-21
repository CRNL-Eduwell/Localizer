#include "Worker.h"
#include "QCoreApplication"
#include "Windows.h"
// --- CONSTRUCTOR ---
Worker::Worker(std::vector<std::vector<double>> p_freqBandValue, std::vector <std::vector<bool>> p_anaDetails, std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks)
{
	// you could copy data from constructor arguments to internal variables here.
	freqBandValue = p_freqBandValue;
	anaDetails = p_anaDetails;
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

		InsermLibrary::ELAN *elan = new InsermLibrary::ELAN(trc, anaDetails.size());

		emit sendElanPointer(elan); //send Elan pointer to pop up window to choose Elec to analyse
		while (bip == false) //While bipole not created 
		{
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
		}
		
		emit sendLogInfo(QString::fromStdString("Bipole created !"));
		emit sendLogInfo(QString::fromStdString("Beginning Analysis ..."));

		InsermLibrary::LOCA *loca = new InsermLibrary::LOCA();
		InsermLibrary::PROV *p_provVISU = new InsermLibrary::PROV(provFiles[i]);
		std::stringstream locaPath;
		locaPath << patientFolder << "/" << expTasks[i];

		//loca->LocaVISU(elan, p_provVISU, locaPath.str(), expTasks[i], tasks[i]);
		
		double frequencyGamma[11]{ 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 };
		int *code = new int[11]{10, 20, 30, 40, 50, 60, 70, 90, 110, 100, 80};
		std::string *strCode = new std::string[11]{"Maison", "Visage", "Animal", "Scene", "Objet", "Pseudo", "Conson", "Scrambled", "Bfix", "Pause", "Fruits"};
		int *window_ms = new int[2]{-500, 1000};
		std::string pathFreq;

		std::stringstream sss;
		SYSTEMTIME LocalTime;
		GetLocalTime(&LocalTime);
		sss << LocalTime.wHour << "h" << LocalTime.wMinute << "m" << LocalTime.wSecond << "s";
		emit sendLogInfo(QString::fromStdString(sss.str()));
		//======================== Generalisation Loca ==============================//

		std::stringstream posFilePath, confFilePath, posXFilePath, pictureLabel, folderTrialsSM;

		/*****************************************************************************************************************/
		/*											Create .pos and dsX.pos File										 */
		/*****************************************************************************************************************/
		posFilePath << locaPath.str() << "/" << expTasks[i] << ".pos";													 //
		posXFilePath << locaPath.str() << "/" << expTasks[i] << "_ds" << (elan->trc->samplingFrequency / 64) << ".pos";  //
		loca->loc_create_pos(posFilePath.str(), posXFilePath.str(), elan->trc, 99, p_provVISU);							 //
		/*****************************************************************************************************************/

		emit sendLogInfo(QString::fromStdString("Pos Created"));

		/*****************************************************************************************************/
		/*											Create .conf File										 */
		/*****************************************************************************************************/
		confFilePath << locaPath.str() << "/" << expTasks[i] << ".conf";									 //
		loca->loc2_write_conf(confFilePath.str(), elan->trc, elan);											 //
		/*****************************************************************************************************/

		emit sendLogInfo(QString::fromStdString("Conf Created"));

		/******************************************************************************************************************************/
		/*														Loc eeg2erp														      */
		/******************************************************************************************************************************/
		loca->loc_eeg2erp(elan, locaPath.str(), expTasks[i], code, 11, strCode, 11, window_ms, 20);									  //
		/******************************************************************************************************************************/

		emit sendLogInfo(QString::fromStdString("ERP Pictures Done"));

		for (int j = 0; j < anaDetails.size(); j++)
		{
			/*****************************************************************************************************************************/			
			/*									EEG2ENV	50Hz -> 150Hz																	 */					
			/*****************************************************************************************************************************/			
			elan->TrcToEnvElan(trcFiles[i].c_str(), elan->elanFreqBand[j], &freqBandValue[i][0], freqBandValue[i].size(), ELAN_HISTO);	 //					
			/*****************************************************************************************************************************/
			emit sendLogInfo(QString::fromStdString("Hilbert Envellope Done"));

			pictureLabel << expTasks[i] << "_f" << freqBandValue[i][0] << "f" << freqBandValue[i][freqBandValue[i].size() - 1] << "_ds" << (elan->trc->samplingFrequency / 64) << "_sm0_trials_";
			folderTrialsSM << locaPath.str() << "/" << expTasks[i] << "_f" << freqBandValue[i][0] << "f" << freqBandValue[i][freqBandValue[i].size() - 1];
																																	
			if (!QDir(&folderTrialsSM.str()[0]).exists())																			
			{																														
				//std::cout << "Creating Output Folder for 50-150 Hz data " << std::endl;													
				QDir().mkdir(&folderTrialsSM.str()[0]);																				
			}

			/*****************************************************************************************************/					
			/*											loc_env2plot											 */					
			/*****************************************************************************************************/					
			pathFreq = "";																						 //				
			pathFreq.append(expTasks[i]);																		 //	
			if (j == 0)
			{
				pathFreq.append("_f50f150_ds8_sm0");																 //					
			}
			else
			{
				pathFreq.append("_f8f24_ds8_sm0");																 //					
			}
			loca->loc_env2plot(elan, j, locaPath.str(), pathFreq, code, 11, strCode, 11, window_ms, 20);		 //					
			/*****************************************************************************************************/				
			//
			/*****************************************************************************************************/				
			/*											loca trialmat											 */				
			/*****************************************************************************************************/					
			//ef_read_elan_file((char*)"D:\\Users\\Florian\\Documents\\Arbeit\\2) Loca Patient\\2014\\LYONNEURO_2014_RENT\\LYONNEURO_2014_RENT_VISU\\LYONNEURO_2014_RENT_VISU_f50f150_ds8_sm0.eeg", p_elan->elanFreqBand[0]);
			loca->loca_trialmat(elan, j, p_provVISU, pictureLabel.str(), folderTrialsSM.str());							 //					    
		}

		std::stringstream().swap(sss);
		GetLocalTime(&LocalTime);
		sss << LocalTime.wHour << "h" << LocalTime.wMinute << "m" << LocalTime.wSecond << "s";
		emit sendLogInfo(QString::fromStdString(sss.str()));
		
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
