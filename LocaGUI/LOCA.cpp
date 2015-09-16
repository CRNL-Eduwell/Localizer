/***********************************************************************************************************************************************************************************************************************************************************/
/********************************************************************************************************               Library                    *********************************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/
#include "LOCA.h"
/***********************************************************************************************************************************************************************************************************************************************************/
#include <time.h>  
#include <QDir>

/*****************************************************************************************************************************************************************************************************************************************************/
/*******************************************************************************************************               Constants                  ****************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/
																																																												 /*||*/
/*****************************************************************************************************************************************************************************************************************************************************/

using namespace std;

/*****************************************************************************************************************************************************************************************************************************************************/
/*******************************************************************************************************            TRIGG     Methods             ****************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/
InsermLibrary::TRIGG::TRIGG(int p_valueTrigger, int p_sampleTrigger, int p_rtMs, int p_rtCode, int p_origPos)
{
	valueTrigger = p_valueTrigger;
	sampleTrigger = p_sampleTrigger;
	rt_ms = p_rtMs;
	rt_code = p_rtCode;
	origPos = p_origPos;
}

InsermLibrary::TRIGG::~TRIGG()
{

}
/*****************************************************************************************************************************************************************************************************************************************************/

/*****************************************************************************************************************************************************************************************************************************************************/
/*******************************************************************************************************            TRIGGINFO Methods             ****************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/
InsermLibrary::TRIGGINFO::TRIGGINFO(unsigned long *p_valueTrigg, unsigned long *p_sampleTrigg, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	trigg = new TRIGG*[numberTrigg];
	for (int i = 0; i < numberTrigg; i++)
	{
		trigg[i] = new TRIGG(p_valueTrigg[i], p_sampleTrigg[i] / downFactor, 1000000, -1, - 1); //10000000 et -1 default value
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	trigg = new TRIGG*[numberTrigg];
	for (int i = 0; i < numberTrigg; i++)
	{
		trigg[i] = new TRIGG(p_valueTrigg[i], p_sampleTrigg[i], p_rtMs[i], -1, -1); // -1 default value
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int *p_rtCode, int *p_origPos, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	trigg = new TRIGG*[numberTrigg];
	for (int i = 0; i < numberTrigg; i++)
	{
		trigg[i] = new TRIGG(p_valueTrigg[i], p_sampleTrigg[i], p_rtMs[i], p_rtCode[i], p_origPos[i]); // -1 default value
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	trigg = new TRIGG*[numberTrigg];
	for (int i = 0; i < numberTrigg; i++)
	{
		trigg[i] = new TRIGG(p_valueTrigg[i], p_sampleTrigg[i] / downFactor, 10000000, -1, -1); //10000000 et -1 default value
	}
}

InsermLibrary::TRIGGINFO::~TRIGGINFO()
{
	for (int i = 0; i < numberTrigg; i++)
	{
		delete trigg[i];
	}
}
/*****************************************************************************************************************************************************************************************************************************************************/

/*****************************************************************************************************************************************************************************************************************************************************/
/*******************************************************************************************************               LOCA Methods               ****************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																																																											  */
/**********************************************************************************************************************************************************************************************************************************************/
InsermLibrary::LOCA::LOCA()
{

}

InsermLibrary::LOCA::~LOCA()
{

}
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																																																											  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::Localize(string p_path)
{
	vector<string> splitPathResult;
	string a_exp_task = "", a_task = "";
	stringstream filePath;
	clock_t t2;

	splitPathResult = split<string>(p_path, "\\");
	a_exp_task = splitPathResult[splitPathResult.size() - 1];

	splitPathResult = split<string>(a_exp_task, "_");
	a_task = splitPathResult[splitPathResult.size() - 1];

	filePath << p_path << "\\" << a_exp_task << ".TRC";

	// 1) Faire l'analyse du TRC pour r�cup�rer les data
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

	//cr�er les bipoles
	InsermLibrary::ELAN elan(trc, 2);
	elan.DisplayChoiceKeepElectrode();
	elan.eeg_loc_montage(elan.trc->nameElectrodePositiv, elan.trc->signalPosition);

	if (a_task == "VISU")
	{
		InsermLibrary::PROV *p_provVISU = new InsermLibrary::PROV("D:\\Users\\Florian\\Documents\\Arbeit\\INSERM\\Temp Files\\Localizer\\VISU.prov");

		t2 = clock();
		LocaVISU(&elan, p_provVISU, p_path, a_exp_task, a_task);
		t2 = clock() - t2;
		printf("Localize VISU : It took me %d clicks (%f seconds).\n", t2, ((float)t2) / CLOCKS_PER_SEC);
		std::cout << endl;
	}
	else if (a_task == "LEC1")
	{
		InsermLibrary::PROV *p_provLEC1 = new InsermLibrary::PROV("D:\\Users\\Florian\\Documents\\Arbeit\\INSERM\\Temp Files\\Localizer\\LEC1.prov");

		t2 = clock();
		LocaLEC1(&elan, p_provLEC1, p_path, a_exp_task, a_task);
		t2 = clock() - t2;
		printf("Localize LEC1 : It took me %d clicks (%f seconds).\n", t2, ((float)t2) / CLOCKS_PER_SEC);
		std::cout << endl;
	}
	else if (a_task == "MCSE")
	{
		InsermLibrary::PROV *p_provMCSE = new InsermLibrary::PROV("D:\\Users\\Florian\\Documents\\Arbeit\\INSERM\\Temp Files\\Localizer\\MCSE.prov");

		t2 = clock();
		LocaMCSE(&elan, p_provMCSE, p_path, a_exp_task, a_task);
		t2 = clock() - t2;
		printf("Localize MCSE : It took me %d clicks (%f seconds).\n", t2, ((float)t2) / CLOCKS_PER_SEC);
		std::cout << endl;
	}
	/*Delete Memory stuff*/
	//delete elan;

}
/**********************************************************************************************************************************************************************************************************************************************/

void InsermLibrary::LOCA::LocaVISU(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, string p_path, string p_exp_task, string p_task)
{
	stringstream posFilePath, confFilePath, posXFilePath, pictureLabel, folderTrialsSM, eeg2envFilePath;
	string pathFreq;
	double frequencyBetta[5]{8, 12, 16, 20, 24};
	double frequencyGamma[11]{ 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 };
	int *code = new int[11]{10, 20, 30, 40, 50, 60, 70, 90, 110, 100, 80};																		   
	string *strCode = new string[11]{"Maison", "Visage", "Animal", "Scene", "Objet", "Pseudo", "Conson", "Scrambled", "Bfix", "Pause", "Fruits"};  
	int *window_ms = new int[2]{-500, 1000};																									   

	/*********************************************************************************************************/
	/*										Create .pos and dsX.pos File									 */
	/*********************************************************************************************************/
	posFilePath << p_path << "/" << p_exp_task << ".pos";													 //
	posXFilePath << p_path << "/" << p_exp_task << "_ds" << (p_elan->trc->samplingFrequency / 64) << ".pos"; //
	loc_create_pos(posFilePath.str(), posXFilePath.str(), p_elan->trc, 99, p_prov);							 //
	/*********************************************************************************************************/

	/*****************************************************************************************************/
	/*											Create .conf File										 */
	/*****************************************************************************************************/
	confFilePath << p_path << "/" << p_exp_task << ".conf";											 //
	loc2_write_conf(confFilePath.str(), p_elan->trc, p_elan);											 //
	/*****************************************************************************************************/

	/******************************************************************************************************************************/
	/*														Loc eeg2erp														      */
	/******************************************************************************************************************************/
	//loc_eeg2erp(p_elan, p_path, p_exp_task, code, 11, strCode, 11, window_ms, 20);	  										      //
	/******************************************************************************************************************************/

	eeg2envFilePath << p_path << "/" << p_exp_task << ".TRC";
	/******************************************************************************************************************************/
	/*									Gamma Analysis 50Hz -> 150 Hz							 								  */
	/******************************************************************************************************************************/
	/*****************************************************************************************************/						  //
	/*									EEG2ENV	50Hz -> 150Hz											 */						  //
	/*****************************************************************************************************/						  //
	//p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[0],frequencyGamma, 11,ELAN_HISTO); //						  //
	/*****************************************************************************************************/					      //
																																  //
	pictureLabel << p_exp_task << "_f50f150_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";						  //
	folderTrialsSM << p_path << "/" << p_exp_task << "_f50f150_trials";														  //
																																  //
	if (!QDir(&folderTrialsSM.str()[0]).exists())																				  //
	{																															  //
		std::cout << "Creating Output Folder for 50-150 Hz data " << endl;														  //
		QDir().mkdir(&folderTrialsSM.str()[0]);																					  //
	}																															  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loc_env2plot											 */						  //
	/*****************************************************************************************************/						  //
	pathFreq = "";																						 //						  //
	pathFreq.append(p_exp_task);																		 //						  //
	pathFreq.append("_f50f150_ds8_sm0");																 //						  //
	//loc_env2plot(p_elan, 0, p_path, pathFreq, code, 11, strCode, 11, window_ms, 20);					 //						  //
	/*****************************************************************************************************/						  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loca trialmat											 */						  //
	/*****************************************************************************************************/						  //
	ef_read_elan_file((char*)"D:\\Users\\Florian\\Documents\\Arbeit\\2) Loca Patient\\2014\\LYONNEURO_2014_RENT\\LYONNEURO_2014_RENT_VISU\\LYONNEURO_2014_RENT_VISU_f50f150_ds8_sm0.eeg", p_elan->elanFreqBand[0]);
	loca_trialmat(p_elan, 0, p_prov, pictureLabel.str(), folderTrialsSM.str());							 //					      //
	/*****************************************************************************************************/						  //
	/******************************************************************************************************************************/

	/******************************************************************************************************************************/
	/*										Beta Analysis 8Hz -> 24 Hz							 								  */
	/******************************************************************************************************************************/
	/*****************************************************************************************************/						  //
	/*										EEG2ENV	8Hz -> 24 Hz										 */						  //
	/*****************************************************************************************************/						  //
	p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[1], frequencyBetta, 5, ELAN_HISTO);//						  //
	/*****************************************************************************************************/					      //
	stringstream().swap(pictureLabel);																							  //
	pictureLabel << p_exp_task << "_f8f24_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";						  //
	stringstream().swap(folderTrialsSM);																						  //
	folderTrialsSM << p_path << "\\" << p_exp_task << "_f8f24_trials";															  //
																																  //
	if (!QDir(&folderTrialsSM.str()[0]).exists())																				  //
	{																															  //
		std::cout << "Creating Output Folder for 8-24 Hz data " << endl;														  //
		QDir().mkdir(&folderTrialsSM.str()[0]);																					  //
	}																															  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loc_env2plot											 */						  //
	/*****************************************************************************************************/						  //
	pathFreq = "";																						 //						  //
	pathFreq.append(p_exp_task);																		 //						  //
	pathFreq.append("_f8f24_ds8_sm0");																	 //						  //
	loc_env2plot(p_elan, 1, p_path, pathFreq, code, 11, strCode, 11, window_ms, 20);				     //						  //
	/*****************************************************************************************************/						  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loca trialmat											 */						  //
	/*****************************************************************************************************/						  //
	loca_trialmat(p_elan, 1, p_prov, pictureLabel.str(), folderTrialsSM.str());							 //					      //
	/*****************************************************************************************************/						  //
	/******************************************************************************************************************************/
}

