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

	std::string outputErpFolder = myeegContainer->RootFileFolder();
	outputErpFolder.append(myeegContainer->RootFileName());
	outputErpFolder.append("_ERP/");
	if (!QDir(&outputErpFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for erp Maps"));
		QDir().mkdir(&outputErpFolder.c_str()[0]);
	}
	string monoErpOutput = outputErpFolder.append(myeegContainer->RootFileName());
	m_triggerContainer->ProcessEventsForExperiment(myprovFile, 99, myeegContainer->sampInfo.downsampFactor);

	vec3<float> bigDataMono = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->flatElectrodes.size(), vec1<float>(windowSam[1] - windowSam[0])));
	vec3<float> bigDataBipo = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));

	cout << "Reading Data Mono ..." << endl;
	for (int i = 0; i < m_triggerContainer->ProcessedTriggerCount(); i++)
	{
		for (int j = 0; j < (windowSam[1] - windowSam[0]); j++)
		{
			for (int k = 0; k < myeegContainer->flatElectrodes.size(); k++)
			{
				int beginPos = m_triggerContainer->ProcessedTriggers()[i].MainSample() + windowSam[0];
				bigDataMono[i][k][j] = myeegContainer->Data()[k][beginPos + j];
			}
		}
	}

	cout << "Reading Data Bipo ..." << endl;
	for (int i = 0; i < m_triggerContainer->ProcessedTriggerCount(); i++)
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

	drawPlots b = drawPlots(myprovFile, monoErpOutput, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(bigDataMono, m_triggerContainer, myeegContainer, 0);
	emit sendLogInfo("Mono Maps Generated");
	b.drawDataOnTemplate(bigDataBipo, m_triggerContainer, myeegContainer, 1);
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

	deleteAndNullify1D(m_triggerContainer);
	m_triggerContainer = new TriggerContainer(myeegContainer->Triggers(), myeegContainer->sampInfo.samplingFrequency);

	for (int i = 0; i < userOpt->freqOption.frequencyBands.size(); i++)
	{
		frequency currentFrequencyBand = frequency(userOpt->freqOption.frequencyBands[i]);
		LOCA::checkShannonCompliance(myeegContainer->sampInfo.samplingFrequency, currentFrequencyBand);
		if (userOpt->anaOption[idCurrentLoca].anaOpt[i].eeg2env)
		{
			myeegContainer->ToHilbert(i, currentFrequencyBand.freqBandValue);
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			CreateEventsFile(myeegContainer, m_triggerContainer, nullptr);
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

	deleteAndNullify1D(m_triggerContainer);
	m_triggerContainer = new TriggerContainer(myeegContainer->Triggers(), myeegContainer->sampInfo.samplingFrequency);

	vector<PROV> provFiles = loadProvCurrentLoca();
	for (int i = 0; i < provFiles.size(); i++)
	{
		CreateEventsFile(myeegContainer, m_triggerContainer, &provFiles[i]);
		createConfFile(myeegContainer);
		m_triggerContainer->ProcessEventsForExperiment(&provFiles[i], 99);

		if (provFiles[i].invertmapsinfo != "")
		{
			m_triggerContainer->SwapStimulationsAndResponses(&provFiles[i]);
		}

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
void InsermLibrary::LOCA::CreateEventsFile(eegContainer *myeegContainer, TriggerContainer *triggerContainer, PROV *myprovFile)
{
	std::string fileNameBase = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();

	std::string triggersPosFilePath = fileNameBase + ".pos";
	std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
	createPosFile(triggersPosFilePath, triggers);
	std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->sampInfo.downsampFactor);
	std::string downsampledTriggersPosFilePath = fileNameBase + "_ds" + to_string(myeegContainer->sampInfo.downsampFactor) + ".pos";
	createPosFile(downsampledTriggersPosFilePath, triggersDownsampled);
}

void InsermLibrary::LOCA::createPosFile(std::string filePath, std::vector<Trigger> & triggers)
{
	std::vector<EEGFormat::ITrigger> iTriggers(triggers.size());
	for (int i = 0; i < iTriggers.size(); i++)
	{
		iTriggers[i] = EEGFormat::ITrigger(triggers[i].MainEvent());
	}
	EEGFormat::ElanFile::SaveTriggers(filePath, iTriggers);
}

void InsermLibrary::LOCA::createConfFile(eegContainer *myeegContainer)
{
	std::string outputConfFilePath = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();

	ofstream confFile(outputConfFilePath + ".conf", ios::out);

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

/*********************************/
/* Various check before analysis */
/*********************************/
string InsermLibrary::LOCA::createIfFreqFolderExistNot(eegContainer *myeegContainer, frequency currentFreq)
{
	string fMin = to_string(currentFreq.freqBandValue[0]);
	string fMax = to_string(currentFreq.freqBandValue[currentFreq.freqBandValue.size() - 1]);
	string freqFolder = myeegContainer->RootFileFolder() + myeegContainer->RootFileName() + "_f" + fMin + "f" + fMax + "/";

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
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, idCurrentFreqfrequency, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = calculateStatisticKruskall(eegData3D, myeegContainer, myprovFile, mapsFolder);

	//==
	drawBars b = drawBars(myprovFile, mapPath, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, significantValue, myeegContainer);

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
		Stats::pValuesKruskall(pValue3D, psign3D, bigData, m_triggerContainer, myeegContainer->sampInfo.downsampledFrequency, myprovFile);
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
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, idCurrentFreqfrequency, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//==
	drawPlots b = drawPlots(myprovFile, mapPath, userOpt->picOption.sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, myeegContainer, 2);

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
	bigData.resize(myeegContainer->BipoleCount(), vec2<float>(m_triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocData(bigData, idCurrentFreqfrequency, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	if (shouldPerformStatTrial(currentLoca->localizerName()))
		significantValue = calculateStatisticWilcoxon(bigData, myeegContainer, myprovFile, mapsFolder);

	//== Draw for each plot and according to a template to reduce drawing time
	std::vector<int> SubGroupStimTrials = m_triggerContainer->SubGroupStimTrials();
	mapsGenerator mGen(userOpt->picOption.sizeTrialmap.width(), userOpt->picOption.sizeTrialmap.height());
	mGen.trialmatTemplate(SubGroupStimTrials, myprovFile);

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
			int indexBegTrigg = SubGroupStimTrials[j];
			int numberSubTrial = SubGroupStimTrials[j + 1] - indexBegTrigg;
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
			if (m_triggerContainer->ProcessedTriggers()[indexBegTrigg + 2].ReactionTimeInMilliSeconds() != -1)
			{
				painterChanel->setPen(QColor(Qt::black));
				for (int l = 0; l < numberSubTrial; l++)
				{
					int xReactionTimeMs = abs(currentWinMs[0]) + m_triggerContainer->ProcessedTriggers()[indexBegTrigg + l].ReactionTimeInMilliSeconds();
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
		Stats::pValuesWilcoxon(pValue3D, psign3D, bigData, m_triggerContainer, myeegContainer->sampInfo.downsampledFrequency, myprovFile);
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
/*	Standard Derivation => mean then sqrt(sum((data-mean)�))	*/
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
