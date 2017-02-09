#include "LOCA.h"
#include <time.h>  
#include <QDir>

#include "barsGenerator.h"

/****************************************************************************************************************************/
/******************************************               LOCA Methods              *****************************************/
/****************************************************************************************************************************/
InsermLibrary::LOCA::LOCA(OptionLOCA *p_options)
{
	opt = p_options;
}

InsermLibrary::LOCA::~LOCA()
{
	deleteAndNullify1D(triggTRC);
	deleteAndNullify1D(triggDownTRC);
	deleteAndNullify1D(triggCatEla);
	deleteAndNullify1D(triggCatElaNoSort);
	deleteAndNullify1D(painter);
	deleteAndNullify1D(pixMap);
	deleteAndNullify1D(opt);
}

/********************************************************************************************************************/
/*																												    */
/********************************************************************************************************************/
void InsermLibrary::LOCA::LocaSauron(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt)
{
	stringstream posFilePath, confFilePath, posXFilePath, eeg2envFilePath, pictureLabel, folderTrialsSM, outputMessage, loadFileDefault;

	/*****************************/
	/*Create.pos and dsX.pos File*/
	/*****************************/
	posFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".pos";													 
	posXFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_ds" << (p_elan->trc->samplingFrequency / 64) << ".pos"; 
	loc_create_pos(posFilePath.str(), posXFilePath.str(), p_elan->trc, 99, p_prov);

	/*******************/
	/*Create .conf File*/
	/*******************/
	confFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".conf"; 
	loc2_write_conf(confFilePath.str(), p_elan->trc, p_elan);

	stringstream().swap(outputMessage);
	outputMessage << "Pos and Conf Files Have been created";
	emit sendLogInfo(QString::fromStdString(outputMessage.str()));

	if (p_anaopt->analysisDetails[0][0])
	{
		loc_eeg2erp(p_elan, p_prov, p_anaopt, "");
		stringstream().swap(outputMessage);
		outputMessage << "ERP Pictures DONE !";
		emit sendLogInfo(QString::fromStdString(outputMessage.str()));
	}

	vector<int> correspondingEvents = processEvents(p_prov);

	//Cr perf LEC1 MCSE MVIS MVEB
	if (p_anaopt->task == "LEC1")
	{
		InsermLibrary::CRperf::LEC1CR(p_anaopt, triggCatEla);
	}
	else if (p_anaopt->task == "MCSE")
	{
		InsermLibrary::CRperf::MCSECR(p_anaopt, triggCatEla);
	}
	else if (p_anaopt->task == "MVIS")
	{
		InsermLibrary::CRperf::MVISMVEBCR(p_anaopt, triggCatEla);
	}
	else if (p_anaopt->task == "MVEB")
	{
		InsermLibrary::CRperf::MVISMVEBCR(p_anaopt, triggCatEla);
	}
	//EndCr

	//Inv
	if (p_prov->invertmapsinfo != "")
	{
		for (int i = 0; i < triggCatEla->numberTrigg; i++)
		{
			int temp = round(triggCatElaNoSort->trigg[i].rt_ms * 64) / 1000;
			triggCatElaNoSort->trigg[i].sampleTrigger = triggCatElaNoSort->trigg[i].sampleTrigger + temp;

			triggCatEla->trigg[i].rt_ms = -triggCatEla->trigg[i].rt_ms;
		}

		for (int i = 0; i < p_prov->visuBlocs.size(); i++)
		{
			p_prov->visuBlocs[i].dispBloc.epochWindow[0] = p_prov->invertmaps.epochWindow[0];
			p_prov->visuBlocs[i].dispBloc.epochWindow[1] = p_prov->invertmaps.epochWindow[1];

			p_prov->visuBlocs[i].dispBloc.baseLineWindow[0] = p_prov->invertmaps.baseLineWindow[0];
			p_prov->visuBlocs[i].dispBloc.baseLineWindow[1] = p_prov->invertmaps.baseLineWindow[1];
		}
	}
	//endinv

	eeg2envFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".TRC";
	for (int i = 0; i < p_anaopt->frequencys.size(); i++)
	{
		if (p_anaopt->analysisDetails[i + 1][0])
		{
			stringstream().swap(outputMessage);
			outputMessage << "Starting Frequency Analysis for " << p_anaopt->frequencys[i][0] << " -> " << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "Hz";
			emit sendLogInfo(QString::fromStdString(outputMessage.str()));

			if (p_anaopt->analysisDetails[i + 1][1]) //if calcul eeg2env
			{
				/**************************/						  
				/* EEG2ENV XXXHz -> YYYHz */						  
				/**************************/						  
				p_elan->TrcToEnvElan(eeg2envFilePath.str(), p_elan->elanFreqBand[i], &p_anaopt->frequencys[i][0], p_anaopt->frequencys[i].size(), ELAN_HISTO);					  
				/**************************/					      
				stringstream().swap(outputMessage);
				outputMessage << "Hilbert Envellope Calculated";
				emit sendLogInfo(QString::fromStdString(outputMessage.str()));

				if (p_anaopt->analysisDetails[i + 1][2])
				{
					//===============================================================================================================================================================================
					stringstream().swap(pictureLabel);
					stringstream().swap(folderTrialsSM);
					if (p_anaopt->task == "AUDI" || p_anaopt->task == "MOTO" || p_anaopt->task == "MVIS"
												 || p_anaopt->task == "MVEB" || p_anaopt->task == "ARFA")
					{
						pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_bar_";
						folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_bar";
					}
					else
					{
						pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_plots_";
						folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_plots";
					}

					if (opt->statsOption.useKruskall)
					{
						if (opt->statsOption.useFDRKrus)
						{
							folderTrialsSM << "_FDR" << opt->statsOption.pKruskall;
						}
						else
						{
							folderTrialsSM << "_P" << opt->statsOption.pKruskall;
						}
					}

					if (!QDir(&folderTrialsSM.str()[0]).exists())
					{
						stringstream().swap(outputMessage);
						outputMessage << "Creating Output Folder for" << p_anaopt->frequencys[i][0] << " -> " << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << " Hz data";
						emit sendLogInfo(QString::fromStdString(outputMessage.str()));
						QDir().mkdir(&folderTrialsSM.str()[0]);
					}
					//===============================================================================================================================================================================

					/*****************************************/
					/*				loc_env2plot			 */
					/*****************************************/	
					if (p_anaopt->task == "AUDI" || p_anaopt->task == "MOTO" || p_anaopt->task == "MVIS"
												 || p_anaopt->task == "MVEB" || p_anaopt->task == "ARFA")
					{
						loc_bar2plot(p_elan, p_prov, p_anaopt, correspondingEvents, i, folderTrialsSM.str());
					}
					else
					{
						loc_env2plot(p_elan, p_prov, p_anaopt, i, folderTrialsSM.str());
					}
					/*****************************************/
					stringstream().swap(outputMessage);
					outputMessage << "Hilbert Env Pictures DONE !";
					emit sendLogInfo(QString::fromStdString(outputMessage.str()));
				}

				if (p_anaopt->analysisDetails[i + 1][3])
				{
					//===============================================================================================================================================================================
					stringstream().swap(pictureLabel);
					stringstream().swap(folderTrialsSM);
					pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";
					folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_trials";

					if (opt->statsOption.useWilcoxon)
					{
						if (opt->statsOption.useFDRWil)
						{
							folderTrialsSM << "_FDR" << opt->statsOption.pWilcoxon;
						}
						else
						{
							folderTrialsSM << "_P" << opt->statsOption.pWilcoxon;
						}
					}

					if (!QDir(&folderTrialsSM.str()[0]).exists())
					{
						stringstream().swap(outputMessage);
						outputMessage << "Creating Output Folder for" << p_anaopt->frequencys[i][0] << " -> " << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << " Hz data";
						emit sendLogInfo(QString::fromStdString(outputMessage.str()));
						QDir().mkdir(&folderTrialsSM.str()[0]);
					}
					//===============================================================================================================================================================================

					loca_trialmat(p_elan, i, p_prov, pictureLabel.str(), folderTrialsSM.str(), correspondingEvents);
					stringstream().swap(outputMessage);
					outputMessage << "Frequency Maps DONE !";
					emit sendLogInfo(QString::fromStdString(outputMessage.str()));
				}
			}
			else //else load eeg file
			{
				//===============================================================================================================================================================================
				stringstream().swap(loadFileDefault);
				loadFileDefault << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0.eeg";
				int loadFile = ef_read_elan_file((char*)loadFileDefault.str().c_str(), p_elan->elanFreqBand[i]);
				//===============================================================================================================================================================================

				if (loadFile == 0)
				{
					if (p_anaopt->analysisDetails[i + 1][2])
					{
						//===============================================================================================================================================================================
						stringstream().swap(pictureLabel);
						stringstream().swap(folderTrialsSM);
						if (p_anaopt->task == "AUDI" || p_anaopt->task == "MOTO" || p_anaopt->task == "MVIS"
													 || p_anaopt->task == "MVEB" || p_anaopt->task == "ARFA")
						{
							pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_bar_";
							folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_bar";
						}
						else
						{
							pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_plots_";
							folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_plots";
						}

						if (opt->statsOption.useKruskall)
						{
							if (opt->statsOption.useFDRKrus)
							{
								folderTrialsSM << "_FDR" << opt->statsOption.pKruskall;
							}
							else
							{
								folderTrialsSM << "_P" << opt->statsOption.pKruskall;
							}
						}

						if (!QDir(&folderTrialsSM.str()[0]).exists())
						{
							stringstream().swap(outputMessage);
							outputMessage << "Creating Output Folder for" << p_anaopt->frequencys[i][0] << " -> " << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << " Hz data";
							emit sendLogInfo(QString::fromStdString(outputMessage.str()));
							QDir().mkdir(&folderTrialsSM.str()[0]);
						}
						//===============================================================================================================================================================================

						/*****************************************/
						/*				loc_env2plot			 */
						/*****************************************/
						if (p_anaopt->task == "AUDI" || p_anaopt->task == "MOTO" || p_anaopt->task == "MVIS"
													 || p_anaopt->task == "MVEB" || p_anaopt->task == "ARFA")
						{
							loc_bar2plot(p_elan, p_prov, p_anaopt, correspondingEvents, i, folderTrialsSM.str());
						}
						else
						{
							loc_env2plot(p_elan, p_prov, p_anaopt, i, folderTrialsSM.str());
						}
						/*****************************************/
						stringstream().swap(outputMessage);
						outputMessage << "Hilbert Env Pictures DONE !";
						emit sendLogInfo(QString::fromStdString(outputMessage.str()));
					}

					if (p_anaopt->analysisDetails[i + 1][3])
					{
						//===============================================================================================================================================================================
						stringstream().swap(pictureLabel);
						stringstream().swap(folderTrialsSM);
						pictureLabel << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_ds" << (p_elan->trc->samplingFrequency / 64) << "_sm0_trials_";
						folderTrialsSM << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << "_f" << p_anaopt->frequencys[i][0] << "f" << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << "_trials";
						
						if (opt->statsOption.useWilcoxon)
						{
							if (opt->statsOption.useFDRWil)
							{
								folderTrialsSM << "_FDR" << opt->statsOption.pWilcoxon;
							}
							else
							{
								folderTrialsSM << "_P" << opt->statsOption.pWilcoxon;
							}
						}

						if (!QDir(&folderTrialsSM.str()[0]).exists())
						{
							stringstream().swap(outputMessage);
							outputMessage << "Creating Output Folder for" << p_anaopt->frequencys[i][0] << " -> " << p_anaopt->frequencys[i][p_anaopt->frequencys[i].size() - 1] << " Hz data";
							emit sendLogInfo(QString::fromStdString(outputMessage.str()));
							QDir().mkdir(&folderTrialsSM.str()[0]);
						}
						//===============================================================================================================================================================================
						
						loca_trialmat(p_elan, i, p_prov, pictureLabel.str(), folderTrialsSM.str(), correspondingEvents);
						stringstream().swap(outputMessage);
						outputMessage << "Frequency Maps DONE !";
						emit sendLogInfo(QString::fromStdString(outputMessage.str()));
					}
				}
				else
				{
					stringstream().swap(outputMessage);
					outputMessage << "No Envellope File found, end of analyse for this frequency";
					emit sendLogInfo(QString::fromStdString(outputMessage.str()));
				}
			}
		}//end freq
	}
}
/********************************************************************************************************************/