void InsermLibrary::LOCA::LocaLEC1(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, string p_path, string p_exp_task, string p_task)
{
	stringstream posFilePath, confFilePath, posXFilePath, pictureLabel, folderTrialsSM, eeg2envFilePath;
	string pathFreq;
	double frequencyBetta[5]{8, 12, 16, 20, 24};
	double frequencyGamma[11]{ 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 };
	int *code = new int[6]{10,20,30,1,2,100};
	string *strCode = new string[6]{"Seman","Phono","Visual","RepY","RepN","Pause"};
	int *window_ms = new int[2]{-1000, 3000};

	/*********************************************************************************************************/
	/*										Create .pos and dsX.pos File									 */
	/*********************************************************************************************************/
	posFilePath << p_path << "\\" << p_exp_task << ".pos";													 //
	posXFilePath << p_path << "\\" << p_exp_task << "_ds" << (p_elan->trc->samplingFrequency / 64) << ".pos";//
	loc_create_pos(posFilePath.str(), posXFilePath.str(), p_elan->trc, 99, p_prov);							 //
	/*********************************************************************************************************/

	/*****************************************************************************************************/
	/*											Create .conf File										 */
	/*****************************************************************************************************/
	confFilePath << p_path << "\\" << p_exp_task << ".conf";											 //
	loc2_write_conf(confFilePath.str(), p_elan->trc, p_elan);											 //
	/*****************************************************************************************************/

	/******************************************************************************************************************************/
	/*														Loc eeg2erp														      */
	/******************************************************************************************************************************/
	loc_eeg2erp(p_elan, p_path, p_exp_task, code, 6, strCode, 6, window_ms, 20);											      //
	/******************************************************************************************************************************/

	eeg2envFilePath << p_path << "\\" << p_exp_task << ".TRC";
	/******************************************************************************************************************************/
	/*									Gamma Analysis 50Hz -> 150 Hz							 								  */
	/******************************************************************************************************************************/
	/*****************************************************************************************************/						  //
	/*									EEG2ENV	50Hz -> 150Hz											 */						  //
	/*****************************************************************************************************/						  //
	p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[0],frequencyGamma, 11,ELAN_HISTO); //						  //
	/*****************************************************************************************************/					      //
																																  //
	pictureLabel << p_exp_task << "_f50f150_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";						  //
	folderTrialsSM << p_path << "\\" << p_exp_task << "_f50f150_trials";														  //
																																  //
	if (!QDir(&folderTrialsSM.str()[0]).exists())																				  //
	{																															  //
		std::cout << "Creating Output Folder for 50-150 Hz data " << endl;														  //
		QDir().mkdir(&folderTrialsSM.str()[0]);																					  //
	}																															  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loc_env2plot											 */						  //
	/*****************************************************************************************************/						  //
	pathFreq = "";																						 //						  //
	pathFreq.append(p_exp_task);																		 //						  //
	pathFreq.append("_f50f150_ds8_sm0");																 //						  //
	loc_env2plot(p_elan, 0, p_path, pathFreq, code, 6, strCode, 6, window_ms, 20);						 //						  //
	/*****************************************************************************************************/						  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loca trialmat											 */						  //
	/*****************************************************************************************************/						  //
	loca_trialmat(p_elan, 0, p_prov, pictureLabel.str(), folderTrialsSM.str());							 //					      //
	/*****************************************************************************************************/						  //
	/******************************************************************************************************************************/

	/******************************************************************************************************************************/
	/*									Beta Analysis 8Hz -> 24 Hz								 								  */
	/******************************************************************************************************************************/
	/*****************************************************************************************************/						  //
	/*									EEG2ENV	8Hz -> 24 Hz											 */						  //
	/*****************************************************************************************************/						  //
	p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[1], frequencyBetta, 5, ELAN_HISTO);//						  //
	/*****************************************************************************************************/					      //
																																  //
	pictureLabel << p_exp_task << "_f8f24_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";						  //
	folderTrialsSM << p_path << "\\" << p_exp_task << "_f8f24_trials";															  //
																																  //
	if (!QDir(&folderTrialsSM.str()[0]).exists())																				  //
	{																															  //
		std::cout << "Creating Output Folder for 8-24 Hz data " << endl;														  //
		QDir().mkdir(&folderTrialsSM.str()[0]);																					  //
	}																															  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loc_env2plot											 */						  //
	/*****************************************************************************************************/						  //
	pathFreq = "";																						 //						  //
	pathFreq.append(p_exp_task);																		 //						  //
	pathFreq.append("_f8f24_ds8_sm0");																	 //						  //
	loc_env2plot(p_elan, 1, p_path, pathFreq, code, 6, strCode, 6, window_ms, 20);						 //						  //
	/*****************************************************************************************************/						  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loca trialmat											 */						  //
	/*****************************************************************************************************/						  //
	loca_trialmat(p_elan, 1, p_prov, pictureLabel.str(), folderTrialsSM.str());							 //					      //
	/*****************************************************************************************************/						  //
	/******************************************************************************************************************************/
}

void InsermLibrary::LOCA::LocaMCSE(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, std::string p_path, std::string p_exp_task, std::string p_task)
{
	stringstream posFilePath, confFilePath, posXFilePath, pictureLabel, folderTrialsSM, eeg2envFilePath;
	string pathFreq;
	double frequencyBetta[5]{8, 12, 16, 20, 24};
	double frequencyGamma[11]{ 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 };
	int *code = new int[7]{10, 60, 110, 160, 101};
	string *strCode = new string[7]{"FACILE", "DIFFICILE", "FAC REP", "DIF REP", "PAUSE YO"};
	int *window_ms = new int[2]{-500, 2500};

	/*********************************************************************************************************/
	/*										Create .pos and dsX.pos File									 */
	/*********************************************************************************************************/
	posFilePath << p_path << "\\" << p_exp_task << ".pos";													 //
	posXFilePath << p_path << "\\" << p_exp_task << "_ds" << (p_elan->trc->samplingFrequency / 64) << ".pos";//
	loc_create_pos(posFilePath.str(), posXFilePath.str(), p_elan->trc, 99, p_prov);							 //
	/*********************************************************************************************************/

	/*****************************************************************************************************/
	/*											Create .conf File										 */
	/*****************************************************************************************************/
	confFilePath << p_path << "\\" << p_exp_task << ".conf";											 //
	loc2_write_conf(confFilePath.str(), p_elan->trc, p_elan);											 //
	/*****************************************************************************************************/

	/******************************************************************************************************************************/
	/*														Loc eeg2erp														      */
	/******************************************************************************************************************************/
	//loc_eeg2erp(p_elan, p_path, p_exp_task, code, 5, strCode, 5, window_ms, 20);											      //
	/******************************************************************************************************************************/

	eeg2envFilePath << p_path << "\\" << p_exp_task << ".TRC";
	/******************************************************************************************************************************/
	/*									Gamma Analysis 50Hz -> 150 Hz							 								  */
	/******************************************************************************************************************************/
	/*****************************************************************************************************/						  //
	/*									EEG2ENV	50Hz -> 150Hz											 */						  //
	/*****************************************************************************************************/						  //
	//p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[0], frequencyGamma,11, ELAN_HISTO);//						  //
	/*****************************************************************************************************/					      //
																																  //
	pictureLabel << p_exp_task << "_f50f150_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";						  //
	folderTrialsSM << p_path << "\\" << p_exp_task << "_f50f150_trials";														  //
																																  //
	if (!QDir(&folderTrialsSM.str()[0]).exists())																				  //
	{																															  //
		std::cout << "Creating Output Folder for 50-150 Hz data " << endl;														  //
		QDir().mkdir(&folderTrialsSM.str()[0]);																					  //
	}																															  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loc_env2plot											 */						  //
	/*****************************************************************************************************/						  //
	pathFreq = "";																						 //						  //
	pathFreq.append(p_exp_task);																		 //						  //
	pathFreq.append("_f50f150_ds8_sm0");																 //						  //
	//loc_env2plot(p_elan, 0, p_path, pathFreq, code, 5, strCode, 5, window_ms, 20);						 //						  //
	/*****************************************************************************************************/						  //
																																  //
	/*****************************************************************************************************/						  //
	/*											loca trialmat											 */						  //
	/*****************************************************************************************************/						  //
	ef_read_elan_file((char*)"D:\\Users\\Florian\\Documents\\Arbeit\\2) Loca Patient\\2015\\LYONNEURO_2015_BOUc1\\LYONNEURO_2015_BOUc1_MCSE\\LYONNEURO_2015_BOUc1_MCSE_f50f150_ds32_sm0.eeg", p_elan->elanFreqBand[0]);
	loca_trialmat(p_elan, 0, p_prov, pictureLabel.str(), folderTrialsSM.str());							 //					      //
	/*****************************************************************************************************/						  //
	/******************************************************************************************************************************/
}

