#include "LOCA.h"

InsermLibrary::LOCA::LOCA(userOption *userOpt)
{
	this->userOpt = userOpt;
}

InsermLibrary::LOCA::~LOCA()
{
	//deleteAndNullify1D(userOpt);
}

/************/
/* eeg2erp  */
/************/
void InsermLibrary::LOCA::eeg2erp(eegContainer *myeegContainer, PROV *myprovFile)
{
	int *windowSam = myprovFile->getBiggestWindowSam(myeegContainer->sampInfo.samplingFrequency);
	vec1<string> origFilePath = split<string>(myeegContainer->originalFilePath, "/");
	string outputFolder = myeegContainer->originalFilePath;
	outputFolder.append("_ERP/");
	if (!QDir(&outputFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for erp Maps"));
		QDir().mkdir(&outputFolder.c_str()[0]);
	}
	string picPathMono = outputFolder; 
	picPathMono = picPathMono.append(origFilePath[origFilePath.size() - 1]);

	processEvents(myeegContainer, myprovFile); 

	vec3<float> bigDataMono = vec3<float>(triggCatEla->triggers.size(), vec2<float>(myeegContainer->flatElectrodes.size(), vec1<float>(windowSam[1] - windowSam[0])));
	vec3<float> bigDataBipo = vec3<float>(triggCatEla->triggers.size(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));

	cout << "Reading Data Mono ..." << endl;
	for (int i = 0; i < triggCatEla->triggers.size(); i++)
	{
		for (int j = 0; j < (windowSam[1] - windowSam[0]); j++)
		{
			for (int k = 0; k < myeegContainer->flatElectrodes.size(); k++)
			{
				int beginPos = triggCatEla->triggers[i].trigger.sample + windowSam[0];
				bigDataMono[i][k][j] = myeegContainer->Data()[k][beginPos + j];
			}
		}
	}

	cout << "Reading Data Bipo ..." << endl;
	for (int i = 0; i < triggCatEla->triggers.size(); i++)
	{
		for (int k = 0; k < myeegContainer->BipoleCount(); k++)
		{
			for (int j = 0; j < (windowSam[1] - windowSam[0]); j++)
			{
				bigDataBipo[i][k][j] = bigDataMono[i][myeegContainer->Bipole(k).first][j] -
									   bigDataMono[i][myeegContainer->Bipole(k).second][j];
			}
		}
	}

	drawPlots b = drawPlots(myprovFile, picPathMono, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(bigDataMono, triggCatEla, myeegContainer, 0);
	emit sendLogInfo("Mono Maps Generated");
	b.drawDataOnTemplate(bigDataBipo, triggCatEla, myeegContainer, 1);
	emit sendLogInfo("Bipo Maps Generated");
	emit incrementAdavnce(1);

	delete windowSam;
}

//		               Three::rings
//		          for:::the::Elven-Kings
//		       under:the:sky,:Seven:for:the
//		     Dwarf-Lords::in::their::halls:of
//		    stone,:Nine             for:Mortal
//		   :::Men:::     ________     doomed::to
//		 die.:One   _,-'...:... `-.    for:::the
//		 ::Dark::  ,- .:::::::::::. `.   Lord::on
//		his:dark ,'  .:::::zzz:::::.  `.  :throne:
//		In:::the/    ::::dMMMMMb::::    \ Land::of
//		:Mordor:\    ::::dMMmgJP::::    / :where::
//		::the::: '.  '::::YMMMP::::'  ,'  Shadows:
//		 lie.::One  `. ``:::::::::'' ,'    Ring::to
//		 ::rule::    `-._```:'''_,-'     ::them::
//		 all,::One      `-----'        ring::to
//		   ::find:::                  them,:One
//		    Ring:::::to            bring::them
//		      all::and::in:the:darkness:bind
//		        them:In:the:Land:of:Mordor
//		           where:::the::Shadows
//		                :::lie.:::

void InsermLibrary::LOCA::LocaSauron(eegContainer* myeegContainer, int idCurrentLoca, locaFolder *currentLoca)
{
	this->idCurrentLoca = idCurrentLoca;
	this->currentLoca = currentLoca;

	for (int i = 0; i < userOpt->freqOption.frequencyBands.size(); i++)
	{
		frequency currentFrequencyBand = frequency(userOpt->freqOption.frequencyBands[i]);
		checkShannonCompliance(myeegContainer->sampInfo.samplingFrequency, currentFrequencyBand);
		if (userOpt->anaOption[idCurrentLoca].anaOpt[i].eeg2env)
		{
			myeegContainer->ToHilbert(i, currentFrequencyBand.freqBandValue);
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			string freqFolder = createIfFreqFolderExistNot(myeegContainer, currentFrequencyBand);
			toBeNamedCorrectlyFunction(myeegContainer, i, freqFolder, userOpt->anaOption[idCurrentLoca].anaOpt[i]);
		}
		else 
		{
			for (int j = 0; j < currentLoca->frequencyFolders().size(); j++)
			{
				int sizeFreq = currentFrequencyBand.freqBandValue.size() - 1;
				string fMin = to_string(currentFrequencyBand.freqBandValue[0]);
				string fMax = to_string(currentFrequencyBand.freqBandValue[sizeFreq]);

				if ((currentLoca->frequencyFolders()[j].frequencyName() == "f" + fMin + "f" + fMax) &&
					(currentLoca->frequencyFolders()[j].filePath(SM0_ELAN) != ""))
				{
					std::string dataFile = currentLoca->frequencyFolders()[j].filePath(SM0_ELAN);

					std::vector<std::string> filesPath;
					filesPath.push_back(dataFile + ".ent");
					filesPath.push_back(dataFile);

					if (EEGFormat::Utility::IsValidFile(filesPath[0]) && EEGFormat::Utility::IsValidFile(filesPath[1]))
					{				
						myeegContainer->LoadFrequencyData(filesPath, i, 0);
						emit incrementAdavnce(1);
						emit sendLogInfo("Envelloppe File Loaded");

						string freqFolder = createIfFreqFolderExistNot(myeegContainer, currentFrequencyBand);
						toBeNamedCorrectlyFunction(myeegContainer, i, freqFolder, userOpt->anaOption[idCurrentLoca].anaOpt[i]);
					}
					else
					{
						emit sendLogInfo("Problem loading file, end of analyse for this frequency");
					}
				}
			}
		}
	}
}

void InsermLibrary::LOCA::LocaFrequency(eegContainer *myeegContainer, int idCurrentLoca)
{
	this->idCurrentLoca = idCurrentLoca;
	this->currentLoca = nullptr;

	for (int i = 0; i < userOpt->freqOption.frequencyBands.size(); i++)
	{
		frequency currentFrequencyBand = frequency(userOpt->freqOption.frequencyBands[i]);
		LOCA::checkShannonCompliance(myeegContainer->sampInfo.samplingFrequency, currentFrequencyBand);
		if (userOpt->anaOption[idCurrentLoca].anaOpt[i].eeg2env)
		{
			myeegContainer->ToHilbert(i, currentFrequencyBand.freqBandValue);
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			if (myeegContainer->triggEeg != nullptr)
				createPosFile(myeegContainer);
		}
	}
}

void InsermLibrary::LOCA::checkShannonCompliance(int p_samplingFrequency, frequency & p_freq)
{
	if (p_freq.freqBandValue[p_freq.freqBandValue.size() - 1] > (p_samplingFrequency / 2))
	{
		int step = p_freq.freqBandValue[1] - p_freq.freqBandValue[0];
		int oldfMax = p_freq.freqBandValue[p_freq.freqBandValue.size() - 1];
		int fMin = p_freq.freqBandValue[0];
		int fMax = ((p_samplingFrequency / 2) / step) * step;

		p_freq.freqBandValue.clear();
		for (int i = 0; i <= ((fMax - fMin) / step); i++)
			p_freq.freqBandValue.push_back(fMin + (i * step));

		p_freq.freqFolderName = "";
		p_freq.freqFolderName.append("f").append(to_string(fMin)).append("f").append(to_string(fMax));
	}
}

void InsermLibrary::LOCA::toBeNamedCorrectlyFunction(eegContainer *myeegContainer, int idCurrentFreqfrequency, 
													 string freqFolder, analysisOption a)
{
	bool env = false, bar = false, trial = false;

	vector<PROV> provFiles = loadProvCurrentLoca();
	for (int i = 0; i < provFiles.size(); i++)
	{
		if (provFiles[i].changeCodeFilePath != "")
			renameTriggers(myeegContainer->triggEeg, myeegContainer->triggEegDownsampled, &provFiles[i]);
		createPosFile(myeegContainer); 
		createConfFile(myeegContainer);
		processEventsDown(myeegContainer, &provFiles[i]);

		if (provFiles[i].invertmapsinfo != "")
			swapStimResp(triggCatEla, &provFiles[i]);

		if (a.env2plot)//env2plot
		{
			if (shouldPerformBarPlot(currentLoca->localizerName()) || isBarPlot(provFiles[i].filePath()))
			{
				barplot(myeegContainer, idCurrentFreqfrequency, &provFiles[i], freqFolder);
				emit incrementAdavnce(provFiles.size());
			}
			else
			{
				if (provFiles[i].invertmapsinfo == "")
				{
					env2plot(myeegContainer, idCurrentFreqfrequency, &provFiles[i], freqFolder);
					emit incrementAdavnce(provFiles.size());
				}
			}
		}

		if (a.trialmat && (isBarPlot(provFiles[i].filePath()) == false || provFiles.size() == 1))
		{
			timeTrialmatrices(myeegContainer, idCurrentFreqfrequency, &provFiles[i], freqFolder);
			emit incrementAdavnce(provFiles.size());
		}
	}
}

/**************************************************/
/* Elan Compatibility							  */
/*   - Pos File : Events from eeg Data			  */
/*	 - Conf File								  */
/*	 - Rename Trigger : For visualisation purpose */
/**************************************************/
void InsermLibrary::LOCA::createPosFile(eegContainer *myeegContainer)
{
	ofstream posFile(myeegContainer->originalFilePath + ".pos", ios::out);
	ofstream posFileX(myeegContainer->originalFilePath + "_ds" + to_string(myeegContainer->sampInfo.downsampFactor) + ".pos", ios::out);

	for (int i = 0; i < myeegContainer->triggEeg->triggers.size(); i++)
	{
		posFile << myeegContainer->triggEeg->triggers[i].trigger.sample << setw(10) 
				<< myeegContainer->triggEeg->triggers[i].trigger.code << setw(10) << "0" << endl;
		posFileX << myeegContainer->triggEegDownsampled->triggers[i].trigger.sample << setw(10)
				 << myeegContainer->triggEegDownsampled->triggers[i].trigger.code << setw(10) << "0" << endl;
	}

	posFile.close();
	posFileX.close();
}

void InsermLibrary::LOCA::createConfFile(eegContainer *myeegContainer)
{
	ofstream confFile(myeegContainer->originalFilePath + ".conf", ios::out);

	confFile << "nb_channel" << "  " << myeegContainer->flatElectrodes.size() << endl;
	confFile << "sec_per_page" << "  " << 4 << endl;
	confFile << "amp_scale_type" << "  " << 1 << endl;
	confFile << "amp_scale_val" << "  " << 1 << endl;

	confFile << "channel_visibility" << endl;
	for (int i = 0; i < myeegContainer->flatElectrodes.size(); i++)
	{
		confFile << 1 << endl;
	}
	/**********************************************************/
	/* We print the bipole Id as long as the next is from the */
	/* same Electrode : A'1, A'2, A'3 ... then when the next  */
	/* one is not from the same Electrode (B'1) we put -1 and */
	/*				we go again until the end				  */
	/**********************************************************/
	confFile << "channel_reference" << endl;
	confFile << "-1" << endl;

	for (int i = 0; i < myeegContainer->BipoleCount() - 1; i++)
	{
		if (myeegContainer->Bipole(i).second + 1 != myeegContainer->Bipole(i + 1).second)
		{
			//-1
			confFile << myeegContainer->Bipole(i).second << endl;
			confFile << "-1" << endl;
		}
		else
		{
			confFile << myeegContainer->Bipole(i).second << endl;
		}
	}
	confFile.close();
}

void InsermLibrary::LOCA::renameTriggers(TRIGGINFO *eegTriggers, TRIGGINFO *downsampledEegTriggers,
	PROV *myprovFile)
{
	stringstream buffer;
	ifstream provFile(myprovFile->changeCodeFilePath, ios::binary);
	if (provFile)
	{
		buffer << provFile.rdbuf();
		provFile.close();
	}
	else
	{
		cout << " Error opening Change Code File @ " << myprovFile->changeCodeFilePath << endl;
	}

	vector<int> oldMainCode, oldSecondaryCode, newMainCode, newSecondaryCode;
	vector<string> lineSplit = InsermLibrary::split<string>(buffer.str(), "\r\n");
	for (int i = 0; i < lineSplit.size(); i++)
	{
		vector<string> elementSplit = InsermLibrary::split<string>(lineSplit[i], "+=");
		oldMainCode.push_back(atoi(&(elementSplit[0])[0]));
		oldSecondaryCode.push_back(atoi(&(elementSplit[1])[0]));
		newMainCode.push_back(atoi(&(elementSplit[2])[0]));
		newSecondaryCode.push_back(atoi(&(elementSplit[3])[0]));
	}
	
	for (int k = 0; k < downsampledEegTriggers->triggers.size(); k++)
	{
		int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;
		int dd = -1;
		int idcode = -1;

		for (int l = 0; l < oldMainCode.size(); l++)
		{
			if (downsampledEegTriggers->triggers[k].trigger.code == oldMainCode[l])
			{
				idMain = k;
				for (int m = 0; m < myprovFile->visuBlocs.size(); m++)
				{
					if(newMainCode[l] == myprovFile->visuBlocs[m].mainEventBloc.eventCode[0])
						idVisuBloc = m;
				}
			}
		}

		if (idMain != -1)
		{
			int winSamMin = round((64 * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMin()) / 1000);
			int winSamMax = round((64 * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMax()) / 1000);

			dd = k + 1;

			while (idSec == -1 && dd < downsampledEegTriggers->triggers.size() - 1)
			{
				for (int l = 0; l < oldMainCode.size(); l++)
				{
					if (downsampledEegTriggers->triggers[dd].trigger.code == oldSecondaryCode[l] && 
						downsampledEegTriggers->triggers[idMain].trigger.code == oldMainCode[l])
					{
						idSec = dd;
						idcode = l;
					}
					else if (downsampledEegTriggers->triggers[dd].trigger.code == oldMainCode[l] && idSec == -1)
					{
						idMain = dd;
						idcode = l;
					}
				}
				dd++;
			}


			if (idMain != -1 && idSec != -1)
			{
				int winMax = downsampledEegTriggers->triggers[idMain].trigger.sample + winSamMax;
				int winMin = downsampledEegTriggers->triggers[idMain].trigger.sample - abs(winSamMin);

				bool isInWindow = (downsampledEegTriggers->triggers[idSec].trigger.sample < winMax) && 
								  (downsampledEegTriggers->triggers[idSec].trigger.sample > winMin);
				bool specialLoca = (currentLoca->localizerName() == "MARA" ||
									currentLoca->localizerName() == "MARM" ||
									currentLoca->localizerName() == "MARD");
				if (isInWindow || specialLoca)
				{
					eegTriggers->triggers[idMain].trigger.code = newMainCode[idcode];
					downsampledEegTriggers->triggers[idMain].trigger.code = newMainCode[idcode];
					eegTriggers->triggers[idSec].trigger.code = newSecondaryCode[idcode];
					downsampledEegTriggers->triggers[idSec].trigger.code = newSecondaryCode[idcode];
				}
			}
		}

	}

}

/******************************************/
/* Trigger manipulation for visualisation */
/******************************************/
void InsermLibrary::LOCA::processEvents(eegContainer *myeegContainer, PROV *myprovFile)
{
	TRIGGINFO *downEegTriggers = new TRIGGINFO(myeegContainer->triggEeg);
	pairStimResp(downEegTriggers, myprovFile);
	deleteUnsignificativEvents(downEegTriggers, myprovFile);

	triggCatEla2 = new TRIGGINFO(downEegTriggers);
	sortTrials(triggCatEla2, myprovFile, myeegContainer->sampInfo.downsampledFrequency);

	deleteAndNullify1D(triggCatEla2);
	deleteAndNullify1D(downEegTriggers);
}

void InsermLibrary::LOCA::processEventsDown(eegContainer *myeegContainer, PROV *myprovFile)
{
	TRIGGINFO *downEegTriggers = new TRIGGINFO(myeegContainer->triggEegDownsampled);
	pairStimResp(downEegTriggers, myprovFile);
	deleteUnsignificativEvents(downEegTriggers, myprovFile);

	if (myprovFile->getSecondaryCodes()[0][0] != 0) //At this point , if there is secondary code, we need to check if all have been paired correctly 
	{
		vector<int> idToDel;
		for (int i = 0; i < downEegTriggers->triggers.size(); i++)
		{
			if (downEegTriggers->triggers[i].response.code == -1)
			{
				idToDel.push_back(i);
			}
		}

		for (int i = idToDel.size() - 1; i >= 0; i--)
			downEegTriggers->triggers.erase(downEegTriggers->triggers.begin() + idToDel[i]);
	}

	triggCatEla2 = new TRIGGINFO(downEegTriggers);
	sortTrials(triggCatEla2, myprovFile, myeegContainer->sampInfo.downsampledFrequency);

	deleteAndNullify1D(triggCatEla2);
	deleteAndNullify1D(downEegTriggers);
}

void InsermLibrary::LOCA::pairStimResp(TRIGGINFO *downsampledEegTriggers, PROV *myprovFile)
{
	vector<int> mainEventsCode = myprovFile->getMainCodes();
	vector<vector<int>> respEventsCode = myprovFile->getSecondaryCodes();
	
	int idVisuBloc = 0;
	for (int k = 0; k < downsampledEegTriggers->triggers.size(); k++)
	{
		int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;
		int dd = -1;
		int idcode = -1;

		for (int l = 0; l < mainEventsCode.size(); l++)
		{
			if (downsampledEegTriggers->triggers[k].trigger.code == mainEventsCode[l])
			{
				idMain = k;
				for (int m = 0; m < myprovFile->visuBlocs.size(); m++)
				{
					if (mainEventsCode[l] == myprovFile->visuBlocs[m].mainEventBloc.eventCode[0])
						idVisuBloc = m;
				}
			}
		}

		if (idMain != -1)
		{
	
			int winSamMin = round((64 * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMin()) / 1000);
			int winSamMax = round((64 * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMax()) / 1000);

			dd = k + 1;

			while (idSec == -1 && dd < downsampledEegTriggers->triggers.size() - 1)
			{
				for (int l = 0; l < mainEventsCode.size(); l++)
				{
					for (int m = 0; m < respEventsCode[l].size(); m++)
					{
						if (downsampledEegTriggers->triggers[dd].trigger.code == respEventsCode[l][m] &&
							downsampledEegTriggers->triggers[idMain].trigger.code == mainEventsCode[l])
						{
							idSec = dd;
							idcode = l;
						}
						else if (downsampledEegTriggers->triggers[dd].trigger.code == mainEventsCode[l] && idSec == -1)
						{
							idMain = dd;
							idcode = l;
						}
					}
				}
				dd++;
			}


			if (idMain != -1 && idSec != -1)
			{
				int winMax = downsampledEegTriggers->triggers[idMain].trigger.sample + winSamMax;
				int winMin = downsampledEegTriggers->triggers[idMain].trigger.sample - abs(winSamMin);

				bool isInWindow = (downsampledEegTriggers->triggers[idSec].trigger.sample < winMax) &&
								  (downsampledEegTriggers->triggers[idSec].trigger.sample > winMin);
				bool specialLoca = (currentLoca->localizerName() == "MARA" ||
									currentLoca->localizerName() == "MARM" ||
									currentLoca->localizerName() == "MARD");
				if (isInWindow)// || specialLoca)
				{
					downsampledEegTriggers->triggers[idMain].response.code = downsampledEegTriggers->triggers[idSec].trigger.code;
					downsampledEegTriggers->triggers[idMain].response.sample = downsampledEegTriggers->triggers[idSec].trigger.sample;
				}
				else
				{
					if (specialLoca)
					{
						downsampledEegTriggers->triggers[idMain].response.code = downsampledEegTriggers->triggers[idSec].trigger.code;
						downsampledEegTriggers->triggers[idMain].response.sample = winMax;
					}
				}
			}
		}
	}
}

//Delete code that are not defined in prov file 
void InsermLibrary::LOCA::deleteUnsignificativEvents(TRIGGINFO *downsampledEegTriggers, PROV *myprovFile)
{
	vector<int> idToKeep, idToDelete;
	for (int i = 0; i < downsampledEegTriggers->triggers.size(); i++)
	{
		bool keepTrigger = false;
		for (int j = 0; j < myprovFile->visuBlocs.size(); j++)
		{
			for (int k = 0; k < myprovFile->visuBlocs[j].mainEventBloc.eventCode.size(); k++)
			{
				if (downsampledEegTriggers->triggers[i].trigger.code ==
					myprovFile->visuBlocs[j].mainEventBloc.eventCode[k])
				{
					keepTrigger = true;
				}
			}
		}

		if (keepTrigger)
			idToKeep.push_back(i);
		else
			idToDelete.push_back(i);
	}

	for (int i = idToDelete.size() - 1; i >= 0; i--)
		downsampledEegTriggers->triggers.erase(downsampledEegTriggers->triggers.begin() + idToDelete[i]);
}

void InsermLibrary::LOCA::sortTrials(TRIGGINFO *eegTriggersTemp, PROV *myprovFile, int downSampFreq)
{
	enum sortingChoice { CodeSort = 'C', LatencySort = 'L' };

	vector<int> dataId;
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
		for (int j = 0; j < eegTriggersTemp->triggers.size(); j++)
		{
			if (eegTriggersTemp->triggers[j].trigger.code == myprovFile->visuBlocs[i].mainEventBloc.eventCode[0])
			{
				dataId.push_back(j);
			}
		}
	}

	deleteAndNullify1D(triggCatEla);
	triggCatEla = new TRIGGINFO(eegTriggersTemp, dataId);
	for (int i = 0; i < triggCatEla->triggers.size(); i++)
	{
		triggCatEla->triggers[i].trigger.time = 1000 * ((float)triggCatEla->triggers[i].trigger.sample / downSampFreq);

		if(triggCatEla->triggers[i].response.sample != -1)
			triggCatEla->triggers[i].response.time = 1000 * ((float)triggCatEla->triggers[i].response.sample / downSampFreq);

		if (triggCatEla->triggers[i].response.sample != -1 && triggCatEla->triggers[i].trigger.sample != -1)
		{
			triggCatEla->triggers[i].rtSample = triggCatEla->triggers[i].response.sample - triggCatEla->triggers[i].trigger.sample;
			triggCatEla->triggers[i].rtMs = 1000 * ((float)triggCatEla->triggers[i].rtSample / downSampFreq);
		}
	}

	//get first id of each new main code
	triggCatEla->subGroupStimTrials.push_back(0);
	vector<int> mainEventsCode = myprovFile->getMainCodes();
	for (int i = 0; i < triggCatEla->triggers.size() - 1; i++)
	{
		if (triggCatEla->triggers[i].trigger.code != triggCatEla->triggers[i + 1].trigger.code)
		{
			triggCatEla->subGroupStimTrials.push_back(i + 1);
		}
	}
	triggCatEla->subGroupStimTrials.push_back(triggCatEla->triggers.size());

	//according to the rest sort by what is wanted
	int beg = 0, end = 0;
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
		string currentSort = myprovFile->visuBlocs[i].dispBloc.sort();
		vector<string> sortSplited = split<string>(currentSort, "_");
		for (int j = 1; j < sortSplited.size(); j++)
		{
			sortingChoice Choice = (sortingChoice)(sortSplited[j][0]);
			switch (Choice)
			{
			//Sort by resp code
			case CodeSort:	
								beg = triggCatEla->subGroupStimTrials[i];
								end = triggCatEla->subGroupStimTrials[i + 1];
								sort(triggCatEla->triggers.begin() + beg, triggCatEla->triggers.begin() + end,
									[](TRIGG a, TRIGG b) {
									return (a.response.code < b.response.code);
								});
				break;
			//Sort by reaction time
			case LatencySort:  
								beg = triggCatEla->subGroupStimTrials[i];
								end = triggCatEla->subGroupStimTrials[i + 1];
								sort(triggCatEla->triggers.begin() + beg, triggCatEla->triggers.begin() + end,
									[](TRIGG a, TRIGG b) {
									return (a.rtMs < b.rtMs);
								});
				break;
			}
		}
	}
}