/******************************************************************************************************************************************/
/*									CREATE POS AND CONF : ELAN COMPATIBILITY															  */
/******************************************************************************************************************************************/
void InsermLibrary::LOCA::loc_create_pos(string posFile_path, string posXFile_path, TRC *p_trc, int p_beginningCode, PROV *p_prov)
{
	int beginValue = 0;

	ofstream fichierPos(posFile_path, ios::out);  // patient.pos with data for samplingFrequency										  
	ofstream fichierPosX(posXFile_path, ios::out);  // patient_dsX.pos with data for samplingFrequency downsampled to 64 sample per sec									  

	//start .pos after last beginningCode that indicates beginning of expe
	vector<int> indexBegin = findIndexes((int*)&p_trc->valueTrigg[0], p_trc->sampleTrigg.size() - 1, p_beginningCode);
	if (indexBegin.size() > 0)
	{
		beginValue = indexBegin[indexBegin.size() - 1] + 1;
	}

	//create two pointer and class containing trigger info for further use
	if (triggTRC != nullptr)
	{
		delete triggTRC;
	}
	triggTRC = new TRIGGINFO(&p_trc->valueTrigg[beginValue], &p_trc->sampleTrigg[beginValue], (int)(p_trc->sampleTrigg.size() - beginValue), 1);

	if (triggDownTRC != nullptr)
	{
		delete triggDownTRC;
	}
	triggDownTRC = new TRIGGINFO(&p_trc->valueTrigg[beginValue], &p_trc->sampleTrigg[beginValue], (int)(p_trc->sampleTrigg.size() - beginValue), (int)(p_trc->samplingFrequency / 64));

	if (p_prov->changeCodeFilePath != "")
	{
		renameTrigger(&triggTRC[0], &triggDownTRC[0], p_prov);
	}

	//write output file
	for (int i = 0; i < (p_trc->sampleTrigg.size() - beginValue); i++)
	{
		fichierPos << triggTRC->trigg[i].sampleTrigger << setw(10) << triggTRC->trigg[i].valueTrigger << setw(10) << "0" << endl;
		fichierPosX << triggDownTRC->trigg[i].sampleTrigger << setw(10) << triggDownTRC->trigg[i].valueTrigger << setw(10) << "0" << endl;
	}

	fichierPos.close();
	fichierPosX.close();
}