/**********************************************************************************************************************************************************************************************************************************************/
/*															CREATE POS AND CONF : ELAN COMPATIBILITY																																		  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::loc_create_pos(string posFile_path, string posXFile_path, MicromedLibrary::TRC *p_trc, int p_beginningCode, InsermLibrary::PROV *p_prov)
{
	int beginValue = 0;

	ofstream fichierPos(posFile_path, ios::out);  // patient.pos with data for samplingFrequency										  
	ofstream fichierPosX(posXFile_path, ios::out);  // patient_dsX.pos with data for samplingFrequency downsampled to 64 sample per sec									  

	//start .pos after last beginningCode that indicates beginning of expe
	vector<int> indexBegin = findNum((int*)&p_trc->valueTrigg[0], p_trc->sampleTrigg.size() - 1, p_beginningCode);
	beginValue = indexBegin[indexBegin.size() - 1] + 1;

	//create two pointer and class containing trigger info for further use
	triggTRC = new TRIGGINFO(&p_trc->valueTrigg[beginValue], &p_trc->sampleTrigg[beginValue], (int)(p_trc->sampleTrigg.size() - beginValue), 1);
	triggDownTRC = new TRIGGINFO(&p_trc->valueTrigg[beginValue], &p_trc->sampleTrigg[beginValue], (int)(p_trc->sampleTrigg.size() - beginValue), (int)(p_trc->samplingFrequency / 64));

	if (p_prov->changeCodeFilePath != "")
	{
		renameTrigger(&triggTRC[0], &triggDownTRC[0], p_prov);
	}

	//write output file
	for (int i = 0; i < (p_trc->sampleTrigg.size() - beginValue); i++)
	{
		fichierPos << triggTRC->trigg[i]->sampleTrigger << setw(10) << triggTRC->trigg[i]->valueTrigger << setw(10) << "0" << endl;
		fichierPosX << triggDownTRC->trigg[i]->sampleTrigger << setw(10) << triggDownTRC->trigg[i]->valueTrigger << setw(10) << "0" << endl;
	}

	fichierPos.close();
	fichierPosX.close();
}

void InsermLibrary::LOCA::renameTrigger(TRIGGINFO *triggers, TRIGGINFO* downsampledTriggers, InsermLibrary::PROV *p_prov)
{
	int indexVisuBloc = 0, winMax = 0, winMin = 0;
	vector<int> oldMainCode, oldSecondaryCode, newMainCode, newSecondaryCode;
	std::stringstream buffer;
	std::ifstream provFile(p_prov->changeCodeFilePath, std::ios::binary);																						   								                                              //
	if (provFile)																																																						      //
	{																																																									      //
		// Copy all file in buffer																																																		      //
		buffer << provFile.rdbuf();																																																		      //
		// Don't need the file anymore ! 																																																      //
		provFile.close();																																																				      //

		std::vector<std::string> lineSplit = split<std::string>(buffer.str(), "\r\n");
		for (int i = 0; i < lineSplit.size(); i++)
		{
			std::vector<std::string> elementSplit = split<std::string>(lineSplit[i], "+=");
			oldMainCode.push_back(atoi(&(elementSplit[0])[0]));
			oldSecondaryCode.push_back(atoi(&(elementSplit[1])[0]));
			newMainCode.push_back(atoi(&(elementSplit[2])[0]));
			newSecondaryCode.push_back(atoi(&(elementSplit[3])[0]));
		}

		for (int i = 0; i < oldMainCode.size(); i++)
		{
			for (int j = 0; j < p_prov->numberVisuBlocs; j++)
			{
				if (p_prov->visuBlocs[j]->mainEvent->mainEventCode == oldMainCode[i])
				{
					indexVisuBloc = j;
				}
			}

			int winSamMin = round((64 * p_prov->visuBlocs[indexVisuBloc]->dispBloc->epochWindow[0]) / 1000);
			int winSamMax = round((64 * p_prov->visuBlocs[indexVisuBloc]->dispBloc->epochWindow[1]) / 1000);

			int count = 0;
			for (int j = 0; j < downsampledTriggers->numberTrigg; j++)
			{
				if (downsampledTriggers->trigg[j]->valueTrigger == oldMainCode[i])
				{
					count = j + 1;
					winMax = downsampledTriggers->trigg[j]->sampleTrigger + winSamMax;
					winMin = downsampledTriggers->trigg[j]->sampleTrigger + winSamMin;
					while ((downsampledTriggers->trigg[count]->sampleTrigger < winMax) && (downsampledTriggers->trigg[count]->sampleTrigger > winMin))
					{
						if (downsampledTriggers->trigg[count]->valueTrigger == oldSecondaryCode[i])
						{
							//cout << triggers->trigg[j]->valueTrigger << "to " << newMainCode[i] << endl;
							//cout << triggers->trigg[count]->valueTrigger << "to " << newSecondaryCode[i] << endl;
							//cout << endl;

							triggers->trigg[j]->valueTrigger = newMainCode[i];
							triggers->trigg[count]->valueTrigger = newSecondaryCode[i];
							//==========================================================================
							downsampledTriggers->trigg[j]->valueTrigger = newMainCode[i];
							downsampledTriggers->trigg[count]->valueTrigger = newSecondaryCode[i];
							//break;

						}
						count++;
					}
				}
			}
		}
	}																																																									      //
	else																																																								      //
	{ 																																																									      //
		std::cout << " Error opening Change Code File @ " << p_prov->changeCodeFilePath.c_str() << std::endl;																															      //
	}																																																									      //
}

void InsermLibrary::LOCA::loc2_write_conf(string confFile_path, MicromedLibrary::TRC *p_trc, InsermLibrary::ELAN *p_elan)
{
	//[=================================================================================================================================]										  
	ofstream fichierConf(confFile_path, ios::out);  // ouverture en �criture avec effacement du fichier ouvert	

	fichierConf << "nb_channel" << "  " << p_trc->numberStoredChannels << endl;
	fichierConf << "sec_per_page" << "  " << 4 << endl;
	fichierConf << "amp_scale_type" << "  " << 1 << endl;
	fichierConf << "amp_scale_val" << "  " << 1 << endl;

	fichierConf << "channel_visibility" << endl;
	for (int i = 0; i < p_trc->numberStoredChannels; i++)
	{
		fichierConf << 1 << endl;
	}

	/***********************************************************************************************************/
	/* On affiche l'index du bipole tant que le suivant fait partie de la m�me �l�ctrode : A'1 , A'2, A'3 ...  */
	/* puis quand le suivant n'est pas de la m�me �l�ctrode ( B'1 ) on met -1 et c'est repartir pour un tour   */
	/***********************************************************************************************************/
	fichierConf << "channel_reference" << endl;
	fichierConf << "-1" << endl;

	for (int i = 0; i <= p_trc->numberStoredChannels; i++)
	{
		if (i <= p_elan->m_bipole_moins.size() - 1)
		{
			if (i == p_elan->m_bipole_moins.size() - 1)
			{
				fichierConf << p_elan->m_bipole_moins[i] << endl;
			}
			else if (p_elan->m_bipole_moins[i] + 1 != p_elan->m_bipole_moins[i + 1])
			{
				//-1
				fichierConf << p_elan->m_bipole_moins[i] << endl;
				fichierConf << "-1" << endl;
			}
			else
			{
				fichierConf << p_elan->m_bipole_moins[i] << endl;
			}
		}
		else
		{
			fichierConf << "-1" << endl;
		}
	}
	//[=================================================================================================================================]										  

}
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																										EEG2ERP																											 				      */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::loc_eeg2erp(InsermLibrary::ELAN *p_elan, string p_path, string p_exp_task, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int* v_window_ms, int nb_site)										  //
{																																																											  //
	string contents;																																																						  //																																																						  //
	int  nbEventUsed = 0, beginPos = 0;																																																		  //
	int v_win_sam[2];																																																						  //
	vector<int> indexEventUsed, eventUsed;																																																  	  //
	double ***m_bigdata_mono, ***m_bigdata_bipo;																																															  //
																																																											  //
	v_win_sam[0] = round(p_elan->trc->samplingFrequency*v_window_ms[0] / 1000);																																								  //
	v_win_sam[1] = round(p_elan->trc->samplingFrequency*v_window_ms[1] / 1000);																																								  //
																																																											  //
	/*******************************************************************************************************************************************************************************************/											  //
	/*												On garde uniquement les �vennements du TRC qui correspondent � ceux passer par v_code et a_code					   						   */											  //
	/*******************************************************************************************************************************************************************************************/											  //
	for (int i = 0; i < triggTRC->numberTrigg /*p_elan->trc->sampleTrigg.size() - 1*/; i++)																									   //											  //
	{																																														   //											  //
		for (int j = 0; j < v_codeLength; j++)																																				   //											  //
		{																																													   //											  //
			if (triggTRC->trigg[i]->valueTrigger/*p_elan->trc->valueTrigg[i]*/ == v_code[j])																								   //											  //
			{																																												   //											  //
				indexEventUsed.push_back(i);																																				   //											  //
				eventUsed.push_back(triggTRC->trigg[i]->valueTrigger/*p_elan->trc->valueTrigg[i]*/);																						   //											  //
			}																																												   //											  //
		}																																													   //											  //
	}																																														   //											  //
																																															   //											  //
	nbEventUsed = indexEventUsed.size();																																					   //											  //
	/*******************************************************************************************************************************************************************************************/											  //
																																																											  //
	/****************************************** Allocate Memory *******************************************/																																  //
	m_bigdata_mono = new double**[nbEventUsed];														  /*||*/																																  //
	for (int i = 0; i < nbEventUsed; i++)													 		  /*||*/																																  //
	{																								  /*||*/																																  //
		m_bigdata_mono[i] = new double*[p_elan->trc->nameElectrodePositiv.size()];					  /*||*/																																  //
		for (int j = 0; j < p_elan->trc->nameElectrodePositiv.size(); j++)							  /*||*/																																  //
		{																							  /*||*/																																  //
			m_bigdata_mono[i][j] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();					  /*||*/																																  //
		}																							  /*||*/																																  //
	}																								  /*||*/																																  //
																									  /*||*/																																  //
	m_bigdata_bipo = new double**[nbEventUsed];														  /*||*/																																  //
	for (int i = 0; i < nbEventUsed; i++)															  /*||*/																																  //
	{																								  /*||*/																																  //
		m_bigdata_bipo[i] = new double*[p_elan->m_bipole.size()];									  /*||*/																																  //
		for (int j = 0; j < p_elan->m_bipole.size(); j++)											  /*||*/																																  //
		{																							  /*||*/																																  //
			m_bigdata_bipo[i][j] = new double[(v_win_sam[1] - v_win_sam[0]) + 1];					  /*||*/																																  //
			for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]) + 1; k++)								  /*||*/																																  //
			{																						  /*||*/																																  //
				m_bigdata_bipo[i][j][k] = 0;														  /*||*/																																  //
			}																						  /*||*/																																  //
		}																							  /*||*/																																  //
	}																								  /*||*/																																  //
	/******************************************************************************************************/																																  //
																																																											  //
	/*******************************************************************************************************************************************************************************************/											  //
	/*														Extract EEG DATA within w_window_ms around event																				   */											  //
	/*******************************************************************************************************************************************************************************************/											  //
	/*********************************************************************************************************************/																	   //											  //
	/*												Read Mono Data														 */																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
	std::cout << "Reading Data Mono ..." << endl;																		 //																	   //											  //
	for (int i = 0; i < nbEventUsed; i++)																				 //																	   //											  //
	{																													 //																	   //											  //
		for (int j = 0; j < (v_win_sam[1] - v_win_sam[0]) + 1; j++)														 //																	   //											  //
		{																												 //																	   //											  //
			for (int k = 0; k < p_elan->trc->nameElectrodePositiv.size(); k++)											 //																	   //											  //
			{																											 //																	   //											  //
				beginPos = triggTRC->trigg[indexEventUsed[i]]->sampleTrigger + v_win_sam[0]; //p_elan->trc->sampleTrigg[indexEventUsed[i]] + v_win_sam[0];									 //																	   //											  //
				m_bigdata_mono[i][k][j] = p_elan->trc->eegData[k][beginPos + j];										 //																	   //											  //
			}																											 //																	   //											  //
		}																												 //																	   //											  //
	}																													 //																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
																																															   //											  //
	/*********************************************************************************************************************/																	   //											  //
	/*												Read Bipo Data														 */																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
	std::cout << "Reading Data Bipo ..." << endl;																		 //																	   //											  //
	for (int i = 0; i < nbEventUsed; i++)																				 //																	   //											  //
	{																													 //																	   //											  //
		for (int k = 0; k < p_elan->m_bipole.size(); k++)																 //																	   //											  //
		{																												 //																	   //											  //
			for (int j = 0; j < (v_win_sam[1] - v_win_sam[0]) + 1; j++)													 //																	   //											  //
			{																											 //																	   //											  //
				m_bigdata_bipo[i][k][j] = m_bigdata_mono[i][p_elan->m_bipole[k]][j] - m_bigdata_mono[i][p_elan->m_bipole_moins[k]][j];														   //											  //
			}																											 //																	   //											  //
		}																												 //																	   //											  //
	}																													 //																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
	/*******************************************************************************************************************************************************************************************/											  //
																																																											  //
	//0 for mono, 1 for bipo , 2 for env2plot
	//mono cards 
	drawCards(p_elan, p_path, p_exp_task, 0, m_bigdata_mono, v_code, v_codeLength, a_code, a_codeLength, v_win_sam, nb_site, indexEventUsed, eventUsed);

	//bipo cards
	drawCards(p_elan, p_path, p_exp_task, 1, m_bigdata_bipo, v_code, v_codeLength, a_code, a_codeLength, v_win_sam, nb_site, indexEventUsed, eventUsed);

	/****************************************** Free Memory *******************************************/																																	  //		
	for (int i = 0; i < nbEventUsed; i++)														  /*||*/																																	  //		
	{																							  /*||*/																																	  //		
		for (int j = 0; j < p_elan->trc->nameElectrodePositiv.size(); j++)						  /*||*/																																	  //		
		{																						  /*||*/																																	  //		
			delete[] m_bigdata_mono[i][j];														  /*||*/																																	  //		
		}																						  /*||*/																																	  //		
		delete[] m_bigdata_mono[i];																  /*||*/																																	  //		
	}																							  /*||*/																																	  //		
	delete[] m_bigdata_mono;																	  /*||*/																																	  //		
	for (int i = 0; i < nbEventUsed; i++)														  /*||*/																																	  //		
	{																							  /*||*/																																	  //		
		for (int j = 0; j < p_elan->m_bipole.size(); j++)										  /*||*/																																	  //		
		{																						  /*||*/																																	  //		
			delete[] m_bigdata_bipo[i][j];														  /*||*/																																	  //		
		}																						  /*||*/																																	  //		
		delete[] m_bigdata_bipo[i];																  /*||*/																																	  //		
	}																							  /*||*/																																	  //		
	delete[] m_bigdata_bipo;																	  /*||*/																																	  //		
	/**************************************************************************************************/																																	  //		
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																										ENV2PLOT																										 					  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::loc_env2plot(InsermLibrary::ELAN *p_elan, int p_numberFrequencyBand, std::string p_path, std::string p_exp_task, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int* v_window_ms, int nb_site)//
{																																																											  //
	string contents;																																																						  //																																																						  //
	int  nbEventUsed = 0, beginInd = 0;																																																		  //
	int v_win_sam[2];																																																						  //
	vector<int> indexEventUsed, eventUsed;																																																  	  //
	double ***m_bigdata_frequency;																																																			  //
	//
	v_win_sam[0] = round(64 * v_window_ms[0] / 1000);																																															  //
	v_win_sam[1] = round(64 * v_window_ms[1] / 1000);																																															  //
	//
	/*******************************************************************************************************************************************************************************************/											  //
	/*												On garde uniquement les �vennements du TRC qui correspondent � ceux passer par v_code et a_code					   						   */											  //
	/*******************************************************************************************************************************************************************************************/											  //
	for (int i = 0; i < triggTRC->numberTrigg /*p_elan->trc->sampleTrigg.size() - 1*/; i++)																															   //											  //
	{																																														   //											  //
		for (int j = 0; j < v_codeLength; j++)																																				   //											  //
		{																																													   //											  //
			if (triggTRC->trigg[i]->valueTrigger/*p_elan->trc->valueTrigg[i]*/ == v_code[j])																								   //											  //
			{																																												   //											  //
				indexEventUsed.push_back(i);																																				   //											  //
				eventUsed.push_back(triggTRC->trigg[i]->valueTrigger/*p_elan->trc->valueTrigg[i]*/);																						   //											  //
			}																																												   //											  //
		}																																													   //											  //
	}																																														   //											  //
																																															   //											  //
	nbEventUsed = indexEventUsed.size();																																					   //											  //
	/*******************************************************************************************************************************************************************************************/											  //
	//
	/****************************************** Allocate Memory *******************************************/																																  //
	m_bigdata_frequency = new double**[nbEventUsed];												  /*||*/																																  //
	for (int i = 0; i < nbEventUsed; i++)															  /*||*/																																  //
	{																								  /*||*/																																  //
		m_bigdata_frequency[i] = new double*[p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb];	  /*||*/																																  //
		for (int j = 0; j < p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; j++)				  /*||*/																																  //
		{																							  /*||*/																																  //
			m_bigdata_frequency[i][j] = new double[(v_win_sam[1] - v_win_sam[0]) + 1];			  	  /*||*/																																  //
			for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]) + 1; k++)							 	  /*||*/																																  //
			{																						  /*||*/																																  //
				m_bigdata_frequency[i][j][k] = 0;													  /*||*/																																  //
			}																						  /*||*/																																  //
		}																							  /*||*/																																  //
	}																								  /*||*/																																  //
	/******************************************************************************************************/																																  //
	//
	/*******************************************************************************************************************************************************************************************/											  //
	/*														Extract EEG DATA within w_window_ms around event																				   */											  //
	/*******************************************************************************************************************************************************************************************/											  //
	/*********************************************************************************************************************/																	   //											  //
	/*												Read Mono Data														 */																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
	std::cout << "Reading Hilberted Bipo ..." << endl;																	 //																	   //											  //
	for (int i = 0; i < nbEventUsed; i++)																				 //																	   //											  //
	{																													 //																	   //											  //
		beginInd = triggTRC->trigg[indexEventUsed[i]]->sampleTrigger + v_win_sam[0]; // p_elan->trc->sampleTrigg[indexEventUsed[i]] + v_win_sam[0];											 //																	   //											  //
		for (int j = 0; j < (v_win_sam[1] - v_win_sam[0]) + 1; j++)														 //																	   //											  //
		{																												 //																	   //											  //
			for (int k = 0; k < p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; k++)								 //																	   //											  //
			{																											 //																	   //											  //
				m_bigdata_frequency[i][k][j] = p_elan->elanFreqBand[p_numberFrequencyBand]->eeg.data_double[0][k][beginInd + j];																   //											  //
			}																											 //																	   //											  //
		}																												 //																	   //											  //
	}																													 //																	   //											  //
	/*********************************************************************************************************************/																	   //											  //
	/*******************************************************************************************************************************************************************************************/											  //

	//env2plot cards
	//drawCardsenv2plot(p_elan, p_path, p_exp_task, m_bigdata_frequency, v_code, v_codeLength, a_code, a_codeLength, v_win_sam, nb_site, indexEventUsed, eventUsed);
	drawCards(p_elan, p_path, p_exp_task, 2, m_bigdata_frequency, v_code, v_codeLength, a_code, a_codeLength, v_win_sam, nb_site, indexEventUsed, eventUsed);

	/****************************************** Free Memory *******************************************/																																	  //			
	for (int i = 0; i < nbEventUsed; i++)														  /*||*/																																	  //		
	{																							  /*||*/																																	  //		
		for (int j = 0; j < p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; j++)			  /*||*/																																	  //		
		{																						  /*||*/																																	  //		
			delete[] m_bigdata_frequency[i][j];													  /*||*/																																	  //		
		}																						  /*||*/																																	  //		
		delete[] m_bigdata_frequency[i];														  /*||*/																																	  //		
	}																							  /*||*/																																	  //		
	delete[] m_bigdata_frequency;																  /*||*/																																	  //		
	/**************************************************************************************************/																																	  //		
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																										Drawcards																										 					  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::drawCards(InsermLibrary::ELAN *p_elan, string p_path, string p_exp_task, int cards2Draw, double *** bigdata, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int v_win_sam[2], int nb_site, vector<int> indexEventUsed, vector<int> EventUsed)
{
	string a_rt;
	stringstream tifNameStream;
	QString tifName;
	int s_figure = 0, s_pos = 0, nb_elec_perfigure = 3, screenWidth = 0, screenHeigth = 0, nbcol = 0, s_x = 0, s_y = 0, compteur = 0;
	vector<int> v_id, v_origid, v_f;
	double *v_erp, *v_std, *v_sem, *v_lim, *m_erpPMax, *m_erpMMax;
	double **m_erpP, **m_erpM, **m_erp, **m_lim, **m_data_se, **data;
	double tempErp, tempStd, tempErpM, tempMax1, tempMax2, maxCurveLegend;																																													
	bool goIn = false;

	/****************************************** Allocate Memory *******************************************/																																	  //
	data = new double*[indexEventUsed.size()];														  /*||*/																																	  //	
	for (int i = 0; i < indexEventUsed.size(); i++)												 	  /*||*/																																	  //	
	{																								  /*||*/																																	  //	
		data[i] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();									  /*||*/																																	  //	
	}																							 	  /*||*/																																	  //	
																									  /*||*/																																	  //	
	v_erp = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();										  /*||*/																																	  //	
	v_std = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();									 	  /*||*/																																	  //	
	v_sem = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();										  /*||*/																																	  //	
	v_lim = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();										  /*||*/																																	  //	
																									  /*||*/																																	  //	
	m_erp = new double*[v_codeLength];																  /*||*/																																	  //	
	m_lim = new double*[v_codeLength];																  /*||*/																																	  //	
	m_erpP = new double*[v_codeLength];																  /*||*/																																	  //	
	m_erpM = new double*[v_codeLength];																  /*||*/																																	  //	
	for (int k = 0; k < v_codeLength; k++)															  /*||*/																																	  //	
	{																								  /*||*/																																	  //	
		m_erp[k] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();									  /*||*/																																	  //	
		m_lim[k] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();									  /*||*/																																	  //	
		m_erpP[k] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();								  /*||*/																																	  //	
		m_erpM[k] = new double[(v_win_sam[1] - v_win_sam[0]) + 1]();								  /*||*/																																	  //	
	}																								  /*||*/																																	  //	
																									  /*||*/																																	  //	
	m_erpPMax = new double[v_codeLength]();															  /*||*/																																	  //	
	m_erpMMax = new double[v_codeLength]();															  /*||*/																																	  //	
	/******************************************************************************************************/																																	  //

	/*******************************************************************************************************************************************************************************************/												  //
	/*																	Create Card for each group of 3 elec																				   */												  //
	/*******************************************************************************************************************************************************************************************/												  //
	for (int i = 0; i < p_elan->ss_elec->a_rt.size(); i++)																																	   //												  //
	{
		a_rt = p_elan->ss_elec->a_rt[i];
		v_id = p_elan->ss_elec->v_id[i];
		v_origid = p_elan->ss_elec->v_origid[i];

		s_pos = i % nb_elec_perfigure;
		if (s_pos == 0)
		{
			if (s_figure > 0) //On a finis une s�rie de 3, on enregistre l'image
			{
				pixMap->save(tifName, "JPG");
			}
			s_figure = s_figure + 1;

			screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
			screenHeigth = GetSystemMetrics(SM_CYFULLSCREEN);
			pixMap = new QPixmap(screenWidth, screenHeigth);
			pixMap->fill(QColor(Qt::white));
			painter = new QPainter(pixMap);
			painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
			painter->setFont(QFont("Arial", 12, 1, false));
			stringstream().swap(tifNameStream);

			switch (cards2Draw)
			{
			case 0 :
				tifNameStream << p_path << "\\" << p_exp_task << "_erp_mono_f" << s_figure << ".jpg";
				break;
			case 1 :
				tifNameStream << p_path << "\\" << p_exp_task << "_erp_bipo_f" << s_figure << ".jpg";
				break;
			case 2 :
				tifNameStream << p_path << "\\" << p_exp_task << "_plot_f" << s_figure << ".jpg";
				break;
			}

			tifName = &tifNameStream.str()[0];
		}

		// as input, I take up to three electrodes, and for each electrode and site
		// along that electrode, I want to plot a certain number of plots.So I need
		// the data for each condition, with a time - axis.
		// draw electrode, with 20 sites, equally spaced.

		nbcol = ceil((double)v_codeLength / 3);

		s_x = screenWidth * 0.0586 + ((190 * (/*nbcol*/3 + 1)) * (i % 3));
		s_y = screenHeigth * 0.0325;
		for (int j = 0; j < v_codeLength; j++)
		{
			switch (j % 3)
			{
			case 0:
				painter->setPen(QColor(0, 0, 255, 255)); //blue
				painter->drawText(s_x, s_y, a_code[j].c_str());
				break;
			case 1:
				painter->setPen(QColor(255, 0, 0, 255)); //red
				painter->drawText(s_x, s_y, a_code[j].c_str());
				break;
			case 2:
				painter->setPen(QColor(0, 255, 0, 255)); //green
				painter->drawText(s_x, s_y, a_code[j].c_str());
				break;
			default:
				painter->setPen(QColor(0, 0, 0, 255)); //black
				break;
			}
			s_y = s_y + 25;

			if ((j + 1) % 3 == 0)
			{
				s_x = s_x + (600 / nbcol);
				s_y = screenHeigth * 0.0325;
			}
		}

		//On repasse en noir
		painter->setPen(QColor(0, 0, 0, 255)); //black

		//�cris le nom de l'�l�c
		painter->drawText((screenWidth * 0.028) + 8 + ((190 * (/*nbcol*/3 + 1)) * (i % 3)), screenHeigth * 0.090, a_rt.c_str());
		/*Dessine la structure de l'�l�ctrode*/
		QRect recElec((screenWidth * 0.028) + ((190 * (/*nbcol*/3 + 1)) * (i % 3)), screenHeigth * 0.094, screenWidth * 0.0105, screenHeigth * 0.812);
		painter->drawRect(recElec);
		painter->fillRect(recElec, QColor(255, 255, 255, 128));

		for (int j = 0; j < nb_site; j++)
		{
			painter->fillRect((screenWidth * 0.028) + ((190 * (/*nbcol*/3 + 1)) * (i % 3)), (screenHeigth * 0.094) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), screenWidth * 0.0105, screenHeigth * 0.0203, QColor(0, 0, 0, 255));//QColor(R G B Alpha)

			//num�ro d'index du plot de l'�l�ctrode
			painter->drawText((screenWidth * 0.015) + ((190 * (/*nbcol*/3 + 1)) * (i % 3)), (screenHeigth * 0.107) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), QString().setNum(j + 1));

			v_f = findNum(&v_id[0], v_id.size(), j + 1);

			switch (cards2Draw)
			{
			case 0:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size());
				break;
			case 1:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size() - 1);
				break;
			case 2:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size() - 1);
				break;
			}

			if (goIn == true)
			{
				//then we can extract the data
				for (int k = 0; k < indexEventUsed.size(); k++)
				{
					for (int l = 0; l < (v_win_sam[1] - v_win_sam[0]) + 1; l++)
					{
						data[k][l] = bigdata[k][compteur][l];			
					}															
				}
				compteur++;

				//then we loop across conditions
				for (int k = 0; k < v_codeLength; k++)
				{
					v_f = findNum(&EventUsed[0], EventUsed.size(), v_code[k]);

					//Init m_data_se
					m_data_se = new double*[v_f.size()];
					for (int l = 0; l < v_f.size(); l++)
					{
						m_data_se[l] = new double[(v_win_sam[1] - v_win_sam[0]) + 1];
					}


					if (v_f.empty() == false)
					{
						for (int l = 0; l < v_f.size(); l++)
						{
							for (int m = 0; m < (v_win_sam[1] - v_win_sam[0]) + 1; m++)
							{
								m_data_se[l][m] = data[v_f[l]][m];
								m_data_se[l][m] = (m_data_se[l][m] - 1000) / 10;
							}
						}

						for (int l = 0; l < (v_win_sam[1] - v_win_sam[0]) + 1; l++)
						{
							tempErp = 0;
							tempStd = 0;
							//calculate v_erp (mean)
							for (int m = 0; m < v_f.size(); m++)
							{
								tempErp += m_data_se[m][l];
							}
							v_erp[l] = tempErp / v_f.size();

							//calculate v_std (standard deviation)
							for (int m = 0; m < v_f.size(); m++)
							{
								tempStd += (m_data_se[m][l] - v_erp[l]) * (m_data_se[m][l] - v_erp[l]);
							}

							if (v_f.size() - 1 == 0)
							{
								v_std[l] = 0;
							}
							else
							{
								v_std[l] = sqrt(tempStd / (v_f.size() - 1));
							}


							//calculate v_sem = v_std / sqrt(size(m_data_se, 2)); % ck
							v_sem[l] = v_std[l] / sqrt(v_f.size());

							//calculate v_lim = 1.96*v_sem;
							v_lim[l] = 1.96*v_sem[l];

							m_erp[k][l] = v_erp[l];
							m_lim[k][l] = v_lim[l];
						}
					}
					else
					{
						std::cout << "ATTENTION, PLEASE : NO EVENT WITH TYPE = " << v_code[k] << endl;
					}

					/*delete tab each turn*/
					for (int l = 0; l < v_f.size(); l++)
					{
						delete[] m_data_se[l];
					}
					delete[] m_data_se;
				}

				//m_erp = m_erp - mean(m_erp(1, :)); % we remove the same value to all the erp's, so that it is more or less centered on zero at the first sample
				tempErpM = 0;
				for (int l = 0; l < v_codeLength; l++)
				{
					tempErpM += m_erp[l][0];
				}
				tempErpM /= v_codeLength;

				for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]) + 1; k++)
				{
					for (int l = 0; l < v_codeLength; l++)
					{
						m_erp[l][k] -= tempErpM;
					}
				}

				//m_erp + lim et m_erp - lim
				for (int k = 0; k < v_codeLength; k++)
				{
					for (int l = 0; l < (v_win_sam[1] - v_win_sam[0]) + 1; l++)
					{
						m_erpP[k][l] = m_erp[k][l] + m_lim[k][l];
						m_erpM[k][l] = m_erp[k][l] - m_lim[k][l];
					}
				}

				for (int k = 0; k < v_codeLength; k++)
				{
					m_erpPMax[k] = *(max_element(m_erpP[k], m_erpP[k] + (v_win_sam[1] - v_win_sam[0]) + 1));
					m_erpMMax[k] = *(max_element(m_erpM[k], m_erpM[k] + (v_win_sam[1] - v_win_sam[0]) + 1));
				}

				tempMax1 = *(max_element(m_erpPMax, m_erpPMax + v_codeLength));
				tempMax2 = *(max_element(m_erpMMax, m_erpMMax + v_codeLength));

				maxCurveLegend = /*round(*/max(tempMax1, tempMax2)/*)*/;

				painter->setPen(QColor(255, 0, 255, 255)); //pink petant
				painter->drawText((screenWidth * 0.04) + ((190 * (/*nbcol*/3 + 1)) * (i % 3)), (screenHeigth * 0.107) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), QString().setNum(round(maxCurveLegend)));
				painter->setPen(QColor(0, 0, 0, 255)); //noir
				
				for (int k = 0; k < v_codeLength; k++)
				{
					QPainterPath qpath, qpathP, qpathM;
					double x, y, yP, yM;

					for (int l = 0; l < (v_win_sam[1] - v_win_sam[0]) + 1; l++)
					{
						v_erp[l] = m_erp[k][l] / maxCurveLegend;
						v_lim[l] = m_lim[k][l] / maxCurveLegend;

						/*x = position de d�part   +  prochaine �l�ctrode   +  data (prochaine) colonne*/
						x = (screenWidth * 0.0586) + ((190 * (/*nbcol*/3 + 1)) * (i % 3)) + ((600 / nbcol) * (k / 3)) + ((l *(600 / nbcol) /*(screenWidth * 0.0586)*/) / ((v_win_sam[1] - v_win_sam[0]) + 1));
						/*y = position de d�part - data + plot �l�ctrode*/
						y = (200 - (100 * (v_erp[l] / 2))) + (screenHeigth * 0.0406 * (j));
						yP = (200 - (100 * ((v_erp[l] + v_lim[l]) / 2))) + (screenHeigth * 0.0406 * (j));
						yM = (200 - (100 * ((v_erp[l] - v_lim[l]) / 2))) + (screenHeigth * 0.0406 * (j));

						if (l == 0)
						{
							qpath.moveTo(QPointF(x, y));
							qpathP.moveTo(QPointF(x, yP));
							qpathM.moveTo(QPointF(x, yM));
						}
						else
						{
							qpath.lineTo(QPointF(x, y));
							qpathP.lineTo(QPointF(x, yP));
							qpathM.lineTo(QPointF(x, yM));
						}
					}

					switch (k % 3)																																							   //											  //
					{																																										   //											  //
					case 0:																																									   //											  //
						painter->setPen(QColor(0, 0, 255, 255)); //blue																														   //											  //
						painter->drawPath(qpath);																																			   //											  //
						painter->drawPath(qpathP);																																			   //											  //
						painter->drawPath(qpathM);																																			   //											  //
						break;																																								   //											  //
					case 1:																																									   //											  //
						painter->setPen(QColor(255, 0, 0, 255)); //red																														   //											  //
						painter->drawPath(qpath);																																			   //											  //
						painter->drawPath(qpathP);																																			   //											  //
						painter->drawPath(qpathM);																																			   //											  //
						break;																																								   //											  //
					case 2:																																									   //											  //
						painter->setPen(QColor(0, 255, 0, 255)); //green																													   //											  //
						painter->drawPath(qpath);																																			   //											  //
						painter->drawPath(qpathP);																																			   //											  //
						painter->drawPath(qpathM);																																			   //											  //
						break;																																								   //											  //
					default:																																								   //											  //
						painter->setPen(QColor(0, 0, 0, 255)); //black																														   //											  //
						break;																																								   //											  //
					}																																										   //											  //
				}																																								 			   //											  //
				//on remet en noir																																							   //											  //
				painter->setPen(QColor(0, 0, 0, 255));																																		   //											  //
			}//end if (v_f.empty() == false)																																				   //											  //
		}//end for nb site 																																									   //											  //
	}//end for nb_elec																																										   //											  //
																																															   //											  //
	pixMap->save(tifName, "JPG");																																							   //											  //
	/*******************************************************************************************************************************************************************************************/											  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	for (int i = 0; i < indexEventUsed.size(); i++)												  /*||*/																																	  //		
	{																							  /*||*/																																	  //		
		delete[] data[i];																		  /*||*/																																	  //		
	}																							  /*||*/																																	  //		
	delete[] data;																				  /*||*/																																	  //		
																								  /*||*/																																	  //
	delete[] v_erp;																				  /*||*/																																	  //		
	delete[] v_std;																				  /*||*/																																	  //		
	delete[] v_sem;																				  /*||*/																																	  //		
	delete[] v_lim;																				  /*||*/																																	  //	
																								  /*||*/																																	  //
	for (int k = 0; k < v_codeLength; k++)														  /*||*/																																	  //	
	{																							  /*||*/																																	  //	
		delete[] m_erp[k];																		  /*||*/																																	  //	
		delete[] m_lim[k];																		  /*||*/																																	  //	
		delete[] m_erpP[k];																		  /*||*/																																	  //	
		delete[] m_erpM[k];																		  /*||*/																																	  //	
	}																							  /*||*/																																	  //	
	delete[] m_erp;																				  /*||*/																																	  //	
	delete[] m_lim;																				  /*||*/																																	  //	
	delete[] m_erpP;																			  /*||*/																																	  //	
	delete[] m_erpM;																			  /*||*/																																	  //	
																								  /*||*/																																	  //
	delete[] m_erpPMax;																			  /*||*/																																	  //	
	delete[] m_erpMMax;																			  /*||*/																																	  //	
	/**************************************************************************************************/																																	  //
}																																																											  //
																																																											  //
