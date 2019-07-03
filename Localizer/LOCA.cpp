#include "LOCA.h"

using namespace Framework::Calculations::Stats;

InsermLibrary::LOCA::LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOption)
{
	m_analysisOpt = analysisOpt;
	m_statOption = statOption;
	m_picOption = picOption;
}

InsermLibrary::LOCA::~LOCA()
{
	EEGFormat::Utility::DeleteAndNullify(m_currentLoca);
	EEGFormat::Utility::DeleteAndNullify(m_triggerContainer);
	EEGFormat::Utility::DeleteAndNullify(m_statOption);
	EEGFormat::Utility::DeleteAndNullify(m_picOption);
}

/************/
/* eeg2erp  */
/************/
void InsermLibrary::LOCA::eeg2erp(eegContainer *myeegContainer, PROV *myprovFile)
{
	int *windowSam = myprovFile->getBiggestWindowSam(myeegContainer->SamplingFrequency());

	std::string outputErpFolder = myeegContainer->RootFileFolder();
	outputErpFolder.append(myeegContainer->RootFileName());
	outputErpFolder.append("_ERP/");
	if (!QDir(&outputErpFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for erp Maps"));
		QDir().mkdir(&outputErpFolder.c_str()[0]);
	}
	string monoErpOutput = outputErpFolder.append(myeegContainer->RootFileName());

	deleteAndNullify1D(m_triggerContainer);
	m_triggerContainer = new TriggerContainer(myeegContainer->Triggers(), myeegContainer->SamplingFrequency());
	m_triggerContainer->ProcessEventsForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());

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

	drawPlots b = drawPlots(myprovFile, monoErpOutput, m_picOption->sizePlotmap);
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
	m_idCurrentLoca = idCurrentLoca;
	m_currentLoca = currentLoca;

	for (int i = 0; i < m_analysisOpt.size(); i++)
	{
		FrequencyBand currentFrequencyBand(m_analysisOpt[i].Band);
		currentFrequencyBand.CheckShannonCompliance(myeegContainer->SamplingFrequency());

		if (m_analysisOpt[i].analysisParameters.eeg2env2)
		{
			Algorithm::AlgorithmCalculator::ExecuteAlgorithm(m_analysisOpt[i].analysisParameters.calculationType, myeegContainer, currentFrequencyBand.FrequencyBins());
			myeegContainer->SaveFrequencyData(m_analysisOpt[i].analysisParameters.outputType, currentFrequencyBand.FrequencyBins());
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			string freqFolder = createIfFreqFolderExistNot(myeegContainer, currentFrequencyBand);
			toBeNamedCorrectlyFunction(myeegContainer, freqFolder, m_analysisOpt[i]);
		}
		else 
		{
			for (int j = 0; j < currentLoca->frequencyFolders().size(); j++)
			{
				string fMin = to_string(currentFrequencyBand.FMin());
				string fMax = to_string(currentFrequencyBand.FMax());
				std::vector<std::string> sm0Files = currentLoca->frequencyFolders()[j].FilePaths(SM0_ELAN);

				if ((currentLoca->frequencyFolders()[j].frequencyName() == "f" + fMin + "f" + fMax) && (sm0Files.size() > 0))
				{
					std::vector<std::string> dataFiles = currentLoca->frequencyFolders()[j].FilePaths(SM0_ELAN);
					int result = myeegContainer->LoadFrequencyData(dataFiles, 0);
					if(result == 0)
					{
						emit incrementAdavnce(1);
						emit sendLogInfo("Envelloppe File Loaded");

						string freqFolder = createIfFreqFolderExistNot(myeegContainer, currentFrequencyBand);
						toBeNamedCorrectlyFunction(myeegContainer, freqFolder, m_analysisOpt[i]);
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
	m_idCurrentLoca = idCurrentLoca;
	m_currentLoca = nullptr;

	m_triggerContainer = new TriggerContainer(myeegContainer->Triggers(), myeegContainer->SamplingFrequency());
	for (int i = 0; i < m_analysisOpt.size(); i++)
	{
		FrequencyBand currentFrequencyBand(m_analysisOpt[i].Band);
		currentFrequencyBand.CheckShannonCompliance(myeegContainer->SamplingFrequency());
		if (m_analysisOpt[i].analysisParameters.eeg2env2)
		{
			Algorithm::AlgorithmCalculator::ExecuteAlgorithm(m_analysisOpt[i].analysisParameters.calculationType, myeegContainer, currentFrequencyBand.FrequencyBins());
			myeegContainer->SaveFrequencyData(m_analysisOpt[i].analysisParameters.outputType, currentFrequencyBand.FrequencyBins());
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			CreateEventsFile(m_analysisOpt[i], myeegContainer, m_triggerContainer, nullptr);
		}
	}
	deleteAndNullify1D(m_triggerContainer);
}

void InsermLibrary::LOCA::toBeNamedCorrectlyFunction(eegContainer *myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a)
{
	m_triggerContainer = new TriggerContainer(myeegContainer->Triggers(), myeegContainer->SamplingFrequency());

	//We generate file.pos and file_dsX.pos if we find a prov file 
	//with the exact same name as the experiment.	
	PROV* mainTask = LoadProvForTask();
	if (mainTask != nullptr)
	{
		CreateEventsFile(a, myeegContainer, m_triggerContainer, mainTask);
		CreateConfFile(myeegContainer);
		EEGFormat::Utility::DeleteAndNullify(mainTask);
	}

	std::vector<PROV> provFiles = LoadAllProvForTask();
	for (int i = 0; i < provFiles.size(); i++)
	{
		m_triggerContainer->ProcessEventsForExperiment(&provFiles[i], 99);

		if (a.env2plot)
		{
			if (shouldPerformBarPlot(m_currentLoca->localizerName()) || isBarPlot(provFiles[i].filePath()))
			{
				barplot(myeegContainer, &provFiles[i], freqFolder);
				emit incrementAdavnce(provFiles.size());
			}
			else
			{
				if (provFiles[i].invertmapsinfo == "")
				{
					env2plot(myeegContainer, &provFiles[i], freqFolder);
					emit incrementAdavnce(provFiles.size());
				}
			}
		}

		if (a.trialmat && (isBarPlot(provFiles[i].filePath()) == false || provFiles.size() == 1))
		{
			timeTrialmatrices(myeegContainer, &provFiles[i], freqFolder);
			emit incrementAdavnce(provFiles.size());
		}
	}

	deleteAndNullify1D(m_triggerContainer);
}

void InsermLibrary::LOCA::CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer *myeegContainer, TriggerContainer *triggerContainer, PROV *myprovFile)
{
	std::string fileNameBase = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();

	EEGFormat::FileType outputType = analysisOpt.analysisParameters.outputType;
	switch (outputType)
	{
		case EEGFormat::FileType::Micromed:
		{
			throw std::runtime_error("Micromed File type is not allowed as an event output file");
			break;
		}
		case EEGFormat::FileType::Elan:
		{
			std::string eventFilePath = fileNameBase + ".pos";
			std::string downsampledEventsFilePath = fileNameBase + "_ds" + to_string(myeegContainer->DownsamplingFactor()) + ".pos";
			std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
			std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());
			CreateFile(outputType, eventFilePath, triggers);
			CreateFile(outputType, downsampledEventsFilePath, triggersDownsampled);
			break;
		}
		case EEGFormat::FileType::BrainVision:
		{
			std::string eventFilePath = fileNameBase + ".vmkr";
			std::string downsampledEventsFilePath = fileNameBase + "_ds" + to_string(myeegContainer->DownsamplingFactor()) + ".vmkr";
			std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
			std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());
			CreateFile(outputType, eventFilePath, triggers);
			CreateFile(outputType, downsampledEventsFilePath, triggersDownsampled);

			std::string frequencySuffix = "f" + std::to_string(analysisOpt.Band.FMin()) + "f" + std::to_string(analysisOpt.Band.FMax());
			RelinkAnalysisFileAnUglyWay(myeegContainer->RootFileFolder(), myeegContainer->RootFileName(),frequencySuffix, to_string(myeegContainer->DownsamplingFactor()));
			break;
		}
		case EEGFormat::FileType::EuropeanDataFormat:
		{
			throw std::runtime_error("European Data Format file type is not allowed as an output file");
			break;
		}
		default:
		{
			throw std::runtime_error("Output file type not recognized");
			break;
		}
	}
}

void InsermLibrary::LOCA::CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger> & triggers)
{
	std::vector<EEGFormat::ITrigger> iTriggers(triggers.size());
	for (int i = 0; i < iTriggers.size(); i++)
	{
		iTriggers[i] = EEGFormat::ITrigger(triggers[i].MainEvent());
	}

	switch (outputType)
	{
		case EEGFormat::FileType::Micromed:
		{
			throw std::runtime_error("Micromed File type is not allowed as an event output file");
			break;
		}
		case EEGFormat::FileType::Elan:
		{
			EEGFormat::ElanFile::SaveTriggers(filePath, iTriggers);
			break;
		}
		case EEGFormat::FileType::BrainVision:
		{
			EEGFormat::BrainVisionFile::SaveMarkers(filePath, iTriggers, std::vector<EEGFormat::INote>());
			break;
		}
		case EEGFormat::FileType::EuropeanDataFormat:
		{
			throw std::runtime_error("European Data Format file type is not allowed as an output file");
			break;
		}
		default:
		{
			throw std::runtime_error("Output file type not recognized");
			break;
		}
	}
}

//Since each BrainVision file offers the possiblity to link an event file and for
//each dsX_smX file we want it to point to the same marker file after creating the data
//we need to open each vhdr file and change the path by hand
void InsermLibrary::LOCA::RelinkAnalysisFileAnUglyWay(const std::string& rootPath, const std::string& fileNameBase, const std::string& frequencySuffix, const std::string& downsamplingFactor)
{
	std::string frequencyFolder = fileNameBase + "_" + frequencySuffix;

	std::vector<std::string> pathToCheck;
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm0.vhdr");
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm250.vhdr");
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm500.vhdr");
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm1000.vhdr");
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm2500.vhdr");
	pathToCheck.push_back(rootPath + frequencyFolder + "/" + frequencyFolder + "_ds" + downsamplingFactor + "_sm5000.vhdr");

	int PathCount = pathToCheck.size();
	for (int i = 0; i < PathCount; i++)
	{
		if (EEGFormat::Utility::IsValidFile(pathToCheck[i]))
		{
			std::vector<std::string> rawHeader = EEGFormat::Utility::ReadTextFile(pathToCheck[i]);
			int LineCount = rawHeader.size();
			if (LineCount == 0)
				throw std::runtime_error("Error, this file should not be empty");

			auto it = std::find_if(rawHeader.begin(), rawHeader.end(), [&](const std::string& str)
			{
				return str.find("MarkerFile=") != std::string::npos; 
			});
			
			if (it != rawHeader.end())
			{
				int id = std::distance(rawHeader.begin(), it);
				rawHeader[id] = "MarkerFile=../" + fileNameBase + "_ds" + downsamplingFactor + ".vmkr";
				
				std::ofstream markersFile(pathToCheck[i], std::ios::trunc | std::ios::binary);
				if (markersFile.is_open())
				{
					for (int j = 0; j < LineCount; j++)
					{
						markersFile << rawHeader[j] << std::endl;
					}
				}
				markersFile.close();
			}
		}
	}
}

/**************************************************/
/*		Elan Compatibility : Conf File			  */
/**************************************************/
void InsermLibrary::LOCA::CreateConfFile(eegContainer *myeegContainer)
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
std::string InsermLibrary::LOCA::createIfFreqFolderExistNot(eegContainer *myeegContainer, FrequencyBand currentFreq)
{
	std::string fMin = std::to_string(currentFreq.FMin());
	std::string fMax = std::to_string(currentFreq.FMax());
	std::string freqFolder = myeegContainer->RootFileFolder() + myeegContainer->RootFileName() + "_f" + fMin + "f" + fMax + "/";

	if (!QDir(&freqFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for" + fMin + " -> " + fMax + " Hz data"));
		QDir().mkdir(&freqFolder.c_str()[0]);
	}

	return freqFolder;
}

PROV* InsermLibrary::LOCA::LoadProvForTask()
{
	std::string MainProvPath = "./Resources/Config/Prov/" + m_currentLoca->localizerName() + ".prov";
	if (EEGFormat::Utility::DoesFileExist(MainProvPath))
	{
		return new PROV(MainProvPath);
	}
	else
	{
		return nullptr;
	}
}

std::vector<PROV> InsermLibrary::LOCA::LoadAllProvForTask()
{
	std::vector<PROV> provFiles;
	for (int i = 0; i < m_statOption->locaWilcoxon.size(); i++)
	{
		if (m_statOption->locaWilcoxon[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			std::string possibleFile = "./Resources/Config/Prov/" + m_statOption->locaWilcoxon[i].toStdString() + ".prov";
			if (EEGFormat::Utility::DoesFileExist(possibleFile))
			{
				provFiles.push_back(PROV(possibleFile));
			}
		}
	}

	for (int i = 0; i < m_statOption->locaKruskall.size(); i++)
	{
		if (m_statOption->locaKruskall[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			std::string possibleFile = "./Resources/Config/Prov/" + m_statOption->locaKruskall[i].toStdString() + ".prov";
			if (EEGFormat::Utility::DoesFileExist(possibleFile))
			{
				provFiles.push_back(PROV(possibleFile));
			}
		}
	}

	return provFiles;
}

bool InsermLibrary::LOCA::shouldPerformBarPlot(std::string locaName)
{
	for (int i = 0; i < m_statOption->locaKruskall.size(); i++)
	{
		if (m_statOption->locaKruskall[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			return true;
		}
	}

	return false;
}

bool InsermLibrary::LOCA::isBarPlot(std::string provFile)
{
	vec1<string> splitFile = split<std::string>(provFile, "_");
	std::string toCheck = splitFile[splitFile.size() - 1];
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
void InsermLibrary::LOCA::barplot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder)
{
	std::string mapsFolder = getMapsFolderBar(freqFolder, myprovFile);
	std::string mapPath = prepareFolderAndPathsBar(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = calculateStatisticKruskall(eegData3D, myeegContainer, myprovFile, mapsFolder);

	//==
	drawBars b = drawBars(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, significantValue, myeegContainer);

	delete windowSam;
}

std::string InsermLibrary::LOCA::getMapsFolderBar(std::string freqFolder, PROV *myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	mapsFolder.append("_bar");

	stringstream streamPValue;
	streamPValue << fixed << setprecision(2) << m_statOption->pKruskall;
	if (m_statOption->kruskall)
	{
		if (m_statOption->FDRkruskall)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	std::vector<std::string> myProv = split<std::string>(myprovFile->filePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

std::string InsermLibrary::LOCA::prepareFolderAndPathsBar(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_bar_");
}

vec1<PVALUECOORD> InsermLibrary::LOCA::calculateStatisticKruskall(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder)
{
	std::vector<PVALUECOORD> significantValue;
	if (m_statOption->kruskall)
	{
		int copyIndex = 0;
		vec3<float> pValue3D; vec3<int> psign3D;
		Stats::pValuesKruskall(pValue3D, psign3D, bigData, m_triggerContainer, myeegContainer->DownsampledFrequency(), myprovFile);
		if (m_statOption->FDRkruskall)
		{
			significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, m_statOption->pKruskall);
		}
		else
		{
			significantValue = Stats::loadPValues(pValue3D, psign3D, m_statOption->pKruskall);
		}
		Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, true);
	}

	return significantValue;
}

/************/
/* Env2Plot */
/************/
void InsermLibrary::LOCA::env2plot(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder)
{
	std::string mapsFolder = getMapsFolderPlot(freqFolder, myprovFile);
	std::string mapPath = prepareFolderAndPathsPlot(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//==
	drawPlots b = drawPlots(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, myeegContainer, 2);

	delete windowSam;
}

std::string InsermLibrary::LOCA::getMapsFolderPlot(std::string freqFolder, PROV *myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	vector<std::string> myProv = split<std::string>(myprovFile->filePath(), "/");
	return mapsFolder.append("_plots").append(" - " + myProv[myProv.size() - 1]);
}

std::string InsermLibrary::LOCA::prepareFolderAndPathsPlot(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_plot_");
}

/************/
/* TrialMat */
/************/
void InsermLibrary::LOCA::timeTrialmatrices(eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder)
{
	vector<PVALUECOORD> significantValue;
	//== get some useful information
	std::string mapsFolder = getMapsFolderTrial(myprovFile, freqFolder);
	std::string mapPath = prepareFolderAndPathsTrial(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());
	int nbCol = myprovFile->nbCol();
	int nbRow = myprovFile->nbRow();

	//== get Bloc of eeg data we want to display center around events
	vec3<float> bigData;
	bigData.resize(myeegContainer->BipoleCount(), vec2<float>(m_triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocData(bigData, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	if (shouldPerformStatTrial(m_currentLoca->localizerName()))
		significantValue = calculateStatisticWilcoxon(bigData, myeegContainer, myprovFile, mapsFolder);

	//== Draw for each plot and according to a template to reduce drawing time
	std::vector<int> SubGroupStimTrials = m_triggerContainer->SubGroupStimTrials();
	mapsGenerator mGen(m_picOption->sizeTrialmap.width(), m_picOption->sizeTrialmap.height());
	mGen.trialmatTemplate(SubGroupStimTrials, myprovFile);

	QPixmap *pixmapChanel = nullptr, *pixmapSubSubMatrix = nullptr;
	QPainter *painterChanel = nullptr, *painterSubSubMatrix = nullptr;
	for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
	{
		deleteAndNullify1D(painterChanel);
		deleteAndNullify1D(pixmapChanel);
		pixmapChanel = new QPixmap(mGen.pixmapTemplate);
		painterChanel = new QPainter(pixmapChanel);

		float stdRes = Measure::MeanOfStandardDeviation(bigData[i]);
		float Maxi = 2.0 * abs(stdRes);
		float Mini = -2.0 * abs(stdRes);
		mGen.graduateColorBar(painterChanel, Maxi);

		int SUBMatrixWidth = mGen.MatrixRect.width() / nbCol;
		int interpolFactorX = m_picOption->interpolationtrialmap.width();	
		int interpolFactorY = m_picOption->interpolationtrialmap.height();

		int indexPos = 0;
		for (int j = 0; j < myprovFile->visuBlocs.size(); j++)
		{
			int *currentWinMs = myprovFile->getWindowMs(j);
			int *currentWinSam = myprovFile->getWindowSam(myeegContainer->DownsampledFrequency(), j);
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
		if (m_statOption->wilcoxon)
		{
			vector<int> allIdCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
			if (allIdCurrentMap.size() > 0)
			{
				vec2<int> idCurrentMap = mGen.checkIfConditionStat(significantValue, allIdCurrentMap, nbRow);
				mGen.displayStatsOnMap(painterChanel, idCurrentMap, significantValue, myprovFile);
			}
		}

		//Display title on map and then Save
		std::string outputPicPath = mapPath;
		std::string elecName = myeegContainer->flatElectrodes[myeegContainer->Bipole(i).first];
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

std::string InsermLibrary::LOCA::getMapsFolderTrial(PROV *myprovFile, std::string freqFolder)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	if (myprovFile->invertmapsinfo == "")
		mapsFolder.append("_trials_stim");
	else
		mapsFolder.append("_trials_resp");

	std::stringstream streamPValue;
	streamPValue << fixed << setprecision(2) << m_statOption->pWilcoxon;
	if (m_statOption->wilcoxon)
	{
		if (m_statOption->FDRwilcoxon)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	std::vector<std::string> myProv = split<std::string>(myprovFile->filePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

std::string InsermLibrary::LOCA::prepareFolderAndPathsTrial(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + to_string(dsSampFreq) + "_sm0_trials_");
}

bool InsermLibrary::LOCA::shouldPerformStatTrial(std::string locaName)
{
	for (int i = 0; i < m_statOption->locaWilcoxon.size(); i++)
	{
		if (m_statOption->locaWilcoxon[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			return true;
		}
	}

	return false;
}

vec1<PVALUECOORD> InsermLibrary::LOCA::calculateStatisticWilcoxon(vec3<float> &bigData, eegContainer *myeegContainer, PROV *myprovFile, std::string freqFolder)
{
	std::vector<PVALUECOORD> significantValue;
	if (m_statOption->wilcoxon)
	{
		int copyIndex = 0;
		vec3<float> pValue3D; vec3<int> psign3D;
		Stats::pValuesWilcoxon(pValue3D, psign3D, bigData, m_triggerContainer, myeegContainer->DownsampledFrequency(), myprovFile);
		if (m_statOption->FDRwilcoxon)
		{
			significantValue = Stats::FDR(pValue3D, psign3D, copyIndex, m_statOption->pWilcoxon);
		}
		else
		{
			significantValue = Stats::loadPValues(pValue3D, psign3D, m_statOption->pWilcoxon);
		}
		Stats::exportStatsData(myeegContainer, myprovFile, significantValue, freqFolder, false);
	}

	return significantValue;
}