void InsermLibrary::LOCA::renameTrigger(TRIGGINFO *triggers, TRIGGINFO* downsampledTriggers, PROV *p_prov)
{
	int indexVisuBloc = 0, winMax = 0, winMin = 0;
	vector<int> oldMainCode, oldSecondaryCode, newMainCode, newSecondaryCode;
	stringstream buffer;
	ifstream provFile(p_prov->changeCodeFilePath, ios::binary);																						   								                                              //
	if (provFile)																																																						      //
	{																																																									      //
		// Copy all file in buffer																																																		      //
		buffer << provFile.rdbuf();																																																		      //
		// Don't need the file anymore ! 																																																      //
		provFile.close();																																																				      //

		vector<string> lineSplit = InsermLibrary::split<string>(buffer.str(), "\r\n");
		for (int i = 0; i < lineSplit.size(); i++)
		{
			vector<string> elementSplit = InsermLibrary::split<string>(lineSplit[i], "+=");
			oldMainCode.push_back(atoi(&(elementSplit[0])[0]));
			oldSecondaryCode.push_back(atoi(&(elementSplit[1])[0]));
			newMainCode.push_back(atoi(&(elementSplit[2])[0]));
			newSecondaryCode.push_back(atoi(&(elementSplit[3])[0]));
		}

		for (int i = 0; i < oldMainCode.size(); i++)
		{
			for (int j = 0; j < p_prov->visuBlocs.size(); j++)
			{
				for (int k = 0; k < p_prov->visuBlocs[j].mainEventBloc.eventCode.size(); k++)
				{
					if (p_prov->visuBlocs[j].mainEventBloc.eventCode[k] == oldMainCode[i])
					{
						indexVisuBloc = j;
					}
				}
			}

			int winSamMin = round((64 * p_prov->visuBlocs[indexVisuBloc].dispBloc.epochWindow[0]) / 1000);
			int winSamMax = round((64 * p_prov->visuBlocs[indexVisuBloc].dispBloc.epochWindow[1]) / 1000);

			int count = 0;
			for (int j = 0; j < downsampledTriggers->numberTrigg; j++)
			{
				if (downsampledTriggers->trigg[j].valueTrigger == oldMainCode[i])
				{
					count = j + 1;
					winMax = downsampledTriggers->trigg[j].sampleTrigger + winSamMax;
					winMin = downsampledTriggers->trigg[j].sampleTrigger - abs(winSamMin);//+ winSamMin;
					while ((downsampledTriggers->trigg[count].sampleTrigger < winMax) && (downsampledTriggers->trigg[count].sampleTrigger > winMin))
					{
						if (downsampledTriggers->trigg[count].valueTrigger == oldSecondaryCode[i])
						{
							//cout << triggers->trigg[j]->valueTrigger << "to " << newMainCode[i] << endl;
							//cout << triggers->trigg[count]->valueTrigger << "to " << newSecondaryCode[i] << endl;
							//cout << endl;

							triggers->trigg[j].valueTrigger = newMainCode[i];
							triggers->trigg[count].valueTrigger = newSecondaryCode[i];
							//==========================================================================
							downsampledTriggers->trigg[j].valueTrigger = newMainCode[i];
							downsampledTriggers->trigg[count].valueTrigger = newSecondaryCode[i];
							//break;
							count++;
						}	
						else
						{
							break;
						}
					}
				}
			}
		}
	}																																																									      //
	else																																																								      //
	{ 																																																									      //
		cout << " Error opening Change Code File @ " << p_prov->changeCodeFilePath.c_str() << endl;																															      //
	}																																																									      //
}