vector<int> InsermLibrary::LOCA::findNum(int *tab, int sizetab, int value2find)																																								  //
{																																																											  //
	vector<int> temp;																																																						  //
	for (int i = 0; i < sizetab; i++)																																																		  //
	{																																																										  //
		if (tab[i] == value2find)																																																			  //
		{																																																									  //
			temp.push_back(i);																																																				  //
		}																																																									  //
	}																																																										  //
																																																											  //
	return temp;																																																							  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/*****************************************************************************************************************************************************************************************************************************************************/
/*																										TRIALMAT																										 						     */
/*****************************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::LOCA::loca_trialmat(InsermLibrary::ELAN *p_elan, int p_numberFrequencyBand, InsermLibrary::PROV *p_prov, std::string p_outputMapLabel, std::string p_outputFolder)
{
	stringstream outputPicLabel;
	QPixmap *pixmapSubMatrix;
	QPainter *painterSubMatrix;
	//[======================================================================================================================================================================================]
	mainEventBLOC **stimEvents = new mainEventBLOC*[p_prov->numberVisuBlocs];
	secondaryEventBLOC ***responseEvents = new secondaryEventBLOC**[p_prov->numberVisuBlocs];
	int * sizeResp = new int[p_prov->numberVisuBlocs];

	int numberCol = p_prov->getNumberCol();
	int numberRow = p_prov->getNumberRow();

	for (int z = 0; z < numberCol; z++)
	{
		for (int y = 0; y < numberRow; y++)
		{
			for (int i = 0; i < p_prov->numberVisuBlocs; i++)
			{
				if ((p_prov->visuBlocs[i]->dispBloc->col == z + 1) && (p_prov->visuBlocs[i]->dispBloc->row == y + 1))
				{
					stimEvents[i] = new mainEventBLOC(p_prov->visuBlocs[i]->mainEvent->mainEventCode, p_prov->visuBlocs[i]->mainEvent->mainEventLabel);

					responseEvents[i] = new secondaryEventBLOC*[p_prov->visuBlocs[i]->numberSecondaryEvents];
					sizeResp[i] = p_prov->visuBlocs[i]->numberSecondaryEvents;

					for (int j = 0; j < p_prov->visuBlocs[i]->numberSecondaryEvents; j++)
					{
						responseEvents[i][j] = new secondaryEventBLOC(p_prov->visuBlocs[i]->secondaryEvents[j]->secondaryEventCode, p_prov->visuBlocs[i]->secondaryEvents[j]->sizeSecondaryEvent, p_prov->visuBlocs[i]->secondaryEvents[j]->secondaryEventLabel);
					}
				}
			}
		}
	}

	cat2ellaRTTrigg(stimEvents, p_prov->numberVisuBlocs, responseEvents, sizeResp);					//	Extract triggers 
	std::vector<int> correspondingEvent = sortTrials(p_prov, stimEvents, numberCol, numberRow);		//	Get the index corresponding to trials sorted according to input	

	//[======================================================================================================================================================================================]
	//We calculate the biggest window there will be so that we extract the block before analysis
	int v_window_ms[2], v_window_sam[2];
	int mini = 0, Maxi = 0;

	for (int i = 0; i < p_prov->numberVisuBlocs; i++)
	{
		mini = min(mini, p_prov->visuBlocs[i]->dispBloc->epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i]->dispBloc->epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	v_window_sam[0] = round((64 * v_window_ms[0]) / 1000);
	v_window_sam[1] = round((64 * v_window_ms[1]) / 1000);

	//cr�er gros tableau[chanel][event][sample] 
	double ***bigdata = new double**[p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb];
	for (int i = 0; i <p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; i++)
	{
		bigdata[i] = new double*[triggCatEla->numberTrigg];   
		for (int j = 0; j < triggCatEla->numberTrigg; j++)
		{
			bigdata[i][j] = new double[(v_window_sam[1] - v_window_sam[0])]();
		}
	}

	cat2ellaExtractData(p_elan->elanFreqBand[p_numberFrequencyBand], bigdata, v_window_sam);

	//[======================================================================================================================================================================================]
	QColor *ColorMapJet = new QColor[512];
	jetColorMap512(ColorMapJet);
	//[======================================================================================================================================================================================]
	MATRIXCOORD *coordMat = new MATRIXCOORD[numberRow * numberCol];
	QPixmap *pixmapTemplate = new QPixmap(576, 432);
	QPainter *painterTemplate = new QPainter(pixmapTemplate);
	pixmapTemplate->fill(QColor(Qt::white));
	createMapTemplate(painterTemplate, coordMat, p_prov, v_window_ms, ColorMapJet, stimEvents, triggCatEla->mainGroupSub, numberRow, numberCol);
	//[======================================================================================================================================================================================]
	v_window_sam[0] = round((64 * p_prov->visuBlocs[0]->dispBloc->epochWindow[0]) / 1000);
	v_window_sam[1] = round((64 * p_prov->visuBlocs[0]->dispBloc->epochWindow[1]) / 1000);
	outputPicLabel << p_outputFolder << "\\" << p_outputMapLabel;
	//[======================================================================================================================================================================================]

	//[======================================================================================================================================================================================]
	vector<vector<vector<double>>> p_value3D;
	for (int i = 0; i < p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; i++)
	{
		vector<vector<double>> p_valueBig;

		for (int j = 0; j < numberCol; j++)
		{
			for (int k = 0; k < numberRow; k++)
			{
				for (int z = 0; z < p_prov->numberVisuBlocs; z++)
				{
					if (p_prov->visuBlocs[z]->dispBloc->col == j + 1)
					{
						if (p_prov->visuBlocs[z]->dispBloc->row == k + 1)
						{
							int a = triggCatEla->mainGroupSub[correspondingEvent[z]];
							int b = triggCatEla->mainGroupSub[correspondingEvent[z] + 1];
							int numberSubTrial = b - a;

							vector<double> baseLineData, eegData;
							vector<vector<double>> eegDataBig;
							double temp = 0, temp2 = 0;

							for (int l = 0; l < numberSubTrial; l++)
							{
								int baselineDebut = round((64 * (p_prov->visuBlocs[z]->dispBloc->baseLineWindow[0] - p_prov->visuBlocs[z]->dispBloc->epochWindow[0])) / 1000);
								int baselineFin = round((64 * (p_prov->visuBlocs[z]->dispBloc->baseLineWindow[1] - p_prov->visuBlocs[z]->dispBloc->epochWindow[0])) / 1000);

								for (int m = 0; m < (baselineFin - baselineDebut); m++)
								{
									temp += bigdata[i][triggCatEla->trigg[a + l]->origPos][baselineDebut + m];
								}
								baseLineData.push_back(temp / (baselineFin - baselineDebut));
								//if (k == 8)
								//{
								//	baseLineData.push_back(temp / (baselineFin - baselineDebut));
								//}
								temp = 0;
							}

							int numberWin = p_prov->visuBlocs[z]->dispBloc->epochWindow[1] / (200 / 2);  // 200/2 car overlap 50% /!\

							for (int n = 0; n < numberWin - 1; n++)
							{
								
								for (int l = 0; l < numberSubTrial; l++)
								{
									//boucle moyenne des X fenetres que l'on veut (m�me taille que base line)

									int winDebut = round((64 * (0 + (100 * n) - p_prov->visuBlocs[z]->dispBloc->epochWindow[0])) / 1000);
									int winFin = round((64 * (200 + (100 * n) - p_prov->visuBlocs[z]->dispBloc->epochWindow[0])) / 1000);

									for (int m = 0; m < (winFin - winDebut); m++)
									{
										temp += bigdata[i][triggCatEla->trigg[a + l]->origPos][winDebut + m];
									}
									eegData.push_back(temp / (winFin - winDebut));
									//if (k == 8)
									//{
									//	eegData.push_back(temp / (winFin - winDebut));
									//}
									temp = 0;
									
								}
								eegDataBig.push_back(eegData);
								eegData.clear();
							}

							vector<double> p_value;
							for (int l = 0; l < eegDataBig.size(); l++)
							{
								p_value.push_back(wilcoxon(baseLineData, eegDataBig[l]));
							}
							p_valueBig.push_back(p_value);
						}
					}
				}
			}
		}
		p_value3D.push_back(p_valueBig);
	}
	//[======================================================================================================================================================================================]


	int V = p_value3D.size() * p_value3D[0].size() * p_value3D[0][0].size();
	int compteur = 0;
	double CV = log(V) + 0.5772;
	double slope = 0.05 / (V * CV);

	PVALUECOORD **testeuh = new PVALUECOORD *[V];
	for (int i = 0; i < p_value3D.size(); i++)
	{
		for (int j = 0; j < p_value3D[i].size(); j++)
		{
			for (int k = 0; k < p_value3D[i][j].size(); k++)
			{
				testeuh[compteur] = new PVALUECOORD();
				testeuh[compteur]->elec = i;
				testeuh[compteur]->condit = j;
				testeuh[compteur]->window = k;
				testeuh[compteur]->vectorpos = compteur;
				testeuh[compteur]->pValue = p_value3D[i][j][k];

				compteur++;
			}
		}
	}

	for (int i = 0; i < V; i++)
	{
		if (testeuh[i]->elec == 102 && testeuh[i]->condit == 8)
		{
			std::cout << testeuh[i]->pValue << std::endl;
		}
	}

	//heapsort if quicksort pas assez rapide              //compteur -1 peut �tre � cause de la sortie de boucle
	std::sort(testeuh, testeuh + compteur,
		[](PVALUECOORD *a, PVALUECOORD *b) {
		return (a->pValue < b->pValue);
	});

	int copyIndex;

	for (int i = 1; i < V; i++)														//pas 0 car premier point pose probl�me
	{
		if (testeuh[i]->pValue > ((double)slope * i))
		{
			copyIndex = i;
			break;
		}
	}

	PVALUECOORD **significantValue = new PVALUECOORD *[copyIndex];

	for (int i = 0; i < copyIndex; i++)
	{
		significantValue[i] = new PVALUECOORD();
		significantValue[i]->elec = testeuh[i]->elec;
		significantValue[i]->condit = testeuh[i]->condit;
		significantValue[i]->window = testeuh[i]->window;
		significantValue[i]->vectorpos = testeuh[i]->vectorpos;
		significantValue[i]->pValue = testeuh[i]->pValue;
	}

	std::sort(significantValue, significantValue + copyIndex,
		[](PVALUECOORD *a, PVALUECOORD *b) {
		return ((a->vectorpos < b->vectorpos));
	});

	for (int i = 0; i < copyIndex; i++)
	{
		cout << significantValue[i]->elec << "-" << significantValue[i]->condit << "-" << significantValue[i]->window << " : " << significantValue[i]->pValue << " | " << p_elan->trc->nameElectrodePositiv[p_elan->m_bipole[significantValue[i]->elec]] << endl;
	}



	//[======================================================================================================================================================================================]

	for (int i = 0; i <p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; i++)
	{
		QPixmap *pixmapChanel = new QPixmap(*pixmapTemplate);
		QPainter *painterChanel = new QPainter(pixmapChanel);

		/********************************************************************/
		/*			Calculate Min Max for each chanel						*/
		/********************************************************************/
		double stdRes = stdMean(bigdata[i], v_window_sam);
		double Maxi = 2 * abs(stdRes);
		double Mini = -2 * abs(stdRes);
		/********************************************************************/

		graduateColorBar(painterChanel, Maxi);

		int fullMatrixWidth = 394;
		int fullMatrixHeigth = 352;
		int SUBMatrixWidth = fullMatrixWidth / numberCol;
		int indexPos = 0;
		int numberSubTrial = 0;
		int interpolFactorX = 10;	//1 if no interpolation and lowest quality for matrix
		int interpolFactorY = 3;	//1 if no interpolation and lowest quality for matrix

		for (int j = 0; j < numberCol; j++)
		{ 
			for (int k = 0; k < numberRow; k++)
			{
				//==================
				//So here we search for the row/col index since it is possible for user to have a prov file totaly not weel ordonanced
				for (int z = 0; z < p_prov->numberVisuBlocs; z++)
				{
					if (p_prov->visuBlocs[z]->dispBloc->col == j + 1)
					{
						if (p_prov->visuBlocs[z]->dispBloc->row == k + 1)
						{
							int winMsMin = p_prov->visuBlocs[z]->dispBloc->epochWindow[0];
							int winMsMax = p_prov->visuBlocs[z]->dispBloc->epochWindow[1];
							int winSamMin = round((64 * winMsMin) / 1000);
							int winSamMax = round((64 * winMsMax) / 1000);
							int a = triggCatEla->mainGroupSub[correspondingEvent[z]];
							int b = triggCatEla->mainGroupSub[correspondingEvent[z] + 1];
							int numberSubTrial = b - a;
							int subsubMatrixHeigth = 0;

							std::vector<int> colorX[512], colorY[512];
							if (interpolFactorX > 1)
							{
								vector<vector<double>> eegDataInterpolated = interpolateData(bigdata[i], numberSubTrial, (winSamMax - winSamMin), a, interpolFactorX);
								vector<vector<double>> eegDataInterpolated2 = interpolateDataVert(eegDataInterpolated, interpolFactorY);
								eegData2ColorMap(eegDataInterpolated2, colorX, colorY, numberSubTrial, (winSamMax - winSamMin), interpolFactorX, interpolFactorY, Mini, Maxi);
								subsubMatrixHeigth = interpolFactorY * (numberSubTrial - 1);
							}
							else
							{
								interpolFactorX = 1; //S�curit� !

								for (int l = 0; l < numberSubTrial; l++)													   
								{																							   
									for (int m = 0; m < (winSamMax - winSamMin); m++)
									{
										double r = (bigdata[i][triggCatEla->trigg[a + l]->origPos][m] - Mini) / (Maxi - Mini);                                     

										int col = 0 + (511 * r);
										if (col < 0)
										{
											col = 0;
										}
										else if (col > 511)
										{
											col = 511;
										}
										/*2Dim vector, since changing he color of the qt pen takes time , we fill every pixel 
											of one color, then the next, then the next , etc ...*/
										colorX[col].push_back(m);
										colorY[col].push_back(l);
									}
								}

								subsubMatrixHeigth = numberSubTrial;
							}

							/*************************************************************/
							/*		ceate subBlock and paste it on the big Matrix		 */
							/*************************************************************/
							QPixmap *pixmapSubSubMatrix = new QPixmap(interpolFactorX * (winSamMax - winSamMin), subsubMatrixHeigth);
							QPainter *painterSubSubMatrix = new QPainter(pixmapSubSubMatrix);					 //
							painterSubSubMatrix->setBackgroundMode(Qt::BGMode::TransparentMode);	 //

							for (int l = 0; l < 512; l++)
							{
								painterSubSubMatrix->setPen(QColor(ColorMapJet[l].red(), ColorMapJet[l].green(), ColorMapJet[l].blue()));	 //
								for (int m = 0; m < colorX[l].size(); m++)
								{
									painterSubSubMatrix->drawPoint(QPoint(colorX[l][m], subsubMatrixHeigth - colorY[l][m]));
								}
							}

							/*Ajout des temps de r�action sur la carte*/
							if (triggCatEla->trigg[a + 2]->rt_ms == 10000000) //si un des temps de r�action � sa valeur de d�fault, � priori ils y seront tous 
							{
								painterSubSubMatrix->setPen(QColor(0, 0, 0, 255)); //black
								for (int l = 0; l < numberSubTrial; l++)
								{
									int timeValue = abs(v_window_ms[0]) + triggCatEla->trigg[a + l]->rt_ms;
									int markPoint = ceil((double)(interpolFactorX * (winSamMax - winSamMin)) / (v_window_ms[1] - v_window_ms[0]) * timeValue);
									painterSubSubMatrix->setBrush(Qt::black);
									painterSubSubMatrix->drawEllipse(QPoint(markPoint, subsubMatrixHeigth - (interpolFactorY * l)), 20, 2);
								}
							}
							/*********************************************/

							indexPos = (((j + 1) * numberRow) - 1) - correspondingEvent[z];
							painterChanel->drawPixmap(coordMat[indexPos].x, coordMat[indexPos].y, pixmapSubSubMatrix->scaled(QSize(coordMat[indexPos].width, coordMat[indexPos].heigth), Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
						}
					}
				}
			}
		}

		/* Ligne Blanche marquant l'�vennement principal */
		int zeroBorder = ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * abs(v_window_ms[0]));
		painterChanel->setPen(QColor(255, 255, 255, 255)); //white
		painterChanel->drawLine(74 + zeroBorder, 385, 74 + zeroBorder, 33);
		painterChanel->drawLine(75 + zeroBorder, 385, 75 + zeroBorder, 33);
		painterChanel->drawLine(76 + zeroBorder, 385, 76 + zeroBorder, 33);
		/************************************************/

		/**********************************************************************************************************/
		/*								Picture Label and output path											  */
		/**********************************************************************************************************/
		string elecName = p_elan->trc->nameElectrodePositiv[p_elan->m_bipole[i]];								  //
		string labelPicPath = p_outputMapLabel;																	  //
		labelPicPath.append(elecName.c_str()).append(".jpg");													  //
		string outputPicPath = outputPicLabel.str();															  //
		outputPicPath.append(elecName.c_str()).append(".jpg");													  //
																												  //
		QFont labelPic;																							  //
		labelPic.setPixelSize(11);																				  //
		painterChanel->setFont(labelPic);																		  //
		painterChanel->drawText(QPoint(120, 23), QString(labelPicPath.c_str()));								  //
		/**********************************************************************************************************/
		
		//================================= Display STATS

		for (int j = 0; j < numberCol; j++)
		{
			for (int k = 0; k < numberRow; k++)
			{
				int xTest = coordMat[((numberRow * numberCol)-1) - (k + (j*numberRow))].x + zeroBorder;
				int yTest = coordMat[((numberRow * numberCol) - 1) - (k + (j*numberRow))].y - 2 + coordMat[((numberRow * numberCol) - 1) - (k + (j*numberRow))].heigth;

				if (k == numberRow - 1)
				{
					yTest = coordMat[((numberRow * numberCol) - 1) - (k + (j*numberRow))].y - 2;
				}

				for (int l = 0; l < p_value3D[i][k].size(); l++)
				{
					for (int m = 0; m < copyIndex; m++)
					{
						if (p_value3D[i][k][l] == significantValue[m]->pValue && significantValue[m]->elec == i && significantValue[m]->window == l)
						{
							int dataPlus2 = ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * abs((200))); //abs de 100 car overlap 50% de la fen�tre de 200 ms
							int xBeg = xTest + ((dataPlus2 / 2) * (l));
							//int xEnd = xTest + ((dataPlus2 /*/ 2*/) * (l + 1));
							int xEnd = xBeg + dataPlus2;

							painterChanel->setPen(QColor(Qt::GlobalColor::white));
							painterChanel->drawLine(xBeg, yTest, xEnd, yTest);
							painterChanel->drawLine(xBeg, yTest + 1, xEnd, yTest + 1);
						}
					}
				}
			}
		}
		//======================================================================================================

		pixmapChanel->save(outputPicPath.c_str(), "JPG");
	}
}

