#include "LOCA.h"

//#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

InsermLibrary::LOCA::LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOption)
{
	m_analysisOpt = analysisOpt;
	m_statOption = statOption;
	m_picOption = picOption;
}

InsermLibrary::LOCA::~LOCA()
{
	EEGFormat::Utility::DeleteAndNullify(m_triggerContainer);
	EEGFormat::Utility::DeleteAndNullify(m_statOption);
	EEGFormat::Utility::DeleteAndNullify(m_picOption);
}

/************/
/* eeg2erp  */
/************/
void InsermLibrary::LOCA::Eeg2erp(eegContainer* myeegContainer, PROV* myprovFile)
{
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->SamplingFrequency());

	std::string outputErpFolder = myeegContainer->RootFileFolder();
	outputErpFolder.append(myeegContainer->RootFileName());
	outputErpFolder.append("_ERP/");
	if (!QDir(&outputErpFolder.c_str()[0]).exists())
	{
		emit sendLogInfo(QString::fromStdString("Creating Output Folder for erp Maps"));
		QDir().mkdir(&outputErpFolder.c_str()[0]);
	}
	std::string monoErpOutput = outputErpFolder.append(myeegContainer->RootFileName());

	deleteAndNullify1D(m_triggerContainer);
	std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
	int samplingFrequency = myeegContainer->SamplingFrequency();
	m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);
	m_triggerContainer->ProcessEventsForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());

	vec3<float> bigDataMono = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->flatElectrodes.size(), vec1<float>(windowSam[1] - windowSam[0])));
	vec3<float> bigDataBipo = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));

	std::cout << "Reading Data Mono ..." << std::endl;
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

	std::cout << "Reading Data Bipo ..." << std::endl;
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

	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, monoErpOutput, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(bigDataMono, m_triggerContainer, myeegContainer, 0);
	emit sendLogInfo("Mono Maps Generated");
	b.drawDataOnTemplate(bigDataBipo, m_triggerContainer, myeegContainer, 1);
	emit sendLogInfo("Bipo Maps Generated");
	emit incrementAdavnce(1);

	delete windowSam;
}