void InsermLibrary::LOCA::loc2_write_conf(string confFile_path, TRC *p_trc, ELAN *p_elan)
{
	//[=================================================================================================================================]										  
	ofstream fichierConf(confFile_path, ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

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
	/* On affiche l'index du bipole tant que le suivant fait partie de la même éléctrode : A'1 , A'2, A'3 ...  */
	/* puis quand le suivant n'est pas de la même éléctrode ( B'1 ) on met -1 et c'est repartir pour un tour   */
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
/******************************************************************************************************************************************/

/**************************************************************/
/*							EEG2ERP							  */
/**************************************************************/
void InsermLibrary::LOCA::loc_eeg2erp(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, string outputFolder)
{
	vector<int> indexEventUsed, eventUsed;
	int windowSample[2]{ (int)round(p_elan->trc->samplingFrequency * p_prov->visuBlocs[0].dispBloc.epochWindow[0] / 1000),
						 (int)round(p_elan->trc->samplingFrequency * p_prov->visuBlocs[0].dispBloc.epochWindow[1] / 1000) };

	for (int i = 0; i < triggTRC->numberTrigg; i++)
	{
		for (int j = 0; j < p_prov->visuBlocs.size(); j++)
		{
			if (triggTRC->trigg[i].valueTrigger == p_prov->visuBlocs[j].mainEventBloc.eventCode[0])
			{
				indexEventUsed.push_back(i);
				eventUsed.push_back(triggTRC->trigg[i].valueTrigger);
			}
		}
	}

	double ***m_bigdata_mono = allocate3DArray<double>(eventUsed.size(), p_elan->trc->nameElectrodePositiv.size(), (windowSample[1] - windowSample[0]) + 1);
	double ***m_bigdata_bipo = allocate3DArray<double>(eventUsed.size(), p_elan->m_bipole.size(), (windowSample[1] - windowSample[0]) + 1);

	cout << "Reading Data Mono ..." << endl;
	for (int i = 0; i < eventUsed.size(); i++)
	{	
		for (int j = 0; j < (windowSample[1] - windowSample[0]) + 1; j++)
		{	
			for (int k = 0; k < p_elan->trc->nameElectrodePositiv.size(); k++)	
			{	
				int beginPos = triggTRC->trigg[indexEventUsed[i]].sampleTrigger + windowSample[0];
				m_bigdata_mono[i][k][j] = p_elan->trc->eegData[k][beginPos + j];
			}
		}	
	}

	cout << "Reading Data Bipo ..." << endl;	
	for (int i = 0; i < eventUsed.size(); i++)
	{	
		for (int k = 0; k < p_elan->m_bipole.size(); k++)	
		{	
			for (int j = 0; j < (windowSample[1] - windowSample[0]) + 1; j++)
			{
				m_bigdata_bipo[i][k][j] = m_bigdata_mono[i][p_elan->m_bipole[k]][j] - m_bigdata_mono[i][p_elan->m_bipole_moins[k]][j];	
			}	
		}
	}

	InsermLibrary::DrawPlotsVisu::drawPlots d = InsermLibrary::DrawPlotsVisu::drawPlots::drawPlots(p_prov, outputFolder);
	d.drawDataOnTemplate(p_elan, p_anaopt, m_bigdata_mono, eventUsed, -1, 0);
	d.drawDataOnTemplate(p_elan, p_anaopt, m_bigdata_bipo, eventUsed, -1, 1);

	deAllocate3DArray(m_bigdata_mono, eventUsed.size(), p_elan->trc->nameElectrodePositiv.size());
	deAllocate3DArray(m_bigdata_bipo, eventUsed.size(), p_elan->m_bipole.size());
}

/**************************************************************/
/*							ENV2PLOT						  */
/**************************************************************/
void InsermLibrary::LOCA::loc_env2plot(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, int currentFreqBand, string outputFolder)
{
	vector<int> indexEventUsed, eventUsed;
	int windowSample[2]{ (int) round(64 * p_prov->visuBlocs[0].dispBloc.epochWindow[0] / 1000),
						 (int) round(64 * p_prov->visuBlocs[0].dispBloc.epochWindow[1] / 1000) };

	for (int i = 0; i < triggTRC->numberTrigg; i++)
	{
		for (int j = 0; j < p_prov->visuBlocs.size(); j++)
		{
			if (triggTRC->trigg[i].valueTrigger == p_prov->visuBlocs[j].mainEventBloc.eventCode[0])
			{
				indexEventUsed.push_back(i);
				eventUsed.push_back(triggTRC->trigg[i].valueTrigger);
			}
		}
	}

	double ***m_bigdata_frequency = allocate3DArray<double>(eventUsed.size(), p_elan->
									elanFreqBand[currentFreqBand]->chan_nb, (windowSample[1] - windowSample[0]) + 1);

	cout << "Reading Hilberted Bipo ..." << endl;	
	for (int i = 0; i < eventUsed.size(); i++)	
	{	
		int beginInd = triggDownTRC->trigg[indexEventUsed[i]].sampleTrigger + windowSample[0];	
		for (int j = 0; j < (windowSample[1] - windowSample[0]) + 1; j++)
		{	
			for (int k = 0; k < p_elan->elanFreqBand[currentFreqBand]->chan_nb; k++)
			{	
				m_bigdata_frequency[i][k][j] = p_elan->elanFreqBand[currentFreqBand]->eeg.data_double[0][k][beginInd + j];
			}	
		}
	}			

	InsermLibrary::DrawPlotsVisu::drawPlots d = InsermLibrary::DrawPlotsVisu::drawPlots::drawPlots(p_prov, outputFolder);
	d.drawDataOnTemplate(p_elan, p_anaopt, m_bigdata_frequency, eventUsed, currentFreqBand, 2);
	deAllocate3DArray(m_bigdata_frequency, eventUsed.size(), p_elan->elanFreqBand[currentFreqBand]->chan_nb);
}

/**************************************************************/
/*							BAR2PLOT						  */
/**************************************************************/
void InsermLibrary::LOCA::loc_bar2plot(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, vector<int> p_correspondingEvents, int currentFreqBand, string outputFolder)
{
	vector<int> indexEventUsed, eventUsed;
	int windowSample[2]{ (int) round(64 * p_prov->visuBlocs[0].dispBloc.epochWindow[0] / 1000),
						 (int) round(64 * p_prov->visuBlocs[0].dispBloc.epochWindow[1] / 1000) };
	int copyIndex = 0;
	vector<vector<vector<double>>> p_value3D;
	vector<vector<vector<int>>> p_sign3D;
	vector<PVALUECOORD> significantValue;

	for (int i = 0; i < triggTRC->numberTrigg; i++)	
	{					
		for (int j = 0; j < p_prov->visuBlocs.size(); j++)	
		{						
			if (triggTRC->trigg[i].valueTrigger == p_prov->visuBlocs[j].mainEventBloc.eventCode[0])	
			{									
				indexEventUsed.push_back(i);
				eventUsed.push_back(triggTRC->trigg[i].valueTrigger);
			}													
		}					
	}																		

	double ***m_bigDataFrequency = allocate3DArray<double>(eventUsed.size(), p_elan->elanFreqBand[currentFreqBand]->chan_nb, (windowSample[1] - windowSample[0]) + 1);

	cout << "Reading Hilberted Mono ..." << endl;
	for (int i = 0; i < eventUsed.size(); i++)
	{		
		int beginInd = triggDownTRC->trigg[indexEventUsed[i]].sampleTrigger + windowSample[0];
		for (int j = 0; j < (windowSample[1] - windowSample[0]) + 1; j++)
		{	
			for (int k = 0; k < p_elan->elanFreqBand[currentFreqBand]->chan_nb; k++)
			{
				m_bigDataFrequency[i][k][j] = p_elan->elanFreqBand[currentFreqBand]->eeg.data_double[0][k][beginInd + j];
			}
		}
	}

	if (opt->statsOption.useKruskall)
	{
		copyIndex = 0;
		p_value3D = InsermLibrary::Stats::pValuesKruskall(p_elan->elanFreqBand[currentFreqBand], p_prov, triggCatEla, p_correspondingEvents, m_bigDataFrequency);
		p_sign3D = InsermLibrary::Stats::signKruskall(p_elan->elanFreqBand[currentFreqBand], p_prov, triggCatEla, p_correspondingEvents, m_bigDataFrequency);
		if (opt->statsOption.useFDRKrus)
		{
			significantValue = InsermLibrary::Stats::FDR(p_value3D, p_sign3D, copyIndex, opt->statsOption.pWilcoxon);
		}
		else
		{
			significantValue = InsermLibrary::Stats::loadPValues(p_value3D, p_sign3D, opt->statsOption.pWilcoxon);
		}

		InsermLibrary::Stats::exportStatsData(p_elan, p_prov, significantValue, outputFolder, true);
	}

	InsermLibrary::DrawPlotsVisu::drawBars b = InsermLibrary::DrawPlotsVisu::drawBars::drawBars(p_prov, outputFolder);
	b.drawDataOnTemplate(p_elan, p_anaopt, currentFreqBand, m_bigDataFrequency, eventUsed, significantValue);

	deAllocate3DArray(m_bigDataFrequency, eventUsed.size(), p_elan->elanFreqBand[currentFreqBand]->chan_nb);
}

/**************************************************************/
/*							TRIALMAT						  */
/**************************************************************/
void InsermLibrary::LOCA::loca_trialmat(ELAN *p_elan, int p_numberFrequencyBand, PROV *p_prov, string p_outputMapLabel, string p_outputFolder, vector<int> p_correspondingEvents)
{
	stringstream outputPicLabel;

	//[======================================================================================================================================================================================]
	//We calculate the biggest window there will be so that we extract the block before analysis
	int v_window_ms[2], v_window_sam[2];
	int mini = 0, Maxi = 0;

	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		mini = min(mini, p_prov->visuBlocs[i].dispBloc.epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i].dispBloc.epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	v_window_sam[0] = round((64 * v_window_ms[0]) / 1000);
	v_window_sam[1] = round((64 * v_window_ms[1]) / 1000);

	//[chanel][event][sample] 
	double ***bigdata = allocate3DArray<double>(p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb, triggCatEla->numberTrigg, (v_window_sam[1] - v_window_sam[0]));
	cat2ellaExtractData(p_elan->elanFreqBand[p_numberFrequencyBand], bigdata, v_window_sam);

	//[======================================================================================================================================================================================]
	int copyIndex = 0;
	vector<vector<vector<double>>> p_value3D;
	vector<vector<vector<int>>> p_sign3D;
	vector<PVALUECOORD> significantValue;

	if (opt->statsOption.useWilcoxon)
	{
		p_value3D = InsermLibrary::Stats::pValuesWilcoxon(p_elan->elanFreqBand[p_numberFrequencyBand], p_prov, triggCatEla, p_correspondingEvents, bigdata);
		p_sign3D = InsermLibrary::Stats::signWilcoxon(p_elan->elanFreqBand[p_numberFrequencyBand], p_prov, triggCatEla, p_correspondingEvents, bigdata);

		if (opt->statsOption.useFDRWil)
		{
			significantValue = InsermLibrary::Stats::FDR(p_value3D, p_sign3D, copyIndex, opt->statsOption.pWilcoxon);
		}
		else
		{
			significantValue = InsermLibrary::Stats::loadPValues(p_value3D, p_sign3D, opt->statsOption.pWilcoxon);
		}

		InsermLibrary::Stats::exportStatsData(p_elan, p_prov, significantValue, p_outputFolder, false);
	}
	
	//[==================================================================================================]
	mapsGenerator mGen(opt->picOption.width, opt->picOption.height);
	mGen.trialmatTemplate(triggCatEla->mainGroupSub, v_window_ms, p_prov);
	//[==================================================================================================]
	int numberCol = p_prov->nbCol();
	int numberRow = p_prov->nbRow();
	v_window_sam[0] = round((64 * p_prov->visuBlocs[0].dispBloc.epochWindow[0]) / 1000);
	v_window_sam[1] = round((64 * p_prov->visuBlocs[0].dispBloc.epochWindow[1]) / 1000);
	outputPicLabel << p_outputFolder << "/" << p_outputMapLabel;

	QPixmap *pixmapChanel = nullptr, *pixmapSubSubMatrix = nullptr;
	QPainter *painterChanel = nullptr, *painterSubSubMatrix = nullptr;
	for (int i = 0; i < p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb; i++)
	{
		deleteAndNullify1D(painterChanel);
		deleteAndNullify1D(pixmapChanel);
		pixmapChanel = new QPixmap(mGen.pixmapTemplate);
		painterChanel = new QPainter(pixmapChanel);

		double stdRes = stdMean(bigdata[i], v_window_sam);
		double Maxi = 2 * abs(stdRes);
		double Mini = -2 * abs(stdRes);
		mGen.graduateColorBar(painterChanel, Maxi);

		int SUBMatrixWidth = mGen.MatrixRect.width() / numberCol;
		int interpolFactorX = opt->picOption.interpolationFactorX;	//1 if no interpolation and lowest quality for matrix
		int interpolFactorY = opt->picOption.interpolationFactorY;	//1 if no interpolation and lowest quality for matrix

		int indexPos = 0;
		int numberSubTrial = 0;
		for (int j = 0; j < numberCol; j++)
		{ 
			for (int k = 0; k < numberRow; k++)
			{
				//==================
				//So here we search for the row/col index since it is possible for user to have a prov file totaly not weel ordonanced
				for (int z = 0; z < p_prov->visuBlocs.size(); z++)
				{
					if (p_prov->visuBlocs[z].dispBloc.col == j + 1)
					{
						if (p_prov->visuBlocs[z].dispBloc.row == k + 1)
						{
							int winMsMin = p_prov->visuBlocs[z].dispBloc.epochWindow[0];
							int winMsMax = p_prov->visuBlocs[z].dispBloc.epochWindow[1];
							int winSamMin = round((64 * winMsMin) / 1000);
							int winSamMax = round((64 * winMsMax) / 1000);
							int nbSampleWindow = winSamMax - winSamMin;
							int indexBegTrigg = triggCatEla->mainGroupSub[p_correspondingEvents[z]];
							int numberSubTrial = triggCatEla->mainGroupSub[p_correspondingEvents[z] + 1] - indexBegTrigg;
							int subsubMatrixHeigth = 0;
							
							vector<int> colorX[512], colorY[512];
							if (interpolFactorX > 1)
							{
								vector<vector<double>> dataInterpolatedHoriz = mGen.horizontalInterpolation(bigdata[i], interpolFactorX, triggCatEla, numberSubTrial, nbSampleWindow, indexBegTrigg);
								vector<vector<double>> dataInterpolatedVerti = mGen.verticalInterpolation(dataInterpolatedHoriz, interpolFactorY);
								mGen.eegData2ColorMap(colorX, colorY, dataInterpolatedVerti, numberSubTrial, nbSampleWindow, interpolFactorX, interpolFactorY, Maxi);
								
								subsubMatrixHeigth = interpolFactorY * (numberSubTrial - 1);
							}
							else
							{
								mGen.eegData2ColorMap(colorX, colorY, bigdata[i], triggCatEla, numberSubTrial, nbSampleWindow, indexBegTrigg, Maxi);
								
								subsubMatrixHeigth = numberSubTrial;
							}

							/***************************************************/
							/*	ceate subBlock and paste it on the big Matrix  */
							/***************************************************/
							deleteAndNullify1D(painterSubSubMatrix);
							deleteAndNullify1D(pixmapSubSubMatrix);
							pixmapSubSubMatrix = new QPixmap(interpolFactorX * nbSampleWindow, subsubMatrixHeigth);	
							painterSubSubMatrix = new QPainter(pixmapSubSubMatrix);	
							painterSubSubMatrix->setBackgroundMode(Qt::BGMode::TransparentMode);
							
							for (int l = 0; l < 512; l++)
							{	
								painterSubSubMatrix->setPen(QColor(mGen.ColorMapJet[l].red(), mGen.ColorMapJet[l].green(), mGen.ColorMapJet[l].blue()));  
								for (int m = 0; m < colorX[l].size(); m++)
								{
									painterSubSubMatrix->drawPoint(QPoint(colorX[l][m], subsubMatrixHeigth - colorY[l][m]));
								}	
							}

							indexPos = (((j + 1) * numberRow) - 1) - p_correspondingEvents[z];	
							painterChanel->drawPixmap(mGen.subMatrixes[indexPos].x(), mGen.subMatrixes[indexPos].y(),
								pixmapSubSubMatrix->scaled(QSize(mGen.subMatrixes[indexPos].width(), mGen.subMatrixes[indexPos].height()), 
								Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));


							/*********************************************************************************************************/
							/*	 							    Ajout des temps de réaction sur la carte							 */
							/*		=> Si un des temps de reaction n'a pas sa valeur de défault, à priori ils y seront tous			 */
							/*********************************************************************************************************/
							if (triggCatEla->trigg[indexBegTrigg + 2].rt_ms != 10000000) 
							{
								painterChanel->setPen(QColor(Qt::black));
								for (int l = 0; l < numberSubTrial; l++)
								{	
									int xReactionTimeMs = abs(winMsMin) + triggCatEla->trigg[indexBegTrigg + l].rt_ms;
									double xScale = (double)(winMsMax - winMsMin) / mGen.MatrixRect.width();
									double xRt = mGen.MatrixRect.x() + (xReactionTimeMs / xScale);

									int yTrialPosition = mGen.subMatrixes[indexPos].y() + mGen.subMatrixes[indexPos].height();	
									double yRt = yTrialPosition - (((double)mGen.subMatrixes[indexPos].height() / numberSubTrial) * l) - 1;
					
									painterChanel->setBrush(Qt::black);	
									painterChanel->drawEllipse(QPoint(xRt, yRt), (int)(0.0034722 * mGen.MatrixRect.width()), (int)(0.004629629 * mGen.MatrixRect.height()));
								}	
							}	
						}
					}
				}
			}
		}

		mGen.drawVerticalZeroLine(painterChanel, v_window_ms);

		// Display STATS
		if (opt->statsOption.useWilcoxon)
		{   
			vector<int> allIndexCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
			if (allIndexCurrentMap.size() > 0)
			{
				vector<vector<int>> indexCurrentMap = mGen.checkIfConditionStat(significantValue, allIndexCurrentMap, p_prov->nbRow());
				mGen.displayStatsOnMap(painterChanel, indexCurrentMap, significantValue, v_window_ms, p_prov->nbRow());
			}
		}

		// Display Title
		string elecName = p_elan->trc->nameElectrodePositiv[p_elan->m_bipole[i]];
		mGen.drawMapTitle(painterChanel, p_outputMapLabel, elecName);

		//Save Map
		string outputPicPath = outputPicLabel.str();
		outputPicPath.append(elecName.c_str()).append(".jpg");
		pixmapChanel->save(outputPicPath.c_str(), "JPG");
	}

	deAllocate3DArray(bigdata, p_elan->elanFreqBand[p_numberFrequencyBand]->chan_nb, triggCatEla->numberTrigg);
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);
	deleteAndNullify1D(painterSubSubMatrix);
	deleteAndNullify1D(pixmapSubSubMatrix);
}

/**************************************************************/
/*				Trigger management							  */
/**************************************************************/
vector<int> InsermLibrary::LOCA::processEvents(PROV *p_prov)
{
	cat2ellaRTTrigg(p_prov);	//	Extract triggers 
	return sortTrials(p_prov);	//	Get the index corresponding to trials sorted according to input	
}

void InsermLibrary::LOCA::cat2ellaRTTrigg(PROV *p_prov)
{
	vector<int> downSampTimeTrigg, downSampValueTrigg, index2delete, index2keep;
	bool tokeep = false, found = false;
	int totnumberevent, s_nextlat, s_rtms, s_rtcode, resplat;

	for (int i = 0; i < triggDownTRC->numberTrigg; i++)
	{
		downSampTimeTrigg.push_back(triggDownTRC->trigg[i].sampleTrigger);
		downSampValueTrigg.push_back(triggDownTRC->trigg[i].valueTrigger);
	}

	//Si les valeurs du .pos ne sont pas dans les valeurs que l'on cherche 
	for (int i = 0; i < downSampValueTrigg.size(); i++)
	{
		tokeep = false;
		for (int j = 0; j < p_prov->visuBlocs.size(); j++)
		{
			for (int k = 0; k < p_prov->visuBlocs[j].mainEventBloc.eventCode.size(); k++)
			{
				if (downSampValueTrigg[i] == p_prov->visuBlocs[j].mainEventBloc.eventCode[k])
				{
					tokeep = true;
				}
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

	if (triggCatEla != nullptr)
	{
		delete triggCatEla;
		triggCatEla = nullptr;
	}
	triggCatEla = new TRIGGINFO(&downSampValueTrigg[0], &downSampTimeTrigg[0], (int)downSampTimeTrigg.size(), 1);

	if (triggCatElaNoSort != nullptr)
	{
		delete triggCatElaNoSort;
		triggCatElaNoSort = nullptr;
	}
	triggCatElaNoSort = new TRIGGINFO(&downSampValueTrigg[0], &downSampTimeTrigg[0], (int)downSampTimeTrigg.size(), 1);
	totnumberevent = downSampTimeTrigg.size();

	vector<int> test;
	for (int m = 0; m < p_prov->visuBlocs.size(); m++)
	{
		for (int j = 0; j < p_prov->visuBlocs[m].secondaryEvents.size(); j++)
		{
			if (find(test.begin(), test.end(), p_prov->visuBlocs[m].secondaryEvents[j].eventCode[0]) == test.end())
			{
				test.push_back(p_prov->visuBlocs[m].secondaryEvents[j].eventCode[0]); //If multiple secondary code, just use the first one of the list
			}
		}
	}

	for (int i = 0; i < totnumberevent; i++)
	{
		if (i < totnumberevent - 1)
		{
			s_nextlat = triggCatEla->trigg[i + 1].sampleTrigger;
		}
		else
		{
			s_nextlat = triggDownTRC->trigg[triggDownTRC->numberTrigg - 1].sampleTrigger;
		}

		vector<int> m_tpos, m_sam;
		for (int k = index2keep[i] + 1; k < triggDownTRC->numberTrigg - 1; k++)
		{
			m_tpos.push_back(triggDownTRC->trigg[k].valueTrigger);
			m_sam.push_back(triggDownTRC->trigg[k].sampleTrigger);
		}

		s_rtms = 10000000;
		s_rtcode = -1;
		//for longueur tableau des réponses  
		for (int j = 0; j < test.size(); j++)
		{
			int s_resp = test[j];

			int re = find(m_tpos.begin(), m_tpos.end(), s_resp) - m_tpos.begin();
			if (re >= 0 && re != m_tpos.size())
			{
				int s_resplat = m_sam[re];
				if (s_resplat < s_nextlat)
				{
					s_rtms = min(s_rtms, 1000 * (((double)s_resplat - triggDownTRC->trigg[index2keep[i]].sampleTrigger) / 64));
					s_rtcode = s_resp; 
				}
			}
		}

		//assign rt_ms value to struct here
		triggCatEla->trigg[i].rt_ms = s_rtms;
		triggCatEla->trigg[i].rt_code = s_rtcode;

		//TEST inv
		triggCatElaNoSort->trigg[i].rt_ms = s_rtms;
		triggCatElaNoSort->trigg[i].rt_code = s_rtcode;
	}

	if (p_prov->invertmapsinfo != "")
	{
		for (int i = triggCatEla->trigg.size() - 1; i >= 0; i--)
		{
			if (triggCatEla->trigg[i].rt_ms == 10000000)
			{
				triggCatElaNoSort->trigg.erase(triggCatElaNoSort->trigg.begin() + i);
				triggCatElaNoSort->numberTrigg = triggCatElaNoSort->trigg.size();
				triggCatEla->trigg.erase(triggCatEla->trigg.begin() + i);
				triggCatEla->numberTrigg = triggCatEla->trigg.size();
			}
		}
	}
}

vector<int> InsermLibrary::LOCA::sortTrials(PROV *p_prov)
{
	enum sortingChoice { CodeSort = 'C', LatencySort = 'L' };
	bool codeSortPrimary = false;
	string sortChoice = "";
	vector<int> correspondingEvent;

	correspondingEvent = sortByMainCode(p_prov);

	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		if (p_prov->visuBlocs[i].dispBloc.col == 1)
		{
			vector<int> otherLine;
			for (int j = 0; j < correspondingEvent.size(); j++)
			{
				if (correspondingEvent[j] == (p_prov->visuBlocs[i].dispBloc.row - 1) + (p_prov->nbRow() * (p_prov->visuBlocs[i].dispBloc.col - 1)))
				{
					otherLine.push_back(j);
				}
			}

			sortChoice = p_prov->visuBlocs[i].dispBloc.sort;
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

vector<int> InsermLibrary::LOCA::sortByMainCode(PROV *p_prov)
{
	//Get index according to code		
	vector<int> correspondingEvent, test;
	for (int z = 0; z < p_prov->nbCol(); z++)
	{
		for (int y = 0; y < p_prov->nbRow(); y++)
		{
			for (int k = 0; k < (p_prov->visuBlocs.size()); k++)
			{
				if ((p_prov->visuBlocs[k].dispBloc.col == z + 1) && (p_prov->visuBlocs[k].dispBloc.row == y + 1))
				{
					correspondingEvent.push_back(y); 	//Get the right order of bloc in case prov file is in disorder
					
					for (int j = 0; j < triggCatEla->numberTrigg; j++)
					{
						for (int w = 0; w < p_prov->visuBlocs[k].mainEventBloc.eventCode.size(); w++)
						{
							if (triggCatEla->trigg[j].valueTrigger == p_prov->visuBlocs[k].mainEventBloc.eventCode[w])   // p_mainEvents[k]->mainEventCode)
							{
								test.push_back(j);
							}
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
		valueT.push_back(triggCatEla->trigg[test[i]].valueTrigger);
		sampleT.push_back(triggCatEla->trigg[test[i]].sampleTrigger);
		rtMs.push_back(triggCatEla->trigg[test[i]].rt_ms);
		rtCode.push_back(triggCatEla->trigg[test[i]].rt_code);
		origPos.push_back(test[i]);
	}

	delete triggCatEla;
	triggCatEla = nullptr;
	triggCatEla = new TRIGGINFO(&valueT[0], &sampleT[0], &rtMs[0], &rtCode[0], &origPos[0], valueT.size(), 1);

	//get index limitation for each trial group according to main code
	triggCatEla->mainGroupSub.push_back(0);
	for (int m = 0; m < sampleT.size() - 1; m++)
	{
		if(valueT[m] != valueT[m + 1])
		{
			triggCatEla->mainGroupSub.push_back(m + 1);
		}
	}
	triggCatEla->mainGroupSub.push_back(triggCatEla->numberTrigg);

	return correspondingEvent;
}

void InsermLibrary::LOCA::sortBySecondaryCode(vector<int> totreat)
{
	for (int i = 0; i < totreat.size(); i++)
	{
		int beginSort = triggCatEla->mainGroupSub[totreat[i]];
		int endSort = triggCatEla->mainGroupSub[totreat[i] + 1];
		
		sort(triggCatEla->trigg.begin() + beginSort, triggCatEla->trigg.begin() + endSort,
			[](TRIGG a, TRIGG b) {
			return (a.rt_code == b.rt_code);
		});
	}
}

void InsermLibrary::LOCA::sortByLatency(vector<int> totreat)
{
	for (int i = 0; i < 1/*totreat.size()*/; i++)
	{		
		int beginSort = triggCatEla->mainGroupSub[totreat[i]];
		int endSort = triggCatEla->mainGroupSub[totreat[i] + 1];

		sort(triggCatEla->trigg.begin() + beginSort, triggCatEla->trigg.begin() + endSort,
				[](TRIGG a, TRIGG b) {
				return (a.rt_ms < b.rt_ms);
			});
	}
}
/**************************************************************/

//== Will be in ELAN Lib later
void InsermLibrary::LOCA::cat2ellaExtractData(elan_struct_t *p_elan_struct, double ***p_eegData, int v_win_sam[2])
{
	//===fin test inv
	cout << "Exctracting data" << endl;
	string toSearch = p_elan_struct->comment;
	if (toSearch.find("by eeg2env C++") != string::npos)
	{
		cout << "new" << endl;
		for (int i = 0; i < p_elan_struct->chan_nb; i++)
		{
			for (int j = 0; j < triggCatEla->numberTrigg; j++)
			{
				//int trigTime = triggCatEla->trigg[j].sampleTrigger + triggCatEla->trigg[j].rt_ms;
				int trigTime = triggCatElaNoSort->trigg[j].sampleTrigger;
				int beginTime = trigTime + v_win_sam[0];
				int endTime = trigTime + v_win_sam[1];

				for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]); k++)
				{
					p_eegData[i][j][k] = (p_elan_struct->eeg.data_double[0][i][beginTime + k] - 1000) / 10; //flo .eeg
				}
			}
		}
	}
	else if (toSearch.find("") != string::npos)
	{
		cout << "running analysis" << endl;
		for (int i = 0; i < p_elan_struct->chan_nb; i++)
		{
			for (int j = 0; j < triggCatEla->numberTrigg; j++)
			{
				//int trigTime = triggCatEla->trigg[j]->sampleTrigger;
				int trigTime = triggCatElaNoSort->trigg[j].sampleTrigger;
				int beginTime = trigTime + v_win_sam[0];
				int endTime = trigTime + v_win_sam[1];

				for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]); k++)
				{
					p_eegData[i][j][k] = (p_elan_struct->eeg.data_double[0][i][beginTime + k] - 1000) / 10; //flo .eeg
				}
			}
		}
	}
	else
	{
		cout << "old" << endl;
		for (int i = 0; i < p_elan_struct->chan_nb; i++)
		{
			for (int j = 0; j < triggCatEla->numberTrigg; j++)
			{
				//int trigTime = triggCatEla->trigg[j]->sampleTrigger;
				int trigTime = triggCatElaNoSort->trigg[j].sampleTrigger;
				int beginTime = trigTime + v_win_sam[0];
				int endTime = trigTime + v_win_sam[1];

				for (int k = 0; k < (v_win_sam[1] - v_win_sam[0]); k++)
				{
					p_eegData[i][j][k] = ((p_elan_struct->eeg.data_double[0][i][beginTime + k] * 10) - 1000) / 10; //old jp .eeg
				}
			}
		}
	}



	cout << "End of Data Extraction " << endl;
}

//== Will be in Math lib later
/****************************************************************/
/*	Standard Derivation => mean then sqrt(sum((data-mean)²))	*/
/****************************************************************/
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
			tempErp += p_eegDataChanel[triggCatEla->trigg[i].origPos][m];
		}																												
		v_erp[i] = tempErp / (p_window_sam[1] - p_window_sam[0]);														
	}																													
																														
	for (int i = 0; i < triggCatEla->numberTrigg; i++)																	
	{																													
		double tempStd = 0;																								
		for (int m = 0; m < (p_window_sam[1] - p_window_sam[0]); m++)													
		{																												
			tempStd += (p_eegDataChanel[triggCatEla->trigg[i].origPos][m] - v_erp[i]) * (p_eegDataChanel[triggCatEla->trigg[i].origPos][m] - v_erp[i]);
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