void InsermLibrary::LOCA::cat2ellaRTTrigg(mainEventBLOC **p_mainEvents, int p_numberMainEvents, secondaryEventBLOC ***p_responseEvents, int* p_sizeRespEvents)
{
	std::vector<int> downSampTimeTrigg, downSampValueTrigg, index2delete, index2keep;
	bool tokeep = false, found = false;
	int totnumberevent, s_nextlat, s_rtms, s_rtcode, resplat;

	for (int i = 0; i < triggDownTRC->numberTrigg; i++)
	{
		downSampTimeTrigg.push_back(triggDownTRC->trigg[i]->sampleTrigger);
		downSampValueTrigg.push_back(triggDownTRC->trigg[i]->valueTrigger);
	}

	//Si les valeurs du .pos ne sont pas dans les valeurs que l'on cherche 
	for (int i = 0; i < downSampValueTrigg.size(); i++)
	{
		tokeep = false;
		for (int j = 0; j < p_numberMainEvents; j++)
		{
			if (downSampValueTrigg[i] == p_mainEvents[j]->mainEventCode)
			{
				tokeep = true;
			}
		}

		if (tokeep == false)
		{
			index2delete.push_back(i);
		}
		else
		{
			index2keep.push_back(i);
		}
	}

	//alors on les supprimes
	for (int i = index2delete.size() - 1; i >= 0; i--)
	{
		downSampValueTrigg.erase(downSampValueTrigg.begin() + index2delete[i]);
		downSampTimeTrigg.erase(downSampTimeTrigg.begin() + index2delete[i]);
	}

	triggCatEla = new TRIGGINFO(&downSampValueTrigg[0], &downSampTimeTrigg[0], (int)downSampTimeTrigg.size(), 1);
	triggCatElaNoSort = new TRIGGINFO(&downSampValueTrigg[0], &downSampTimeTrigg[0], (int)downSampTimeTrigg.size(), 1);
	totnumberevent = downSampTimeTrigg.size();

	std::vector<int> test;
	for (int m = 0; m < p_numberMainEvents; m++)
	{
		for (int j = 0; j < p_sizeRespEvents[m]; j++)
		{
			if (find(test.begin(), test.end(), p_responseEvents[m][j]->secondaryEventCode[0]) == test.end())
			{
				test.push_back(p_responseEvents[m][j]->secondaryEventCode[0]); //If multiple secondary code, just use the first one of the list
			}
		}
	}

	for (int i = 0; i < totnumberevent; i++)
	{
		if (i < totnumberevent - 1)
		{
			s_nextlat = triggCatEla->trigg[i + 1]->sampleTrigger;
		}
		else
		{
			s_nextlat = triggDownTRC->trigg[triggDownTRC->numberTrigg - 1]->sampleTrigger;
		}


		std::vector<int> m_tpos, m_sam;
		for (int k = index2keep[i] + 1; k < triggDownTRC->numberTrigg - 1; k++)
		{
			m_tpos.push_back(triggDownTRC->trigg[k]->valueTrigger);
			m_sam.push_back(triggDownTRC->trigg[k]->sampleTrigger);
		}

		s_rtms = 10000000;
		s_rtcode = -1;
		//for longueur tableau des r�ponses  
		for (int j = 0; j < test.size(); j++)
		{
			int s_resp = test[j];

			int re = find(m_tpos.begin(), m_tpos.end(), s_resp) - m_tpos.begin();
			if (re >= 0 && re != m_tpos.size())
			{
				int s_resplat = m_sam[re];
				if (s_resplat < s_nextlat)
				{
					s_rtms = min(s_rtms, 1000 * (((double)s_resplat - triggDownTRC->trigg[index2keep[i]]->sampleTrigger) / 64));
					s_rtcode = s_resp; 
				}
			}
		}

		//assign rt_ms value to struct here
		triggCatEla->trigg[i]->rt_ms = s_rtms;
		triggCatEla->trigg[i]->rt_code = s_rtcode;
	}
}