//First we need to check if all trigg have found a resp , otherwise delete
void InsermLibrary::LOCA::swapStimResp(TRIGGINFO *eegTriggers, PROV *myprovFile)
{
	for (int i = 0; i < eegTriggers->triggers.size(); i++)
	{
		swap(eegTriggers->triggers[i].trigger, eegTriggers->triggers[i].response);
		eegTriggers->triggers[i].rtSample = -eegTriggers->triggers[i].rtSample;
		eegTriggers->triggers[i].rtMs = -eegTriggers->triggers[i].rtMs;
	}

	//This is the new window to visualize data
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
		myprovFile->visuBlocs[i].dispBloc.window(myprovFile->invertmaps.epochWindow[0], myprovFile->invertmaps.epochWindow[1]);
		myprovFile->visuBlocs[i].dispBloc.baseLine(myprovFile->invertmaps.baseLineWindow[0], myprovFile->invertmaps.baseLineWindow[1]);
	}
}

/*********************************/
/* Various check before analysis */
/*********************************/
string InsermLibrary::LOCA::createIfFreqFolderExistNot(eegContainer *myeegContainer, frequency currentFreq)
{
	string fMin = to_string(currentFreq.freqBandValue[0]);
	string fMax = to_string(currentFreq.freqBandValue[currentFreq.freqBandValue.size() - 1]);
	string freqFolder = myeegContainer->originalFilePath + "_f" + fMin + "f" + fMax + "/";

	if (!QDir(&freqFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for" + fMin + " -> " + fMax + " Hz data"));
		QDir().mkdir(&freqFolder.c_str()[0]);
	}

	return freqFolder;
}

vector<PROV> InsermLibrary::LOCA::loadProvCurrentLoca()
{
	vector<PROV> provFiles;
	for (int i = 0; i < userOpt->statOption.locaWilcoxon.size(); i++)
	{
		if (userOpt->statOption.locaWilcoxon[i].contains(QString::fromStdString(currentLoca->localizerName())))
		{
			provFiles.push_back(PROV("./Resources/Config/Prov/" + userOpt->statOption.locaWilcoxon[i].toStdString() + ".prov"));
		}
	}

	for (int i = 0; i < userOpt->statOption.locaKruskall.size(); i++)
	{
		if (userOpt->statOption.locaKruskall[i].contains(QString::fromStdString(currentLoca->localizerName())))
		{
			provFiles.push_back(PROV("./Resources/Config/Prov/" + userOpt->statOption.locaKruskall[i].toStdString() + ".prov"));
		}
	}

	if (provFiles.size() == 0) //We search if .prov with the loca name and no stat requierement
	{
		provFiles.push_back(PROV("./Resources/Config/Prov/" + currentLoca->localizerName() + ".prov"));
	}

	return provFiles;
}

bool InsermLibrary::LOCA::shouldPerformBarPlot(string locaName)
{
	for (int i = 0; i < userOpt->statOption.locaKruskall.size(); i++)
	{
		if (userOpt->statOption.locaKruskall[i].contains(QString::fromStdString(currentLoca->localizerName())))
		{
			return true;
		}
	}

	return false;
}

bool InsermLibrary::LOCA::isBarPlot(string provFile)
{
	vec1<string> splitFile = split<string>(provFile, "_");
	string toCheck = splitFile[splitFile.size() - 1];
	transform(toCheck.begin(), toCheck.end(), toCheck.begin(), ::tolower);
	if (toCheck == "_barplot")
	{
		return true;
	}
	else
	{
		return false;
	}
}

/************/
/* Barplot  */
/************/
void InsermLibrary::LOCA::barplot(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, 
								  string freqFolder)
{
	string mapsFolder = getMapsFolderBar(freqFolder, myprovFile);
	string mapPath = prepareFolderAndPathsBar(mapsFolder, myeegContainer->sampInfo.downsampFactor);
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->sampInfo.downsampledFrequency);

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(triggCatEla->triggers.size(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, idCurrentFreqfrequency, 0, triggCatEla, windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = calculateStatisticKruskall(eegData3D, myeegContainer, myprovFile, mapsFolder);

	//==
	drawBars b = drawBars(myprovFile, mapPath, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(eegData3D, triggCatEla, significantValue, myeegContainer);

	delete windowSam;
}

string InsermLibrary::LOCA::getMapsFolderBar(string freqFolder, PROV *myprovFile)
{
	string mapsFolder = freqFolder;
	vec1<string> dd = split<string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	mapsFolder.append("_bar");

	stringstream streamPValue;
	streamPValue << fixed << setprecision(2) << userOpt->statOption.pKruskall;
	if (userOpt->statOption.kruskall)
	{
		if (userOpt->statOption.FDRkruskall)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	vector<string> myProv = split<string>(myprovFile->filePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

string InsermLibrary::LOCA::prepareFolderAndPathsBar(string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<string> dd = split<string>(mapsFolder, "/");

	return string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_bar_");
}

vec1<PVALUECOORD> InsermLibrary::LOCA::calculateStatisticKruskall(vec3<float> &bigData, eegContainer *myeegContainer,
																  PROV *myprovFile, string freqFolder)
{
	vector<PVALUECOORD> significantValue;
	if (userOpt->statOption.kruskall)
	{
		int copyIndex = 0;
		vec3<float> pValue3D; vec3<int> psign3D;
		Stats::pValuesKruskall(pValue3D, psign3D, bigData, triggCatEla, myeegContainer->sampInfo.downsampledFrequency, myprovFile);
		if (userOpt->statOption.FDRkruskall)
		{
			significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, userOpt->statOption.pKruskall);
		}
		else
		{
			significantValue = Stats::loadPValues(pValue3D, psign3D, userOpt->statOption.pKruskall);
		}
		Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, true);
	}

	return significantValue;
}

/************/
/* Env2Plot */
/************/
void InsermLibrary::LOCA::env2plot(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, 
								   string freqFolder)
{
	string mapsFolder = getMapsFolderPlot(freqFolder, myprovFile);
	string mapPath = prepareFolderAndPathsPlot(mapsFolder, myeegContainer->sampInfo.downsampFactor);
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->sampInfo.downsampledFrequency);

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(triggCatEla->triggers.size(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, idCurrentFreqfrequency, 0, triggCatEla, windowSam);

	//==
	drawPlots b = drawPlots(myprovFile, mapPath, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(eegData3D, triggCatEla, myeegContainer, 2);

	delete windowSam;
}

string InsermLibrary::LOCA::getMapsFolderPlot(string freqFolder, PROV *myprovFile)
{
	string mapsFolder = freqFolder;
	vec1<string> dd = split<string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	vector<string> myProv = split<string>(myprovFile->filePath(), "/");
	return mapsFolder.append("_plots").append(" - " + myProv[myProv.size() - 1]);
}

string InsermLibrary::LOCA::prepareFolderAndPathsPlot(string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<string> dd = split<string>(mapsFolder, "/");

	return string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_plot_");
}

/************/
/* TrialMat */
/************/
void InsermLibrary::LOCA::timeTrialmatrices(eegContainer *myeegContainer, int idCurrentFreqfrequency, PROV *myprovFile, 
											string freqFolder)
{
	vector<PVALUECOORD> significantValue;
	//== get some useful information
	string mapsFolder = getMapsFolderTrial(myprovFile, freqFolder);
	string mapPath = prepareFolderAndPathsTrial(mapsFolder, myeegContainer->sampInfo.downsampFactor);
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->sampInfo.downsampledFrequency);
	int nbCol = myprovFile->nbCol();
	int nbRow = myprovFile->nbRow();

	//== get Bloc of eeg data we want to display center around events
	vec3<float> bigData;
	bigData.resize(myeegContainer->BipoleCount(), vec2<float>(triggCatEla->triggers.size(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocData(bigData, idCurrentFreqfrequency, 0, triggCatEla, windowSam);

	//== calculate stat
	if (shouldPerformStatTrial(currentLoca->localizerName()))
		significantValue = calculateStatisticWilcoxon(bigData, myeegContainer, myprovFile, mapsFolder);

	//== Draw for each plot and according to a template to reduce drawing time
	mapsGenerator mGen(userOpt->picOption.sizeTrialmap.width(), userOpt->picOption.sizeTrialmap.height());
	mGen.trialmatTemplate(triggCatEla->subGroupStimTrials, myprovFile);

	QPixmap *pixmapChanel = nullptr, *pixmapSubSubMatrix = nullptr;
	QPainter *painterChanel = nullptr, *painterSubSubMatrix = nullptr;
	for (int i = 0; i < myeegContainer->elanFrequencyBand[idCurrentFreqfrequency][0]->ElectrodeCount(); i++)
	{
		deleteAndNullify1D(painterChanel);
		deleteAndNullify1D(pixmapChanel);
		pixmapChanel = new QPixmap(mGen.pixmapTemplate);
		painterChanel = new QPainter(pixmapChanel);

		float stdRes = stdMean(bigData[i], windowSam);
		float Maxi = 2.0 * abs(stdRes);
		float Mini = -2.0 * abs(stdRes);
		mGen.graduateColorBar(painterChanel, Maxi);

		int SUBMatrixWidth = mGen.MatrixRect.width() / nbCol;
		int interpolFactorX = userOpt->picOption.interpolationtrialmap.width();	
		int interpolFactorY = userOpt->picOption.interpolationtrialmap.height();

		int indexPos = 0;
		for (int j = 0; j < myprovFile->visuBlocs.size(); j++)
		{
			int *currentWinMs = myprovFile->getWindowMs(j);
			int *currentWinSam = myprovFile->getWindowSam(myeegContainer->sampInfo.downsampledFrequency, j);
			int nbSampleWindow = currentWinSam[1] - currentWinSam[0];
			int indexBegTrigg = triggCatEla->subGroupStimTrials[j];
			int numberSubTrial = triggCatEla->subGroupStimTrials[j + 1] - indexBegTrigg;
			int subsubMatrixHeigth = 0;

			vec1<int> colorX[512], colorY[512];
			if (interpolFactorX > 1)
			{
				vec2<float> dataInterpolatedHoriz = mGen.horizontalInterpolation(bigData[i], interpolFactorX,
																				 indexBegTrigg, numberSubTrial);
				vec2<float> dataInterpolatedVerti = mGen.verticalInterpolation(dataInterpolatedHoriz, interpolFactorY);
				mGen.eegData2ColorMap(colorX, colorY, dataInterpolatedVerti, Maxi);
				subsubMatrixHeigth = interpolFactorY * (numberSubTrial - 1);
			}
			else
			{
				mGen.eegData2ColorMap(colorX, colorY, bigData[i], Maxi);
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

			for (int k = 0; k < 512; k++)
			{
				painterSubSubMatrix->setPen(QColor(mGen.ColorMapJet[k].red(), mGen.ColorMapJet[k].green(), mGen.ColorMapJet[k].blue()));
				for (int l = 0; l < colorX[k].size(); l++)
				{
					painterSubSubMatrix->drawPoint(QPoint(colorX[k][l], subsubMatrixHeigth - colorY[k][l]));
				}
			}

			indexPos = myprovFile->visuBlocs.size() - 1 - j;
			painterChanel->drawPixmap(mGen.subMatrixes[indexPos].x(), mGen.subMatrixes[indexPos].y(),
				pixmapSubSubMatrix->scaled(QSize(mGen.subMatrixes[indexPos].width(), mGen.subMatrixes[indexPos].height()),
					Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));

			/********************************************************************/
			/*	 		Add reaction time on map								*/
			/*	=> If one has not the default value, they should all be there	*/
			/********************************************************************/
			if (triggCatEla->triggers[indexBegTrigg + 2].rtMs != -1)
			{
				painterChanel->setPen(QColor(Qt::black));
				for (int l = 0; l < numberSubTrial; l++)
				{
					int xReactionTimeMs = abs(currentWinMs[0]) + triggCatEla->triggers[indexBegTrigg + l].rtMs;
					double xScale = (double)(currentWinMs[1] - currentWinMs[0]) / mGen.MatrixRect.width();
					double xRt = mGen.MatrixRect.x() + (xReactionTimeMs / xScale);

					int yTrialPosition = mGen.subMatrixes[indexPos].y() + mGen.subMatrixes[indexPos].height();
					double yRt = yTrialPosition - (((double)mGen.subMatrixes[indexPos].height() / numberSubTrial) * l) - 1;

					if (xRt <= (mGen.MatrixRect.x() + mGen.MatrixRect.width()))
					{
						painterChanel->setBrush(Qt::black);
						painterChanel->drawEllipse(QPoint(xRt, yRt), (int)(0.0034722 * mGen.MatrixRect.width()),
												  (int)(0.004629629 * mGen.MatrixRect.height()));
					}
				}
			}

			delete currentWinMs;
			delete currentWinSam;
		}

		mGen.drawVerticalZeroLine(painterChanel, myprovFile);

		//Display Stat
		if (userOpt->statOption.wilcoxon)
		{
			vector<int> allIdCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
			if (allIdCurrentMap.size() > 0)
			{
				vec2<int> idCurrentMap = mGen.checkIfConditionStat(significantValue, allIdCurrentMap, nbRow);
				mGen.displayStatsOnMap(painterChanel, idCurrentMap, significantValue, myprovFile);
			}
		}

		//Display title on map and then Save
		string outputPicPath = mapPath;
		string elecName = myeegContainer->flatElectrodes[myeegContainer->Bipole(i).first];
		outputPicPath.append(elecName.c_str()).append(".jpg");
		mGen.drawMapTitle(painterChanel, outputPicPath);
		pixmapChanel->save(outputPicPath.c_str(), "JPG");
	}

	delete windowSam;
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);
	deleteAndNullify1D(painterSubSubMatrix);
	deleteAndNullify1D(pixmapSubSubMatrix);

	emit sendLogInfo("Time Trials Matrices generated");
}

string InsermLibrary::LOCA::getMapsFolderTrial(PROV *myprovFile, string freqFolder)
{
	string mapsFolder = freqFolder;
	vec1<string> dd = split<string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	if (myprovFile->invertmapsinfo == "")
		mapsFolder.append("_trials_stim");
	else
		mapsFolder.append("_trials_resp");

	stringstream streamPValue;
	streamPValue << fixed << setprecision(2) << userOpt->statOption.pWilcoxon;
	if (userOpt->statOption.wilcoxon)
	{
		if (userOpt->statOption.FDRwilcoxon)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	vector<string> myProv = split<string>(myprovFile->filePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

string InsermLibrary::LOCA::prepareFolderAndPathsTrial(string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<string> dd = split<string>(mapsFolder, "/");

	return string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_trials_");
}

bool InsermLibrary::LOCA::shouldPerformStatTrial(string locaName)
{
	for (int i = 0; i < userOpt->statOption.locaWilcoxon.size(); i++)
	{
		if (userOpt->statOption.locaWilcoxon[i].contains(QString::fromStdString(currentLoca->localizerName())))
		{
			return true;
		}
	}

	return false;
}

vec1<PVALUECOORD> InsermLibrary::LOCA::calculateStatisticWilcoxon(vec3<float> &bigData, eegContainer *myeegContainer,
															PROV *myprovFile, string freqFolder)
{
	vector<PVALUECOORD> significantValue;
	if (userOpt->statOption.wilcoxon)
	{
		int copyIndex = 0;
		vec3<float> pValue3D; vec3<int> psign3D;
		Stats::pValuesWilcoxon(pValue3D, psign3D, bigData, triggCatEla, myeegContainer->sampInfo.downsampledFrequency, myprovFile);
		if (userOpt->statOption.FDRwilcoxon)
		{
			significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, userOpt->statOption.pWilcoxon);
		}
		else
		{
			significantValue = Stats::loadPValues(pValue3D, psign3D, userOpt->statOption.pWilcoxon);
		}
		Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, false);
	}

	return significantValue;
}


/****************************************************************/
/*	Standard Derivation => mean then sqrt(sum((data-mean)²))	*/
/****************************************************************/
float InsermLibrary::LOCA::stdMean(vec2<float> eegDataChanel, int windowSam[2])
{
	int nbTrigg = eegDataChanel.size();
	int nbSampleWin = windowSam[1] - windowSam[0];
	vec1<float> erp = vec1<float>(nbTrigg);
	vec1<float> stdDeviation = vec1<float>(nbTrigg);

	for (int i = 0; i < nbTrigg; i++)
	{
		double tempErp = 0;
		for (int m = 0; m < nbSampleWin; m++)
		{
			tempErp += eegDataChanel[i][m];
		}
		erp[i] = tempErp / nbSampleWin;
	}

	for (int i = 0; i < nbTrigg; i++)
	{
		double tempStd = 0;
		for (int m = 0; m < nbSampleWin; m++)
		{
			tempStd += (eegDataChanel[i][m] - erp[i]) * (eegDataChanel[i][m] - erp[i]);
		}
		stdDeviation[i] = sqrt(tempStd / (nbSampleWin - 1));
	}

	double tempstd = 0;
	for (int k = 0; k < nbTrigg; k++)
	{
		tempstd += stdDeviation[k];
	}
	tempstd /= nbTrigg;

	return tempstd;
}