void InsermLibrary::LOCA::Localize(eegContainer* myeegContainer, int idCurrentLoca, locaFolder* currentLoca)
{
	m_idCurrentLoca = idCurrentLoca;
	m_currentLoca = currentLoca;

	int examCountToProcess = static_cast<int>(m_analysisOpt.size());
	for (int i = 0; i < examCountToProcess; i++)
	{
		FrequencyBand currentFrequencyBand(m_analysisOpt[i].Band);
		if (m_analysisOpt[i].analysisParameters.eeg2env2)
		{
			currentFrequencyBand.CheckShannonCompliance(myeegContainer->SamplingFrequency());

			Algorithm::AlgorithmCalculator::ExecuteAlgorithm(m_analysisOpt[i].analysisParameters.calculationType, myeegContainer, currentFrequencyBand.FrequencyBins());
			myeegContainer->SaveFrequencyData(m_analysisOpt[i].analysisParameters.outputType, currentFrequencyBand.FrequencyBins());
			emit incrementAdavnce(1);
			emit sendLogInfo("Hilbert Envelloppe Calculated");

			std::string freqFolder = CreateFrequencyFolder(myeegContainer, currentFrequencyBand);
			GenerateMapsAndFigures(myeegContainer, freqFolder, m_analysisOpt[i]);
		}
		else
		{
			int frequencyCount = static_cast<int>(currentLoca->frequencyFolders().size());
			for (int j = 0; j < frequencyCount; j++)
			{
				std::string fMin = std::to_string(currentFrequencyBand.FMin());
				std::string fMax = std::to_string(currentFrequencyBand.FMax());
				std::vector<std::string> sm0Files = currentLoca->frequencyFolders()[j].FilePaths(SM0_ELAN);

				if ((currentLoca->frequencyFolders()[j].frequencyName() == "f" + fMin + "f" + fMax) && (sm0Files.size() > 0))
				{
					std::vector<std::string> dataFiles = currentLoca->frequencyFolders()[j].FilePaths(SM0_ELAN);
					int result = myeegContainer->LoadFrequencyData(dataFiles, 0);
					if (result == 0)
					{
						emit incrementAdavnce(1);
						emit sendLogInfo("Envelloppe File Loaded");

						std::string freqFolder = CreateFrequencyFolder(myeegContainer, currentFrequencyBand);
						GenerateMapsAndFigures(myeegContainer, freqFolder, m_analysisOpt[i]);
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

void InsermLibrary::LOCA::LocalizeMapsOnly(eegContainer* myeegContainer, int idCurrentLoca)
{
	m_idCurrentLoca = idCurrentLoca;
	m_currentLoca = nullptr;
	int examCountToProcess = static_cast<int>(m_analysisOpt.size());

	std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
	int samplingFrequency = myeegContainer->SamplingFrequency();
	m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);
	for (int i = 0; i < examCountToProcess; i++)
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

void InsermLibrary::LOCA::GenerateMapsAndFigures(eegContainer* myeegContainer, std::string freqFolder, FrequencyBandAnalysisOpt a)
{
	std::vector<EEGFormat::ITrigger> triggers = myeegContainer->Triggers();
	int samplingFrequency = myeegContainer->SamplingFrequency();
	m_triggerContainer = new TriggerContainer(triggers, samplingFrequency);

	//We generate file.pos and file_dsX.pos if we find a prov file 
	//with the exact same name as the experiment.	
	PROV* mainTask = LoadProvForTask();
	if (mainTask != nullptr)
	{
		CreateEventsFile(a, myeegContainer, m_triggerContainer, mainTask);
		CreateConfFile(myeegContainer);
		EEGFormat::Utility::DeleteAndNullify(mainTask);
	}
	else
	{
		//if we are at this point, no prov file to generate figures
		//but we still need to advance the progress bar
		emit sendLogInfo("No Protocol file found, no maps will be generated");
		if (a.env2plot) emit incrementAdavnce(1);
		if (a.trialmat) emit incrementAdavnce(1);
	}

	std::vector<PROV> provFiles = LoadAllProvForTask();
	for (size_t i = 0; i < provFiles.size(); i++)
	{
		m_triggerContainer->ProcessEventsForExperiment(&provFiles[i], 99);
		if (m_triggerContainer->ProcessedTriggerCount() == 0)
		{
			emit sendLogInfo("No Trigger found for this experiment, aborting maps generation");
			continue;
		}

		if (a.env2plot)
		{
			if (ShouldPerformBarPlot(m_currentLoca->localizerName()) || IsBarPlot(provFiles[i].filePath()))
			{
				Barplot(myeegContainer, &provFiles[i], freqFolder);
				emit incrementAdavnce(static_cast<int>(provFiles.size()));
			}
			else
			{
				if (provFiles[i].invertmapsinfo == "")
				{
					Env2plot(myeegContainer, &provFiles[i], freqFolder);
					emit incrementAdavnce(static_cast<int>(provFiles.size()));
				}
			}
		}

		if (a.trialmat && (IsBarPlot(provFiles[i].filePath()) == false || provFiles.size() == 1))
		{
			TimeTrialMatrices(myeegContainer, &provFiles[i], freqFolder);
			emit incrementAdavnce(static_cast<int>(provFiles.size()));
		}

		if (a.correMaps)
		{
			CorrelationMaps(myeegContainer, freqFolder);
			emit incrementAdavnce(static_cast<int>(provFiles.size()));
		}
	}

	deleteAndNullify1D(m_triggerContainer);
}

void InsermLibrary::LOCA::CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer* myeegContainer, TriggerContainer* triggerContainer, PROV* myprovFile)
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
		std::string downsampledEventsFilePath = fileNameBase + "_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + ".pos";
		std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
		std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());
		CreateFile(outputType, eventFilePath, triggers);
		CreateFile(outputType, downsampledEventsFilePath, triggersDownsampled);
		break;
	}
	case EEGFormat::FileType::BrainVision:
	{
		std::string eventFilePath = fileNameBase + ".vmrk";
		std::string downsampledEventsFilePath = fileNameBase + "_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + ".vmrk";
		std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
		std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());

		//We do not add the datafile indication for the ds.vmrk because we chose not to duplicate said file for each fXX_fYY_dsZ eeg file
		//and thus we can not make it point toward each of the analysis data file.
		std::string eventDataFilePath = myeegContainer->RootFileName() + ".eeg";
		CreateFile(outputType, eventFilePath, triggers, eventDataFilePath);
		CreateFile(outputType, downsampledEventsFilePath, triggersDownsampled, "");

		std::string frequencySuffix = "f" + std::to_string(analysisOpt.Band.FMin()) + "f" + std::to_string(analysisOpt.Band.FMax());
		RelinkAnalysisFileAnUglyWay(myeegContainer->RootFileFolder(), myeegContainer->RootFileName(), frequencySuffix, std::to_string(myeegContainer->DownsamplingFactor()));
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

void InsermLibrary::LOCA::CreateFile(EEGFormat::FileType outputType, std::string filePath, std::vector<Trigger>& triggers, std::string extraFilePath)
{
	std::vector<EEGFormat::ITrigger> iTriggers(triggers.size());
	for (int i = 0; i < static_cast<int>(iTriggers.size()); i++)
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
		EEGFormat::BrainVisionFile::SaveMarkers(filePath, extraFilePath, iTriggers, std::vector<EEGFormat::INote>());
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
void InsermLibrary::LOCA::CreateConfFile(eegContainer* myeegContainer)
{
	std::string outputConfFilePath = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();
	int electrodesCount = static_cast<int>(myeegContainer->flatElectrodes.size());

	std::ofstream confFile(outputConfFilePath + ".conf", std::ios::out);
	confFile << "nb_channel" << "  " << electrodesCount << std::endl;
	confFile << "sec_per_page" << "  " << 4 << std::endl;
	confFile << "amp_scale_type" << "  " << 1 << std::endl;
	confFile << "amp_scale_val" << "  " << 1 << std::endl;

	confFile << "channel_visibility" << std::endl;
	for (int i = 0; i < electrodesCount; i++)
	{
		confFile << 1 << std::endl;
	}
	/**********************************************************/
	/* We print the bipole Id as long as the next is from the */
	/* same Electrode : A'1, A'2, A'3 ... then when the next  */
	/* one is not from the same Electrode (B'1) we put -1 and */
	/*				we go again until the end				  */
	/**********************************************************/
	confFile << "channel_reference" << std::endl;
	confFile << "-1" << std::endl;

	for (int i = 0; i < myeegContainer->BipoleCount() - 1; i++)
	{
		if (myeegContainer->Bipole(i).second + 1 != myeegContainer->Bipole(i + 1).second)
		{
			//-1
			confFile << myeegContainer->Bipole(i).second << std::endl;
			confFile << "-1" << std::endl;
		}
		else
		{
			confFile << myeegContainer->Bipole(i).second << std::endl;
		}
	}
	confFile.close();
}

//! Create output folder if it does not exist
/*!
  \param myeegContainer contains the path informations of the data to process
  \param currentFreq contains the frequency informations
  \return The path of the created folder
*/
std::string InsermLibrary::LOCA::CreateFrequencyFolder(eegContainer* myeegContainer, FrequencyBand currentFreq)
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

InsermLibrary::PROV* InsermLibrary::LOCA::LoadProvForTask()
{
	std::string MainProvPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/" + m_currentLoca->localizerName() + ".prov";
	if (EEGFormat::Utility::DoesFileExist(MainProvPath))
	{
		return new PROV(MainProvPath);
	}
	else
	{
		return nullptr;
	}
}

std::vector<InsermLibrary::PROV> InsermLibrary::LOCA::LoadAllProvForTask()
{
	std::string provFolder = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/";

	std::vector<PROV> provFiles;
	for (int i = 0; i < static_cast<int>(m_statOption->locaWilcoxon.size()); i++)
	{
		if (m_statOption->locaWilcoxon[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			std::string possibleFile = provFolder + m_statOption->locaWilcoxon[i].toStdString() + ".prov";
			if (EEGFormat::Utility::DoesFileExist(possibleFile))
			{
				provFiles.push_back(PROV(possibleFile));
			}
		}
	}

	for (int i = 0; i < static_cast<int>(m_statOption->locaKruskall.size()); i++)
	{
		if (m_statOption->locaKruskall[i].contains(QString::fromStdString(m_currentLoca->localizerName())))
		{
			std::string possibleFile = provFolder + m_statOption->locaKruskall[i].toStdString() + ".prov";
			if (EEGFormat::Utility::DoesFileExist(possibleFile))
			{
				provFiles.push_back(PROV(possibleFile));
			}
		}
	}

	return provFiles;
}

bool InsermLibrary::LOCA::ShouldPerformBarPlot(std::string locaName)
{
	for (int i = 0; i < static_cast<int>(m_statOption->locaKruskall.size()); i++)
	{
		if (m_statOption->locaKruskall[i].contains(QString::fromStdString(locaName)))
		{
			return true;
		}
	}

	return false;
}

bool InsermLibrary::LOCA::IsBarPlot(std::string provFile)
{
	vec1<std::string> splitFile = split<std::string>(provFile, "_");
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
void InsermLibrary::LOCA::Barplot(eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = GetBarplotMapsFolder(freqFolder, myprovFile);
	std::string mapPath = PrepareFolderAndPathsBar(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = ProcessKruskallStatistic(eegData3D, myeegContainer, myprovFile, mapsFolder);

	//==
	InsermLibrary::DrawbarsPlots::drawBars b = InsermLibrary::DrawbarsPlots::drawBars(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, significantValue, myeegContainer);

	delete windowSam;
}

std::string InsermLibrary::LOCA::GetBarplotMapsFolder(std::string freqFolder, PROV* myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	mapsFolder.append("_bar");

	std::stringstream streamPValue;
	streamPValue << std::fixed << std::setprecision(2) << m_statOption->pKruskall;
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

std::string InsermLibrary::LOCA::PrepareFolderAndPathsBar(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_bar_");
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::ProcessKruskallStatistic(vec3<float>& bigData, eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
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
void InsermLibrary::LOCA::Env2plot(eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = GetEnv2PlotMapsFolder(freqFolder, myprovFile);
	std::string mapPath = PrepareFolderAndPathsPlot(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//==
	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, myeegContainer, 2);

	delete windowSam;
}

std::string InsermLibrary::LOCA::GetEnv2PlotMapsFolder(std::string freqFolder, PROV* myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	std::vector<std::string> myProv = split<std::string>(myprovFile->filePath(), "/");
	return mapsFolder.append("_plots").append(" - " + myProv[myProv.size() - 1]);
}

std::string InsermLibrary::LOCA::PrepareFolderAndPathsPlot(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_plot_");
}

/************/
/* TrialMat */
/************/
void InsermLibrary::LOCA::TimeTrialMatrices(eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
{
	std::vector<PVALUECOORD> significantValue;
	//== get some useful information
	std::string mapsFolder = GetTrialmatFolder(myprovFile, freqFolder);
	std::string mapPath = PrepareFolderAndPathsTrial(mapsFolder, myeegContainer->DownsamplingFactor());
	int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());
	int nbRow = myprovFile->nbRow();

	//== get Bloc of eeg data we want to display center around events
	vec3<float> bigData;
	bigData.resize(myeegContainer->BipoleCount(), vec2<float>(m_triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocData(bigData, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	if (ShouldPerformTrialmatStats(m_currentLoca->localizerName()))
		significantValue = ProcessWilcoxonStatistic(bigData, myeegContainer, myprovFile, mapsFolder);

	//== Draw for each plot and according to a template to reduce drawing time
	std::vector<int> SubGroupStimTrials = m_triggerContainer->SubGroupStimTrials();
	InsermLibrary::DrawCard::mapsGenerator mGen(m_picOption->sizeTrialmap.width(), m_picOption->sizeTrialmap.height());
	mGen.trialmatTemplate(SubGroupStimTrials, myprovFile);

	QPixmap* pixmapChanel = nullptr, * pixmapSubSubMatrix = nullptr;
	QPainter* painterChanel = nullptr, * painterSubSubMatrix = nullptr;
	for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
	{
		deleteAndNullify1D(painterChanel);
		deleteAndNullify1D(pixmapChanel);
		pixmapChanel = new QPixmap(mGen.pixmapTemplate);
		painterChanel = new QPainter(pixmapChanel);

		float stdRes = Framework::Calculations::Stats::Measure::MeanOfStandardDeviation(bigData[i]);
		float Maxi = 2.0 * abs(stdRes);
		float Mini = -2.0 * abs(stdRes);
		float AbsMax = std::fmax(abs(Maxi), abs(Mini));
		mGen.graduateColorBar(painterChanel, AbsMax);

		int interpolFactorX = m_picOption->interpolationtrialmap.width();
		int interpolFactorY = m_picOption->interpolationtrialmap.height();

		int indexPos = 0;
		int conditionCount = static_cast<int>(myprovFile->visuBlocs.size());
		for (int j = 0; j < conditionCount; j++)
		{
			int* currentWinMs = myprovFile->getWindowMs(j);
			int* currentWinSam = myprovFile->getWindowSam(myeegContainer->DownsampledFrequency(), j);
			int nbSampleWindow = currentWinSam[1] - currentWinSam[0];
			int indexBegTrigg = SubGroupStimTrials[j];
			int numberSubTrial = SubGroupStimTrials[j + 1] - indexBegTrigg;
			int subsubMatrixHeigth = 0;

			vec1<int> colorX[512], colorY[512];
			if (interpolFactorX > 1)
			{
				vec2<float> dataInterpolatedHoriz = mGen.horizontalInterpolation(bigData[i], interpolFactorX, indexBegTrigg, numberSubTrial);
				vec2<float> dataInterpolatedVerti = mGen.verticalInterpolation(dataInterpolatedHoriz, interpolFactorY);
				mGen.eegData2ColorMap(colorX, colorY, dataInterpolatedVerti, AbsMax);
				subsubMatrixHeigth = interpolFactorY * (numberSubTrial - 1);
			}
			else
			{
				mGen.eegData2ColorMap(colorX, colorY, bigData[i], AbsMax);
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

				int colorCount = static_cast<int>(colorX[k].size());
				for (int l = 0; l < colorCount; l++)
				{
					painterSubSubMatrix->drawPoint(QPoint(colorX[k][l], subsubMatrixHeigth - colorY[k][l]));
				}
			}

			indexPos = myprovFile->visuBlocs.size() - 1 - j;
			painterChanel->drawPixmap(mGen.subMatrixes[indexPos].x(), mGen.subMatrixes[indexPos].y(),
				pixmapSubSubMatrix->scaled(QSize(mGen.subMatrixes[indexPos].width(), mGen.subMatrixes[indexPos].height()),
					Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));

			/********************************************************************/
			/*	Add reaction time on map, if the calculated position is not		*/
			/*  inside the map , no need to draw it							 	*/
			/********************************************************************/
			painterChanel->setPen(QColor(Qt::black));
			for (int l = 0; l < numberSubTrial; l++)
			{
				int xReactionTimeMs = abs(currentWinMs[0]) + m_triggerContainer->ProcessedTriggers()[indexBegTrigg + l].ReactionTimeInMilliSeconds();
				double xScale = (double)(currentWinMs[1] - currentWinMs[0]) / mGen.MatrixRect.width();
				double xRt = mGen.MatrixRect.x() + (xReactionTimeMs / xScale);

				int yTrialPosition = mGen.subMatrixes[indexPos].y() + mGen.subMatrixes[indexPos].height();
				double yRt = yTrialPosition - (((double)mGen.subMatrixes[indexPos].height() / numberSubTrial) * l) - 1;

				if (xRt >= mGen.MatrixRect.x() && xRt <= (mGen.MatrixRect.x() + mGen.MatrixRect.width()))
				{
					painterChanel->setBrush(Qt::black);
					painterChanel->drawEllipse(QPoint(xRt, yRt), (int)(0.0034722 * mGen.MatrixRect.width()),
						(int)(0.004629629 * mGen.MatrixRect.height()));
				}
			}

			delete currentWinMs;
			delete currentWinSam;
		}

		mGen.drawVerticalZeroLine(painterChanel, myprovFile);

		//Display Stat
		if (m_statOption->wilcoxon)
		{
			std::vector<int> allIdCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
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

std::string InsermLibrary::LOCA::GetTrialmatFolder(PROV* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	if (myprovFile->invertmapsinfo == "")
		mapsFolder.append("_trials_stim");
	else
		mapsFolder.append("_trials_resp");

	std::stringstream streamPValue;
	streamPValue << std::fixed << std::setprecision(2) << m_statOption->pWilcoxon;
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

std::string InsermLibrary::LOCA::PrepareFolderAndPathsTrial(std::string mapsFolder, int dsSampFreq)
{
	if (!QDir(&mapsFolder.c_str()[0]).exists())
		QDir().mkdir(&mapsFolder.c_str()[0]);

	vec1<std::string> pathSplit = split<std::string>(mapsFolder, "/");

	return std::string(mapsFolder + "/" + pathSplit[pathSplit.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_trials_");
}

bool InsermLibrary::LOCA::ShouldPerformTrialmatStats(std::string locaName)
{
	for (int i = 0; i < static_cast<int>(m_statOption->locaWilcoxon.size()); i++)
	{
		if (m_statOption->locaWilcoxon[i].contains(QString::fromStdString(locaName)))
		{
			return true;
		}
	}

	return false;
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::ProcessWilcoxonStatistic(vec3<float>& bigData, eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
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

/********************/
/* Correlation Maps */
/********************/


void InsermLibrary::LOCA::CorrelationMaps(eegContainer* myeegContainer, std::string freqFolder)
{
	//Hardcoded stuff, look for parameters later
	int halfWindowSizeInSeconds = 15;

	//=== end hardcoded stuff
	int stepInSample = round(myeegContainer->elanFrequencyBand[0]->SamplingFrequency() * halfWindowSizeInSeconds);
	std::vector<int> windowsCenter;
	windowsCenter.push_back(stepInSample);
	while (windowsCenter[windowsCenter.size() - 1] + (2 * stepInSample) < myeegContainer->elanFrequencyBand[0]->NumberOfSamples())
	{
		windowsCenter.push_back(windowsCenter[windowsCenter.size() - 1] + stepInSample);
	}

	//We remove the two extreme border windows to avoid border effects 
	windowsCenter.erase(windowsCenter.begin() + (windowsCenter.size() - 1));
	windowsCenter.erase(windowsCenter.begin());

	int TriggerCount = windowsCenter.size();
	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(TriggerCount, vec2<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), vec1<float>(2 * stepInSample)));
	std::vector<std::vector<float>> currentData = myeegContainer->elanFrequencyBand[0]->Data(EEGFormat::DataConverterType::Analog);
	for (int i = 0; i < TriggerCount; i++)
	{
		for (int j = 0; j < (2 * stepInSample); j++)
		{
			int beginTime = windowsCenter[i] - stepInSample;

			for (int k = 0; k < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					eegData3D[i][k][j] = 0;
				else
					eegData3D[i][k][j] = (currentData[k][beginTime + j] - 100);
			}
		}
	}

	//== Compute circular coordinates
	//		=> CAN NOT DO THAT WITHOUT PTS 
	//		=> AT THE MOMENT WILL DO A LABEL COMPARAISON , SAME CHARACTER PART AND 
	//		=> DIFFERENT NUMBER WILL BE USED AND DISTANCE OF 4 CONTACT WILL BE USED	(MIGHT CHANGE)


	std::vector<std::vector<int>> dist = std::vector<std::vector<int>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<int>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
	for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
	{
		for (int j = 0; j < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); j++)
		{
			std::string mainLabel = myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label();
			std::string secondaryLabel = myeegContainer->elanFrequencyBand[0]->Electrode(j)->Label();

			//from InsermLibrary::eegContainer::GetIndexFromElectrodeLabel
			int goodId = -1;
			for (int k = 0; k < mainLabel.size(); k++)
			{
				if (isdigit(mainLabel[k]) && mainLabel[k] != 0)
				{
					goodId = k;
					break;
				}
			}
			std::string subLabel = mainLabel.substr(0, goodId);
			int contactIndex = stoi(mainLabel.substr(goodId, mainLabel.size()));

			goodId = -1;
			for (int k = 0; k < secondaryLabel.size(); k++)
			{
				if (isdigit(secondaryLabel[k]) && secondaryLabel[k] != 0)
				{
					goodId = k;
					break;
				}
			}
			std::string subLabel2 = secondaryLabel.substr(0, goodId);
			int contactIndex2 = stoi(secondaryLabel.substr(goodId, secondaryLabel.size()));

			dist[i][j] = (subLabel == subLabel2) ? contactIndex2 - contactIndex : 30;
		}
	}


	//== Compute surrogate (nb = 10000 , hardcoded for now)
	std::vector<float> surrogates = std::vector<float>(10000);
	for (int ii = 0; ii < 10000; ii++)
	{
		std::vector<int> rand;
		for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
		{
			rand.push_back(i);
		}

		// obtain a time-based seed and shuffle
		unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(rand.begin(), rand.end(), std::default_random_engine(seed));

		int seedIndex1 = rand[0];
		//==================================
		std::vector<int> matches;
		for (int i = 0; i < dist[seedIndex1].size(); i++)
		{
			if (dist[seedIndex1][i] == 30)
			{
				matches.push_back(i);
			}
		}

		seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(matches.begin(), matches.end(), std::default_random_engine(seed));

		int seedIndex2 = matches[0];
		//==================================
		std::vector<int> eventRand;
		for (int i = 0; i < round(TriggerCount / 3); i++)
		{
			eventRand.push_back(i);
		}

		seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(eventRand.begin(), eventRand.end(), std::default_random_engine(seed));

		int seedIndexEvent = eventRand[0];
		//==================================

		//coeff = pearsonCoefficient(baseline, ChannelData)
		surrogates[ii] = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData3D[seedIndexEvent][seedIndex1], eegData3D[seedIndexEvent][seedIndex2]);
	}

	std::vector<float> surrogatesAbs = std::vector<float>(surrogates);
	for (int i = 0; i < surrogatesAbs.size(); i++)
	{
		surrogatesAbs[i] = abs(surrogates[i]);
	}
	std::sort(surrogatesAbs.begin(), surrogatesAbs.end());
	float s_rmax = surrogatesAbs[surrogatesAbs.size() - 1]; //not the same way as jp's script , see if it changes result 
	float s_rmin = -1 * s_rmax;

	std::vector<std::vector<float>> bigCorrePlus = std::vector<std::vector<float>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
	for (int ii = 0; ii < TriggerCount; ii++)
	{
		for (int i = 0; i < bigCorrePlus.size(); i++)
		{
			for (int j = 0; j < bigCorrePlus[i].size(); j++)
			{
				if (i == j)
				{
					bigCorrePlus[i][j] = 1;
					continue;
				}

				float corre = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData3D[ii][i], eegData3D[ii][j]);
				bigCorrePlus[i][j] = (corre / TriggerCount);// corre > s_rmax ? corre / TriggerCount : 0;
			}
		}
	}

	//==================================

	QPixmap* pixmapChanel = new QPixmap(1200, 1200);
	QPainter* painterChanel = new QPainter(pixmapChanel);

	//#define PI 3.14159265f
	int electrodeCount = myeegContainer->elanFrequencyBand[0]->ElectrodeCount();
	int radius = 570;
	for (int i = 0; i < electrodeCount; i++)
	{
		QString label = QString::fromStdString(myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label());

		//elec label
		float angle = (2 * 3.14159265f * i) / electrodeCount;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		painterChanel->drawText(QPoint(600 + x, 600 - y), label);

		//== correlation circle point
		x = (radius - 60) * cos(angle);
		y = (radius - 60) * sin(angle);
		painterChanel->drawEllipse(QPoint(600 + x, 600 - y), 8, 8);
	}

	int s_minpct_toshow = 10;
	for (int i = 0; i < electrodeCount - 1; i++)
	{
		float angle = (2 * 3.14159265f * i) / electrodeCount;
		float x = (radius - 60) * cos(angle);
		float y = (radius - 60) * sin(angle);
		for (int j = 1; j < electrodeCount; j++)
		{
			float angle2 = (2 * 3.14159265f * j) / electrodeCount;
			float x2 = (radius - 60) * cos(angle2);
			float y2 = (radius - 60) * sin(angle2);

			if (dist[i][j] > 4 && dist[i][j] < 30)
			{
				float width = ((float)100 / s_minpct_toshow) * bigCorrePlus[i][j];
				if (width > 0)
				{
					//painterChanel->drawEllipse(QPoint(600 + x, 600 - y), 8, 8);
					painterChanel->drawLine(600 + x, 600 - y, 600 + x2, 600 - y2);
				}
			}
		}
	}


	pixmapChanel->save("D:/Users/Florian/Desktop/test.jpg", "JPG");
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);


	emit sendLogInfo("Correlation map generated");
}

//
//int InsermLibrary::eegContainer::GetIndexFromElectrodeLabel(std::string myString)
//{
//	for (int j = 0; j < myString.size(); j++)
//	{
//		if (isdigit(myString[j]) && myString[j] != 0)
//		{
//			return j;
//		}
//	}
//	return -1;
//}