std::vector<int> InsermLibrary::LOCA::sortTrials(InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int numCol, int numRow)
{
	enum sortingChoice { CodeSort = 'C', LatencySort = 'L' };
	bool codeSortPrimary = false;
	string sortChoice = "";
	std::vector<int> correspondingEvent;

	correspondingEvent = sortByMainCode(p_prov, p_mainEvents, numCol, numRow);

	for (int i = 0; i < p_prov->numberVisuBlocs; i++)
	{
		if (p_prov->visuBlocs[i]->dispBloc->col == 1)
		{
			std::vector<int> otherLine;
			for (int j = 0; j < correspondingEvent.size(); j++)
			{
				if (correspondingEvent[j] == (p_prov->visuBlocs[i]->dispBloc->row - 1) + (numRow * (p_prov->visuBlocs[i]->dispBloc->col - 1)))
				{
					otherLine.push_back(j);
				}
			}

			sortChoice = p_prov->visuBlocs[i]->dispBloc->sort;
			vector<string> sortSplit = split<string>(sortChoice, "_");

			for (int j = 1; j < sortSplit.size(); j++)
			{
				sortingChoice Choice = (sortingChoice)(sortSplit[j][0]);
				int numberChoice = int(sortSplit[j][1]) - 48; // -48 because integer 0 is 48 in char so it'll be the number

				switch (Choice)
				{
				case CodeSort:		sortBySecondaryCode(otherLine);
									break;
				case LatencySort:   sortByLatency(otherLine);
									break;
				}
			}
		}
	}

	return correspondingEvent;
}

std::vector<int> InsermLibrary::LOCA::sortByMainCode(InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int numCol, int numRow)
{
	//Get index according to code		
	std::vector<int> correspondingEvent, test;
	for (int z = 0; z < numCol; z++)
	{
		for (int y = 0; y < numRow; y++)
		{
			for (int k = 0; k < (p_prov->numberVisuBlocs); k++)
			{
				if ((p_prov->visuBlocs[k]->dispBloc->col == z + 1) && (p_prov->visuBlocs[k]->dispBloc->row == y + 1))
				{
					correspondingEvent.push_back(y); 	//Get the right order of bloc in case prov file is in disorder

					for (int j = 0; j < triggCatEla->numberTrigg; j++)
					{
						if (triggCatEla->trigg[j]->valueTrigger == p_mainEvents[k]->mainEventCode)
						{
							test.push_back(j);
						}
					}
				}
			}
		}
	}

	//swap it
	vector<int> valueT, sampleT, rtMs, rtCode, origPos;
	for (int i = 0; i < triggCatEla->numberTrigg; i++)
	{
		valueT.push_back(triggCatEla->trigg[test[i]]->valueTrigger);
		sampleT.push_back(triggCatEla->trigg[test[i]]->sampleTrigger);
		rtMs.push_back(triggCatEla->trigg[test[i]]->rt_ms);
		rtCode.push_back(triggCatEla->trigg[test[i]]->rt_code);
		origPos.push_back(test[i]);
	}

	triggCatEla->~TRIGGINFO();
	triggCatEla = new TRIGGINFO(&valueT[0], &sampleT[0], &rtMs[0], &rtCode[0], &origPos[0], valueT.size(), 1);

	//get index limitation for each trial group according to main code
	triggCatEla->mainGroupSub.push_back(0);
	for (int m = 0; m < sampleT.size() - 1; m++)
	{
		if (sampleT[m] > sampleT[m + 1])
		{
			triggCatEla->mainGroupSub.push_back(m + 1);
		}
	}
	triggCatEla->mainGroupSub.push_back(triggCatEla->numberTrigg);

	return correspondingEvent;
}

void InsermLibrary::LOCA::sortBySecondaryCode(std::vector<int> totreat)
{
	for (int i = 0; i < totreat.size(); i++)
	{
		int beginSort = triggCatEla->mainGroupSub[totreat[i]];
		int endSort = triggCatEla->mainGroupSub[totreat[i] + 1];
		
		std::sort(triggCatEla->trigg + beginSort, triggCatEla->trigg + endSort,
			[](TRIGG *a, TRIGG *b) {
			return (a->rt_code == b->rt_code);
		});
	}
}

void InsermLibrary::LOCA::sortByLatency(std::vector<int> totreat)
{
	for (int i = 0; i < 1/*totreat.size()*/; i++)
	{		
		int beginSort = triggCatEla->mainGroupSub[totreat[i]];
		int endSort = triggCatEla->mainGroupSub[totreat[i] + 1];

		std::sort(triggCatEla->trigg + beginSort, triggCatEla->trigg + endSort,
				[](TRIGG *a, TRIGG *b) {
				return (a->rt_ms < b->rt_ms);
			});
	}
}

void InsermLibrary::LOCA::cat2ellaExtractData(elan_struct_t *p_elan_struct, double ***p_eegData, int v_win_sam[2])
{
	cout << "Exctracting data" << endl;

	for (int i = 0; i < p_elan_struct->chan_nb; i++)
	{
		for (int j = 0; j < triggCatEla->numberTrigg; j++)
		{
			//int trigTime = triggCatEla->trigg[j]->sampleTrigger;
			int trigTime = triggCatElaNoSort->trigg[j]->sampleTrigger;
			int beginTime = trigTime + v_win_sam[0];
			int endTime = trigTime + v_win_sam[1];

			for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]); k++)
			{
				//p_eegData[i][j][k] = ((p_elan_struct->eeg.data_double[0][i][beginTime + k] * 10) - 1000) / 10; //old jp .eeg
				p_eegData[i][j][k] = (p_elan_struct->eeg.data_double[0][i][beginTime + k] - 1000) / 10; //flo .eeg
			}
		}
	}

	cout << "End of Data Extraction " << endl;
}

std::vector<std::vector<double>> InsermLibrary::LOCA::interpolateData(double **p_eegData, int p_numberSubTrial, int p_windowSize, int p_beginTrigg, int p_interpolFactor)
{
	vector<vector<double>> eegDataInterpolated;
	for (int l = 0; l < p_numberSubTrial; l++)
	{
		vector<double> eegDataOneTrial;
		for (int m = 0; m < p_windowSize; m++)
		{
			double val1 = p_eegData[triggCatEla->trigg[p_beginTrigg + l]->origPos][m];
			double val2 = p_eegData[triggCatEla->trigg[p_beginTrigg + l]->origPos][m + 1];

			for (int n = 0; n < p_interpolFactor; n++)
			{
				double coeff = (double)n / p_interpolFactor;
				eegDataOneTrial.push_back(((1 - coeff)*val1) + (coeff * val2));
			}
		}
		eegDataInterpolated.push_back(eegDataOneTrial);
	}

	return eegDataInterpolated;
}

std::vector<std::vector<double>> InsermLibrary::LOCA::interpolateDataVert(std::vector<std::vector<double>> p_eegData, int p_interpolFactor)
{
	int vertSize = p_eegData.size();
	int horizSize = p_eegData[0].size();

	vector<vector<double>> eegDataInterpolated;
	for (int l = 0; l < horizSize; l++)
	{
		vector<double> eegDataOneSample;
		for (int m = 0; m < vertSize - 1; m++)
		{
			double val1 = p_eegData[m][l];
			double val2 = p_eegData[m + 1][l];

			for (int n = 0; n < p_interpolFactor; n++)
			{
				double coeff = (double)n / p_interpolFactor;
				eegDataOneSample.push_back(((1 - coeff)*val1) + (coeff * val2));
			}
		}
		eegDataInterpolated.push_back(eegDataOneSample);
	}

	return eegDataInterpolated;
}

void InsermLibrary::LOCA::eegData2ColorMap(std::vector<std::vector<double>> p_eegDataInterpolated, std::vector<int> p_colorX[512], std::vector<int> p_colorY[512], int p_numberSubTrial, int p_windowSize, int p_interpolFactorX, int p_interpolFactorY, double p_minVal, double p_maxVal)
{
	for (int l = 0; l < p_interpolFactorY * (p_numberSubTrial - 1); l++)
	{
		for (int m = 0; m < p_interpolFactorX * p_windowSize; m++)
		{
			//double r = (p_eegDataInterpolated[l][m] - p_minVal) / (p_maxVal - p_minVal); //si p_eegDataInterpolated seulement par le nombre de sample
			double r = (p_eegDataInterpolated[m][l] - p_minVal) / (p_maxVal - p_minVal);  //si p_eegDataInterpolated par nombre de sample puis par trials

			int col = 0 + (511 * r);
			if (col < 0)
			{
				col = 0;
			}
			else if (col > 511)
			{
				col = 511;
			}
			/*2Dim vector, since changing he color of the qt pen takes time , we fill every pixel
			of one color, then the next, then the next , etc ...*/
			p_colorX[col].push_back(m);
			p_colorY[col].push_back(l);
		}
	}
}

/********************************************************************************************************************************************/
/*							Create a template with non variable object inside the image													    */
/********************************************************************************************************************************************/
void InsermLibrary::LOCA::createMapTemplate(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, InsermLibrary::PROV *p_prov, int v_window_ms[2], QColor *p_colorMap, mainEventBLOC **p_mainEvents, vector<int> p_indexes, int p_numberRow, int p_numberCol)
{
	matrixLines(p_painterTemplate, p_coordMat, p_indexes, p_numberRow, p_numberCol);
	createTimeLegend(p_painterTemplate, v_window_ms);
	createTrialLegend(p_painterTemplate, p_coordMat, p_prov, p_mainEvents, p_numberRow);
	createColorBar(p_painterTemplate, p_colorMap);
}

void InsermLibrary::LOCA::createColorBar(QPainter *p_painterTemplate, QColor *p_colorMap)
{
	/*************************************************************************************************************************************************************************************/
	/*					Create Rectangle for Color legend according to chosen color map	and pregraduate it with the zero since it won't change place									 */
	/*************************************************************************************************************************************************************************************/
	p_painterTemplate->drawRect(484, 33, 21, 352);																																		 //
	for (int i = 0; i < 512; i++)																																						 //
	{																																													 //
		QPainterPath qpathRect;																																							 //
		qpathRect.moveTo(QPoint(485, 33 + (((double)352 / 512)*i)));																													 //
		qpathRect.lineTo(QPoint(485 + 19, 33 + (((double)352 / 512)*(i + 1))));																											 //
																																														 //
		p_painterTemplate->setPen(p_colorMap[511 - i]);																																	 //
		p_painterTemplate->drawPath(qpathRect);																																			 //
	}																																													 //
	p_painterTemplate->drawLine(484, 209, 488, 209);																																	 //
	p_painterTemplate->drawText(QPoint(507, 215), QString("0"));																														 //
	p_painterTemplate->drawLine(501, 209, 505, 209);																																	 //
	/*************************************************************************************************************************************************************************************/
}

void InsermLibrary::LOCA::createTrialLegend(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int p_numberMainEvents)
{
	/************************************************************************************************************************************************************/
	/*																Matrix Trial Legend																			*/
	/************************************************************************************************************************************************************/
	int d, e;
	QFont fontTrialLegend;																																		//
	fontTrialLegend.setPixelSize(15);																															//
	double stepTrialLegend = 352 / p_numberMainEvents;																											//
																																								//
	p_painterTemplate->setPen(QColor(Qt::darkRed));																												//
	p_painterTemplate->setFont(fontTrialLegend);																												//

	for (int a = 0; a < p_numberMainEvents; a++)
	{
		for (int i = 0; i < p_prov->numberVisuBlocs; i++)
		{
			if (p_prov->visuBlocs[i]->dispBloc->col == 1)
			{
				if (p_prov->visuBlocs[i]->dispBloc->row == a + 1)
				{
					if (a == 0)
					{
						d = 385;
						e = p_coordMat[p_numberMainEvents - 1 - a].y;
					}
					else
					{
						d = p_coordMat[p_numberMainEvents - a].y;
						e = p_coordMat[p_numberMainEvents - a - 1].y;
					}
					
					p_painterTemplate->drawText(QPoint(1, (d - (double)(d - e) / 2)), QString(p_mainEvents[i]->mainEventLabel.c_str()));
				}
			}
		}
	}
	///************************************************************************************************************************************************************/

}

void InsermLibrary::LOCA::createTimeLegend(QPainter *p_painterTemplate, int v_window_ms[2])
{
	/*********************************************************************************************************************************************************************************************************/
	/*																					Matrix Time Legend																									 */
	/*********************************************************************************************************************************************************************************************************/
	int zeroBorder = ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * abs(v_window_ms[0]));																											 //
	double stepTimeLegend = v_window_ms[1] / 5;																																								 //
																																																			 //
	p_painterTemplate->setBackgroundMode(Qt::BGMode::OpaqueMode);																																	 		 //
	p_painterTemplate->drawLine(75, 385, 469, 385);																																							 //
	p_painterTemplate->drawLine(469, 385, 469, 381);																																						 //
																																																			 //
	p_painterTemplate->drawText(QPoint((73 + zeroBorder) - ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * stepTimeLegend) - 9, 400), QString(QString().number(0 - stepTimeLegend)));				 //
	p_painterTemplate->drawText(QPoint(73 + zeroBorder, 400), QString("0"));																																 //
	p_painterTemplate->drawText(QPoint((73 + zeroBorder) + ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * (stepTimeLegend)) - 9, 400), QString(QString().number(0 + stepTimeLegend)));				 //
	p_painterTemplate->drawText(QPoint((73 + zeroBorder) + ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * (2 * stepTimeLegend)) - 9, 400), QString(QString().number(0 + (2 * stepTimeLegend))));	 //
	p_painterTemplate->drawText(QPoint((73 + zeroBorder) + ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * (3 * stepTimeLegend)) - 9, 400), QString(QString().number(0 + (3 * stepTimeLegend))));	 //
	p_painterTemplate->drawText(QPoint((73 + zeroBorder) + ceil((double)394 / (v_window_ms[1] - v_window_ms[0]) * (4 * stepTimeLegend)) - 9, 400), QString(QString().number(0 + (4 * stepTimeLegend))));	 //
	p_painterTemplate->drawText(QPoint(460, 400), QString(QString().number(v_window_ms[1])));																												 //
	/*********************************************************************************************************************************************************************************************************/
}

void InsermLibrary::LOCA::matrixLines(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, vector<int> p_indexes, int p_numberRow, int p_numberCol)
{
	int xBegin = 0, yBegin = 0, xEnd = 0, yEnd = 0;
	int fullMatrixWidth = 394, fullMatrixHeigth = 352;
	int numberHorizLine = p_numberRow - 1, numberVerticLine = p_numberCol - 1;
	int actualMatrixWidth = fullMatrixWidth - (numberVerticLine * 2), actualMatrixHeigth = fullMatrixHeigth - (numberHorizLine * 2);
	int heigthcumul;

	////Fill an array with x,y coordinates and width and heigth corresponding to each part of submatrix
	for (int i = 0; i < p_numberCol; i++)
	{
		heigthcumul = 0;
		for (int j = 0; j < p_numberRow; j++)
		{
			p_coordMat[j + (i*p_numberRow)].width = (actualMatrixWidth / p_numberCol);
			p_coordMat[j + (i*p_numberRow)].heigth = round(actualMatrixHeigth * ((double)(p_indexes[p_numberRow - j] - p_indexes[p_numberRow - 1 - j]) / triggCatEla->numberTrigg));

			p_coordMat[j + (i*p_numberRow)].x = 75 + (((actualMatrixWidth / p_numberCol) + 2) * i);
			p_coordMat[j + (i*p_numberRow)].y = 33 + heigthcumul;

			heigthcumul += p_coordMat[j + (i*p_numberRow)].heigth + 2;
		}
	}


	//horizontal lines
	for (int i = 0; i < p_numberRow - 1; i++)
	{
		xBegin = p_coordMat[i+1].x;
		yBegin = p_coordMat[i+1].y - 2;
		xEnd = p_coordMat[i].x + fullMatrixWidth - 1;
		yEnd = yBegin;

		p_painterTemplate->setPen(QColor(Qt::black));
		p_painterTemplate->drawLine(xBegin, yBegin, xEnd, yEnd);

		xBegin = p_coordMat[i + 1].x;
		yBegin = p_coordMat[i + 1].y - 1;
		xEnd = p_coordMat[i].x + fullMatrixWidth - 1;
		yEnd = yBegin;

		p_painterTemplate->setPen(QColor(Qt::black));
		p_painterTemplate->drawLine(xBegin, yBegin, xEnd, yEnd);
	}

	//vertical lines
	if (p_numberCol > 1)
	{
		for (int i = 0; i < p_numberCol - 1; i++)
		{
			xBegin = p_coordMat[((i+1)*p_numberRow)].x - 2;
			yBegin = 33;
			xEnd = xBegin;
			yEnd = 33 + fullMatrixHeigth;

			p_painterTemplate->drawLine(xBegin, yBegin, xEnd, yEnd);

			xBegin = p_coordMat[((i + 1)*p_numberRow)].x - 1;
			yBegin = 33;
			xEnd = xBegin;
			yEnd = 33 + fullMatrixHeigth;

			p_painterTemplate->drawLine(xBegin, yBegin, xEnd, yEnd);

		}
	}
}
/********************************************************************************************************************************************/

/********************************************************************************************************************/
/*						Calculate standard derivation = mean then sqrt(sum((data-mean)�))							*/
/********************************************************************************************************************/
double InsermLibrary::LOCA::stdMean(double **p_eegDataChanel, int p_window_sam[2])
{
	double *v_erp, *v_std;

	v_erp = new double[triggCatEla->numberTrigg];
	v_std = new double[triggCatEla->numberTrigg];


	for (int i = 0; i < triggCatEla->numberTrigg; i++)																    
	{																													
		double tempErp = 0;																								
		for (int m = 0; m < (p_window_sam[1] - p_window_sam[0]); m++)													
		{																												
			tempErp += p_eegDataChanel[triggCatEla->trigg[i]->origPos][m];
		}																												
		v_erp[i] = tempErp / (p_window_sam[1] - p_window_sam[0]);														
	}																													
																														
	for (int i = 0; i < triggCatEla->numberTrigg; i++)																	
	{																													
		double tempStd = 0;																								
		for (int m = 0; m < (p_window_sam[1] - p_window_sam[0]); m++)													
		{																												
			tempStd += (p_eegDataChanel[triggCatEla->trigg[i]->origPos][m] - v_erp[i]) * (p_eegDataChanel[triggCatEla->trigg[i]->origPos][m] - v_erp[i]);
		}																												
		v_std[i] = sqrt(tempStd / (p_window_sam[1] - p_window_sam[0] - 1));													
	}																													

	double tempstd = 0;													
	for (int k = 0; k < triggCatEla->numberTrigg; k++)					
	{																	
		tempstd += v_std[k];											
	}																	
	tempstd /= triggCatEla->numberTrigg;								
	

	delete[] v_erp;
	delete[] v_std;

	return tempstd;
}
/********************************************************************************************************************/

/******************************************************************************************************************/
/*			Create Rectangle for Color legend according to chosen color map	and graduate it						  */
/******************************************************************************************************************/
void InsermLibrary::LOCA::graduateColorBar(QPainter *p_painterTemplate, int p_maxiValue)																								 //
{																																														 //
	p_painterTemplate->setPen(QColor(0, 0, 0, 255));																																	 //
	double stepColorLegend = (double)p_maxiValue / 10;																																	 //
	double stepColorLegend2 = ((double)352 / (2 * stepColorLegend));																													 //
																																														 //
	for (int i = 0; i < floor(stepColorLegend); i++)																																	 //
	{																																													 //
		p_painterTemplate->drawLine(484, 209 + (stepColorLegend2 + stepColorLegend2 * i), 488, 209 + (stepColorLegend2 + stepColorLegend2 * i));								 		 //
		p_painterTemplate->drawLine(501, 209 + (stepColorLegend2 + stepColorLegend2 * i), 505, 209 + (stepColorLegend2 + stepColorLegend2 * i));								 		 //
																																														 //
		p_painterTemplate->drawLine(484, 209 - (stepColorLegend2 + stepColorLegend2 * i), 488, 209 - (stepColorLegend2 + stepColorLegend2 * i));								 		 //
		p_painterTemplate->drawLine(501, 209 - (stepColorLegend2 + stepColorLegend2 * i), 505, 209 - (stepColorLegend2 + stepColorLegend2 * i));								 		 //
																																														 //
		p_painterTemplate->drawText(QPoint(507, 215 + (((double)352 / (2 * stepColorLegend)) + (((double)352 / (2 * stepColorLegend)) * i))), QString(QString().number(-10 - (10 * i))));//
		p_painterTemplate->drawText(QPoint(507, 215 - (((double)352 / (2 * stepColorLegend)) + (((double)352 / (2 * stepColorLegend)) * i))), QString(QString().number(10 + (10 * i)))); //
	}																																													 //
}																																														 //
/******************************************************************************************************************/

/******************************************************************************************************************/
/*																												  */
/******************************************************************************************************************/
void InsermLibrary::LOCA::jetColorMap512(QColor *p_C)
{
	int compteur = 0;
	for (int i = 0; i < 57; i++)
	{
		p_C[i].setBlue(143.4375 + (i * 1.9649));
	}

	compteur = 57;
	for (int i = 0; i < 130; i++)
	{
		p_C[compteur].setGreen((double)0.4366 + (i * 1.9649));
		p_C[compteur].setBlue(255);
		compteur++;
	}

	compteur = 187;
	for (int i = 0; i < 130; i++)
	{
		p_C[compteur].setRed(0.8733 + (i * 1.9649));
		p_C[compteur].setGreen(255);
		p_C[compteur].setBlue(254.1267 - (i * 1.9649));
		compteur++;
	}

	compteur = 317;
	for (int i = 0; i < 130; i++)
	{
		p_C[compteur].setRed(255);
		p_C[compteur].setGreen(253.6901 - (i * 1.9649));
		p_C[compteur].setBlue(0);
		compteur++;
	}

	compteur = 447;
	for (int i = 0; i < 65; i++)
	{
		p_C[compteur].setRed(253.2534 - (i * 1.9649));
		p_C[compteur].setGreen(0);
		p_C[compteur].setBlue(0);
		compteur++;
	}

	//for (int i = 447; i < 512; i++)
	//{
	//	std::cout << C[i].red() <<" et " << C[i].green() << " et " << C[i].blue() << endl;
	//}
}
/******************************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/

/******************************************************************************************************************/
/*																												  */
/******************************************************************************************************************/
template<typename T> vector<T> InsermLibrary::LOCA::split(const T & str, const T & delimiters)					  //
{																												  //
	/*Exemple : vector<string> v = split<string>("Hello, there; World", ";,"); */								  //
	vector<T> v;																								  //
	T::size_type start = 0;																						  //
	auto pos = str.find_first_of(delimiters, start);															  //
	while (pos != T::npos) {																					  //
		if (pos != start) // ignore empty tokens																  //
			v.emplace_back(str, start, pos - start);															  //
		start = pos + 1;																						  //
		pos = str.find_first_of(delimiters, start);																  //
	}																											  //
	if (start < str.length()) // ignore trailing delimiter														  //
		v.emplace_back(str, start, str.length() - start); // add what's left of the string						  //
	return v;																									  //
}																												  //
/******************************************************************************************************************/
