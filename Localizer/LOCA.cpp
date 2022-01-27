#include "LOCA.h"

//#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <QDebug>

InsermLibrary::LOCA::LOCA(std::vector<FrequencyBandAnalysisOpt>& analysisOpt, statOption* statOption, picOption* picOption, std::string ptsFilePath)
{
	m_analysisOpt = analysisOpt;
	m_statOption = statOption;
	m_picOption = picOption;
	m_PtsFilePath = ptsFilePath;
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
        if (a.statFiles) emit incrementAdavnce(1);
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

        if(a.statFiles)
        {
            StatisticalFiles(myeegContainer, &provFiles[i], freqFolder);
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

    int ElectrodeCount = myeegContainer->elanFrequencyBand[0]->ElectrodeCount();
	int stepInSample = round(myeegContainer->elanFrequencyBand[0]->SamplingFrequency() * halfWindowSizeInSeconds);
    std::vector<int> windowsCenter = DefineCorrelationWindowsCenter(stepInSample, myeegContainer->elanFrequencyBand[0]->NumberOfSamples());
	int TriggerCount = windowsCenter.size();

	//== get Bloc of eeg data we want to display center around events
    vec3<float> eegData3D = vec3<float>(TriggerCount, vec2<float>(ElectrodeCount, vec1<float>(2 * stepInSample)));
    std::vector<std::vector<float>> currentData = myeegContainer->elanFrequencyBand[0]->Data(EEGFormat::DataConverterType::Analog);
	for (int i = 0; i < TriggerCount; i++)
	{
		for (int j = 0; j < (2 * stepInSample); j++)
		{
			int beginTime = windowsCenter[i] - stepInSample;

            for (int k = 0; k < ElectrodeCount; k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					eegData3D[i][k][j] = 0;
				else
                    eegData3D[i][k][j] = currentData[k][beginTime + j];
			}
		}
	}

	//== Compute circular coordinates
	std::vector<std::vector<float>> dist = (m_PtsFilePath != "") ? ComputeElectrodesDistancesFromPts(myeegContainer) : ComputeElectrodesDistances(myeegContainer);

	//== Compute surrogate (nb = 1000000 , hardcoded for now)
    float s_rmax = ComputeSurrogate(ElectrodeCount, TriggerCount, 1000000, dist, eegData3D);
	float s_rmin = -1 * s_rmax;

    //== Computecorrelations
    std::vector<std::vector<float>> bigCorrePlus = std::vector<std::vector<float>>(ElectrodeCount, std::vector<float>(ElectrodeCount));
    std::vector<std::vector<float>> bigCorreMinus = std::vector<std::vector<float>>(ElectrodeCount, std::vector<float>(ElectrodeCount));
	for (int ii = 0; ii < TriggerCount; ii++)
	{
        for (int i = 0; i < ElectrodeCount; i++)
		{
            for (int j = 0; j < ElectrodeCount; j++)
			{
				if (i == j)
				{
					bigCorrePlus[i][j] = 1;
                    bigCorreMinus[i][j] = 1;
					continue;
				}

				float corre = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData3D[ii][i], eegData3D[ii][j]);
				bigCorrePlus[i][j] += (corre > s_rmax) ? (1.0f / TriggerCount) : 0;
                bigCorreMinus[i][j] += (corre < s_rmin) ? (1.0f / TriggerCount) : 0;
			}
		}
	}

    std::string outputFilePath = DefineMapPath(freqFolder, myeegContainer->DownsamplingFactor(), halfWindowSizeInSeconds * 2);
    //==================================
	int width = 2400;
	int height = 1200;

	QPixmap* pixmapChanel = new QPixmap(width, height);
	pixmapChanel->fill(QColor(Qt::white));
	QPainter* painterChanel = new QPainter(pixmapChanel);

	int offset = width / 2;
	DrawCorrelationCircle(painterChanel, myeegContainer, width / 2, height / 2, 0); //negativ correlation area
	DrawCorrelationCircle(painterChanel, myeegContainer, width / 2, height / 2, offset); //positiv correlation area

	DrawCorrelationOnCircle(painterChanel, height / 2, 0, dist, bigCorreMinus); //negativ correlations
	DrawCorrelationOnCircle(painterChanel, height / 2, offset, dist, bigCorrePlus); //positiv correlations

    pixmapChanel->save(outputFilePath.c_str(), "JPG");
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);


	emit sendLogInfo("Correlation map generated");
}

std::string InsermLibrary::LOCA::DefineMapPath(std::string freqFolder, int dsSampFreq, int windowSizeInSec)
{
    vec1<std::string> pathSplit = split<std::string>(freqFolder, "/");
    return std::string(freqFolder + "/" + pathSplit[pathSplit.size() - 1] + "_ds" + std::to_string(dsSampFreq) + "_sm0_ccircle_w" + std::to_string(windowSizeInSec) + "s.jpg");
}

//Define correlation Windows
//from the file size and halfwindowsize, determine the centers (in sample)
//of all the windows that will be considered for correlation computation
std::vector<int> InsermLibrary::LOCA::DefineCorrelationWindowsCenter(int halfWindowSizeInSample, int fileSizeInSample)
{
    std::vector<int> windowsCenter;

    windowsCenter.push_back(halfWindowSizeInSample);
    while (windowsCenter[windowsCenter.size() - 1] + (2 * halfWindowSizeInSample) < fileSizeInSample)
    {
        windowsCenter.push_back(windowsCenter[windowsCenter.size() - 1] + halfWindowSizeInSample);
    }

    //We remove the two extreme border windows to avoid border effects
    windowsCenter.erase(windowsCenter.begin() + (windowsCenter.size() - 1));
    windowsCenter.erase(windowsCenter.begin());

    return windowsCenter;
}

std::vector<std::vector<float>> InsermLibrary::LOCA::ComputeElectrodesDistances(eegContainer* myeegContainer)
{
    std::vector<std::vector<float>> dist = std::vector<std::vector<float>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
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
    return dist;
}

std::vector<std::vector<float>> InsermLibrary::LOCA::ComputeElectrodesDistancesFromPts(eegContainer* myeegContainer)
{
    std::vector<std::string> rawFile = readTxtFile(m_PtsFilePath);
    int electrodeCount = QString::fromStdString(rawFile[2]).toInt();

    std::vector<std::string> Label = std::vector<std::string>(electrodeCount);
    std::vector<float> x = std::vector<float>(electrodeCount);
    std::vector<float> y = std::vector<float>(electrodeCount);
    std::vector<float> z = std::vector<float>(electrodeCount);

    for(int i = 0; i < electrodeCount; i++)
    {
        std::vector<std::string> rawLine = InsermLibrary::split<std::string>(rawFile[3 + i],"\t ");
        Label[i] = rawLine[0];
        x[i] = QString::fromStdString(rawLine[1]).toFloat();
        y[i] = QString::fromStdString(rawLine[2]).toFloat();
        z[i] = QString::fromStdString(rawLine[3]).toFloat();
    }

    std::vector<std::vector<float>> dist = std::vector<std::vector<float>>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount(), std::vector<float>(myeegContainer->elanFrequencyBand[0]->ElectrodeCount()));
    for (int i = 0; i < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); i++)
    {
		for (int j = 0; j < myeegContainer->elanFrequencyBand[0]->ElectrodeCount(); j++)
		{
			std::string mainLabel = myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label();
			std::string mainLabel2 = mainLabel;
			std::replace(mainLabel2.begin(), mainLabel2.end(), '\'', 'p');

			std::string secondaryLabel = myeegContainer->elanFrequencyBand[0]->Electrode(j)->Label();
			std::string secondaryLabel2 = secondaryLabel;
			std::replace(secondaryLabel2.begin(), secondaryLabel2.end(), '\'', 'p');

			std::vector<std::string>::iterator it = std::find(Label.begin(), Label.end(), mainLabel2);
			int index = std::distance(Label.begin(), it);

			std::vector<std::string>::iterator it2 = std::find(Label.begin(), Label.end(), secondaryLabel2);
			int index2 = std::distance(Label.begin(), it2);

			float distt = 0.0f;
			//if one of the indexes is not found we consider coordinates to be 0,0,0
			if (index2 == Label.size())
			{
				distt = sqrtf((0 - x[index]) * (0 - x[index]) + (0 - y[index]) * (0 - y[index]) + (0 - z[index]) * (0 - z[index]));
			}
			else if (index == Label.size())
			{
				distt = sqrtf((x[index2] * x[index2]) + (y[index2] * y[index2]) + (z[index2] * z[index2]));
			}
			else
			{
				distt = sqrtf((x[index2] - x[index]) * (x[index2] - x[index]) + (y[index2] - y[index]) * (y[index2] - y[index]) + (z[index2] - z[index]) * (z[index2] - z[index]));
			}
				
			dist[i][j] = distt;
		}
    }

    return dist;
}

/// <summary>
/// Compute surrogates
/// </summary>
/// <param name="electrodeCount"></param>
/// <param name="triggerCount"></param>
/// <param name="surrogateCount"></param>
/// <param name="distances"></param>
/// <param name="eegData"></param>
/// <returns>the surrogate value at 99.99%</returns>
float InsermLibrary::LOCA::ComputeSurrogate(int electrodeCount, int triggerCount, int surrogateCount, vec2<float> distances, vec3<float> eegData)
{
    std::vector<float> surrogates = std::vector<float>(surrogateCount);
	#pragma omp parallel for
    for (int ii = 0; ii < surrogateCount; ii++)
    {
        std::vector<int> rand;
        for (int i = 0; i < electrodeCount; i++)
        {
            rand.push_back(i);
        }

        // obtain a time-based seed and shuffle
        unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(rand.begin(), rand.end(), std::default_random_engine(seed));

        int seedIndex1 = rand[0];
        //==================================
        std::vector<int> matches;
        for (int i = 0; i < distances[seedIndex1].size(); i++)
        {
            if (distances[seedIndex1][i] >= 30)
            {
                matches.push_back(i);
            }
        }

		unsigned int seed2 = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(matches.begin(), matches.end(), std::default_random_engine(seed2));

        int seedIndex2 = matches[0];
        //==================================
        std::vector<int> eventRand;
        for (int i = 0; i < round(triggerCount / 3); i++)
        {
            eventRand.push_back(i);
        }

		unsigned int seed3 = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(eventRand.begin(), eventRand.end(), std::default_random_engine(seed3));

        int seedIndexEvent = eventRand[0];
        int seedIndexEvent2 = (triggerCount - 1) - seedIndexEvent;
        //==================================

        surrogates[ii] = Framework::Calculations::Stats::Correlation::pearsonCoefficient(eegData[seedIndexEvent][seedIndex1], eegData[seedIndexEvent2][seedIndex2]);
    }

    std::vector<float> surrogatesAbs = std::vector<float>(surrogates);
    for (int i = 0; i < surrogateCount; i++)
    {
        surrogatesAbs[i] = abs(surrogates[i]);
    }
    std::sort(surrogatesAbs.begin(), surrogatesAbs.end());

	int index = round(0.9999f * surrogateCount);
	return surrogatesAbs[index];
}

void InsermLibrary::LOCA::DrawCorrelationCircle(QPainter* painterChanel, eegContainer* myeegContainer, int halfwidth, int halfheight, int offset)
{
	//#define PI 3.14159265f
	int radius = halfheight - 30;
	int ElectrodeCount = myeegContainer->elanFrequencyBand[0]->ElectrodeCount();
	std::string elecNameStringTemp = "%#";
	for (int i = 0; i < ElectrodeCount; i++)
	{
		QString label = QString::fromStdString(myeegContainer->elanFrequencyBand[0]->Electrode(i)->Label());
		QColor color = GetColorFromLabel(label.toStdString(), elecNameStringTemp);

		//elec label
		float angle = (2 * 3.14159265f * i) / ElectrodeCount;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		painterChanel->drawText(QPoint(offset + halfheight + x, halfheight - y), label);

		//== correlation circle point
		x = (radius - 60) * cos(angle);
		y = (radius - 60) * sin(angle);
		painterChanel->setPen(color);
		painterChanel->drawEllipse(QPoint(offset + halfheight + x, halfheight - y), 8, 8);
		painterChanel->setPen(Qt::black);
	}

	m_colorId = -1;
}

void InsermLibrary::LOCA::DrawCorrelationOnCircle(QPainter* painterChanel, int halfheight, int offset, std::vector<std::vector<float>> dist, std::vector<std::vector<float>> corre)
{
	int ElectrodeCount = corre.size();
	int radius = halfheight - 30;
	int s_minpct_toshow = 10;
	for (int i = 0; i < ElectrodeCount - 1; i++)
	{
		float angle = (2 * 3.14159265f * i) / ElectrodeCount;
		float x = (radius - 60) * cos(angle);
		float y = (radius - 60) * sin(angle);
		for (int j = 1; j < ElectrodeCount; j++)
		{
			float angle2 = (2 * 3.14159265f * j) / ElectrodeCount;
			float x2 = (radius - 60) * cos(angle2);
			float y2 = (radius - 60) * sin(angle2);

			if (dist[i][j] > 25)
			{
				float width = floor(((float)100 / s_minpct_toshow) * corre[i][j]);
				if (width > 0.0f)
				{
					QPoint p1(offset + halfheight + x, halfheight - y);
					QPoint p2(offset + halfheight + x2, halfheight - y2);
					painterChanel->drawLine(p1, p2);
				}
			}
		}
	}
}

QColor InsermLibrary::LOCA::GetColorFromLabel(std::string label, std::string& memoryLabel)
{
	QColor colors[]{ QColor(Qt::red), QColor(Qt::blue), QColor(Qt::darkYellow), QColor(Qt::green), QColor(Qt::gray), QColor(Qt::cyan), QColor(Qt::black), QColor(Qt::magenta) };

	std::string result = "";
	int resId = -1;

	int goodId = GetIndexFromElectrodeLabel(label);

	if (goodId != -1)
	{
		result = label.substr(0, goodId);
		resId = stoi(label.substr(goodId, label.size()));
	}
	else
	{
		result = label;
	}

	if (result.find(memoryLabel) != std::string::npos && (result.length() == memoryLabel.length()))
	{
		return colors[m_colorId];
	}
	else
	{
		memoryLabel = result;
		m_colorId = (m_colorId + 1) % 8;
		return colors[m_colorId];
	}
}

int InsermLibrary::LOCA::GetIndexFromElectrodeLabel(std::string myString)
{
	for (int j = 0; j < myString.size(); j++)
	{
		if (isdigit(myString[j]) && myString[j] != 0)
		{
			return j;
		}
	}
	return -1;
}

void InsermLibrary::LOCA::StatisticalFiles(eegContainer* myeegContainer, PROV* myprovFile, std::string freqFolder)
{
    //std::vector<PVALUECOORD> significantValue;
    //== get some useful information
    std::string mapsFolder = GetTrialmatFolder(myprovFile, freqFolder);
    std::string mapPath = PrepareFolderAndPathsTrial(mapsFolder, myeegContainer->DownsamplingFactor());
    int* windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());

    //== get Bloc of eeg data we want to display center around events
    vec3<float> bigData;
    bigData.resize(myeegContainer->BipoleCount(), vec2<float>(m_triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
    myeegContainer->GetFrequencyBlocData(bigData, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	std::vector<std::vector<std::vector<double>>> Stat_Z_CCS, Stat_P_CCS;
    for(int i = 0;i < bigData.size(); i++)
    {
        std::vector<std::vector<double>> Stat_Z_CS, Stat_P_CS;
        std::vector<int> ids = m_triggerContainer->SubGroupStimTrials();
        for(int j = 0; j < static_cast<int>(ids.size() - 1); j++)
        {
            int beg = ids[j];
            int end = ids[j+1];

            int baselineBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.baseLineMin())/1000) - windowSam[0];
            int baselineEnd = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.baseLineMax())/1000)  - windowSam[0];
            int windowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.windowMin())/1000)  - windowSam[0];
            int windowEnd = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.windowMax())/1000)  - windowSam[0];

            //Copy needed data , all trials for one condition and the associated samples
            vec1<double> baselineData = vec1<double>();
            vec2<double> conditionData = vec2<double>(end-beg, vec1<double>());
            for(int k = 0; k < conditionData.size(); k++)
            {
                int sum = std::accumulate(bigData[i][beg+k].begin() + baselineBegin, bigData[i][beg+k].begin() + baselineEnd, 0);
                double bl = static_cast<double>(sum) / (baselineEnd - baselineBegin);
                baselineData.push_back(bl); //baseline mean for this trial

                //copy relevant data
                std::vector<float>::iterator begIter = bigData[i][beg + k].begin() + windowBegin;
                std::vector<float>::iterator endIter = bigData[i][beg + k].begin() + windowEnd;
                conditionData[k] = vec1<double>(begIter, endIter);
            }

            //loop over timebins
            std::vector<double> v_stat_Z, v_stat_P;
            int timeBinsCount = static_cast<int>(conditionData[0].size());
            for(int k = 0; k < timeBinsCount; k++)
            {
                std::vector<double> dataToCompare;
                for(int l = 0; l < conditionData.size(); l++)
                {
                    dataToCompare.push_back(conditionData[l][k]);
                }

                std::pair<double, double> pz = Framework::Calculations::Stats::wilcoxon_rank_sum(dataToCompare, baselineData);
                v_stat_P.push_back(pz.first);
                v_stat_Z.push_back(pz.second);
            }

            Stat_P_CS.push_back(v_stat_P);
            Stat_Z_CS.push_back(v_stat_Z);
        }

        Stat_P_CCS.push_back(Stat_P_CS);
        Stat_Z_CCS.push_back(Stat_Z_CS);
    }

    std::vector<PVALUECOORD> significantValue;
    if (m_statOption->FDRwilcoxon)
    {
        int V = Stat_P_CCS.size() * Stat_P_CCS[0].size() * Stat_P_CCS[0][0].size();
        float CV = log(V) + 0.5772;
        float slope = m_statOption->pWilcoxon / (V * CV);

        std::vector<PVALUECOORD> preFDRValues = loadPValues(Stat_P_CCS);

        std::sort(preFDRValues.begin(), preFDRValues.end(),
            [](PVALUECOORD firstValue, PVALUECOORD secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });

        int copyIndex = 0;
        for (int i = 0; i < V; i++)
        {
            if (preFDRValues[i].pValue > ((double)slope * (i + 1)))
            {
                copyIndex = i;
                break;
            }
        }

        for (int i = 0; i < copyIndex; i++)
        {
            significantValue.push_back(preFDRValues[i]);
        }

        //verifier si celui la est necessaire ??
        std::sort(significantValue.begin(), significantValue.end(),
            [](PVALUECOORD firstValue, PVALUECOORD secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });
    }
    else
    {
        significantValue = loadPValues(Stat_P_CCS, m_statOption->pWilcoxon);
    }

	//Kruskall
	std::vector<std::vector<std::vector<std::vector<double>>>> v_stat_K4, v_stat_P4;
	for (int i = 0; i < bigData.size(); i++)
	{
		std::vector<std::vector<std::vector<double>>> v_stat_K3, v_stat_P3;
		std::vector<int> ids = m_triggerContainer->SubGroupStimTrials();
		for (int j = 0; j < static_cast<int>(ids.size() - 2); j++)
		{
			int firstConditionBeg = ids[j];
			int firstConditionEnd = ids[j + 1];
			int firstConditionWindowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.windowMin()) / 1000) - windowSam[0];
			int firstConditionWindowEnd = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.windowMax()) / 1000) - windowSam[0];

			//copy relevant data
			std::vector<std::vector<double>> firstConditionData;
            for (int k = 0; k < (firstConditionEnd - firstConditionBeg); k++)
			{
                std::vector<float>::iterator begIter = bigData[i][firstConditionBeg + k].begin() + firstConditionWindowBegin;
                std::vector<float>::iterator endIter = bigData[i][firstConditionBeg + k].begin() + firstConditionWindowEnd;
				firstConditionData.push_back(vec1<double>(begIter, endIter));
			}

			std::vector<std::vector<double>> v_stat_K2, v_stat_P2;
            for (int k = j + 1; k < static_cast<int>(ids.size() - 1); k++)
			{
				int secondConditionBeg = ids[k];
				int secondConditionEnd = ids[k + 1];
				int secondConditionWindowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[k].dispBloc.windowMin()) / 1000) - windowSam[0];
				int secondConditionWindowEnd = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[k].dispBloc.windowMax()) / 1000) - windowSam[0];

				//copy relevant data
				std::vector<std::vector<double>> secondConditionData;
                for (int l = 0; l < (secondConditionEnd - secondConditionBeg); l++)
				{
                    std::vector<float>::iterator begIter = bigData[i][secondConditionBeg + l].begin() + secondConditionWindowBegin;
                    std::vector<float>::iterator endIter = bigData[i][secondConditionBeg + l].begin() + secondConditionWindowEnd;
					secondConditionData.push_back(vec1<double>(begIter, endIter));
				}

				//loop over timebins
				std::vector<double> v_stat_K, v_stat_P;
				int timeBinsCount = static_cast<int>(firstConditionData[0].size());
				for (int l = 0; l < timeBinsCount; l++)
				{
					double firstMean = 0;
					std::vector<float> firstDataToCompare;
					for (int m = 0; m < firstConditionData.size(); m++)
					{
						firstMean += firstConditionData[m][l];
						firstDataToCompare.push_back(firstConditionData[m][l]);
					}
					firstMean /= firstConditionData.size();

					double secondMean = 0;
					std::vector<float> secondDataToCompare;
					for (int m = 0; m < secondConditionData.size(); m++)
					{
						secondMean += secondConditionData[m][l];
						secondDataToCompare.push_back(secondConditionData[m][l]);
					}
					secondMean /= secondConditionData.size();

                    //TODO : first pass doesn't send back a p value (=0) , why ?
					float* dataArray[2];
					int nbSamplePerGroup[2];
					double p = 0, H = 0;
					dataArray[0] = firstDataToCompare.data();
					dataArray[1] = secondDataToCompare.data();
					nbSamplePerGroup[0] = firstDataToCompare.size();
					nbSamplePerGroup[1] = secondDataToCompare.size();
					Framework::Calculations::Stats::kruskal_wallis(dataArray, 2, nbSamplePerGroup, &H, &p, 1);

					int sign_FirstMinusSecond = (firstMean - secondMean) < 0 ? -1 : 1;
					v_stat_K.push_back(sign_FirstMinusSecond * ((p < m_statOption->pWilcoxon) ? 1 : 0 ));
					v_stat_P.push_back(p);
				}

				v_stat_K2.push_back(v_stat_K);
				v_stat_P2.push_back(v_stat_P);
			}
			v_stat_K3.push_back(v_stat_K2);
			v_stat_P3.push_back(v_stat_P2);
		}
		v_stat_K4.push_back(v_stat_K3);
		v_stat_P4.push_back(v_stat_P3);
	}

    std::vector<PVALUECOORD_KW> significantValue2;
	if (m_statOption->FDRkruskall)
	{
        qDebug() << "fdr ";
        int V = v_stat_P4.size() * v_stat_P4[0].size() * v_stat_P4[0][0].size() * v_stat_P4[0][0][0].size();
        float CV = log(V) + 0.5772;
        float slope = m_statOption->pWilcoxon / (V * CV);

        std::vector<PVALUECOORD_KW> preFDRValues = loadPValues_KW(v_stat_P4);

        std::sort(preFDRValues.begin(), preFDRValues.end(),
            [](PVALUECOORD_KW firstValue, PVALUECOORD_KW secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });

        int copyIndex = 0;
        for (int i = 0; i < V; i++)
        {
            if (preFDRValues[i].pValue > ((double)slope * (i + 1)))
            {
                copyIndex = i;
                break;
            }
        }

        for (int i = 0; i < copyIndex; i++)
        {
            significantValue2.push_back(preFDRValues[i]);
        }

        //verifier si celui la est necessaire ??
        std::sort(significantValue2.begin(), significantValue2.end(),
            [](PVALUECOORD_KW firstValue, PVALUECOORD_KW secondValue) {
            return (firstValue.pValue < secondValue.pValue);
        });
	}
	else
	{
        qDebug() << "no fdr ";
        significantValue2 = loadPValues_KW(v_stat_P4, m_statOption->pKruskall);
	}

    std::vector<int> ids = m_triggerContainer->SubGroupStimTrials();
    std::vector<std::pair<int,int>> codesPairs;
    for (int j = 0; j < static_cast<int>(ids.size() - 2); j++)
    {
        std::vector<int> firstCodes = myprovFile->visuBlocs[j].mainEventBloc.eventCode;
        for (int k = j + 1; k < static_cast<int>(ids.size() - 1); k++)
        {
            std::vector<int> secondCodes = myprovFile->visuBlocs[k].mainEventBloc.eventCode;
            codesPairs.push_back(std::make_pair(firstCodes[0], secondCodes[0]));
        }
    }


    //at this point everything is processed, now we need to export that in an ELAN File
	std::vector<std::vector<double>> ChannelDataToWrite;
	std::vector<std::pair<int, int>> posSampleCodeToWrite;
    for(int i = 0; i < bigData.size(); i++)
    {
        std::vector<double> statToWrite;
        std::vector<std::pair<int, int>> posSampleCode;
        int sampleAlreadyWritten = 0;

		bool isChannelReactive = false;
        std::vector<int> ids = m_triggerContainer->SubGroupStimTrials();
        for(int j = 0; j < static_cast<int>(ids.size() - 1); j++)
        {
            int beg = ids[j];
            int end = ids[j+1];

            std::vector<int> codes = myprovFile->visuBlocs[j].mainEventBloc.eventCode;
			int windowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[j].dispBloc.windowMin()) / 1000);

            for(int k = 0; k < 3; k++) //repeat for better visibility in hibop
            {
                for(int l = 0; l < Stat_Z_CCS[i][j].size(); l++)
                {
                    statToWrite.push_back(10 * Stat_Z_CCS[i][j][l]);
                }

                int sample = std::abs(windowBegin) + sampleAlreadyWritten;
                int code = codes[0];
                posSampleCode.push_back(std::make_pair(sample,code));
                sampleAlreadyWritten += Stat_Z_CCS[i][j].size();
            }

            std::vector<int> indices;
            auto it = significantValue.begin();
            while ((it = std::find_if(it, significantValue.end(),[&](PVALUECOORD const& obj){ return obj.elec == i && obj.condit == j; })) != significantValue.end())
            {
                indices.push_back(std::distance(significantValue.begin(), it));
                it++;
            }

            //tout les indices correspondant dans le vector
            std::vector<double> signif = std::vector<double>(Stat_Z_CCS[i][j].size(), 0);
            for(int k = 0; k < indices.size(); k++)
            {
                int indexSignif = significantValue[indices[k]].window;
                signif[indexSignif] = 10;
            }

			for (int k = 0; k < 3; k++) //repeat for better visibility in hibop
			{
				for (int l = 0; l < signif.size(); l++)
				{
                    statToWrite.push_back(signif[l]);
				}
				int sample = std::abs(windowBegin) + sampleAlreadyWritten;
				int code = 1000 + codes[0];
				posSampleCode.push_back(std::make_pair(sample, code));
				sampleAlreadyWritten += signif.size();
			}
        }

		//ensuite mettre les 9999
		int windowBegin = (((float)myeegContainer->DownsampledFrequency() * myprovFile->visuBlocs[0].dispBloc.windowMin()) / 1000);
        int valueToWrite = 30 * (isChannelReactive ? 1 : 0);
		for (int j = 0; j < 3; j++) //repeat for better visibility in hibop
		{
			for (int k = 0; k < Stat_Z_CCS[0][0].size(); k++)
			{
                statToWrite.push_back(valueToWrite);
			}
			int sample = std::abs(windowBegin) + sampleAlreadyWritten;
			int code = 9999;
			posSampleCode.push_back(std::make_pair(sample, code));
			sampleAlreadyWritten += Stat_Z_CCS[0][0].size();
		}

		//need to add kruskall data to writable data
        int bigCounter = 0;
		for (int j = 0; j < static_cast<int>(ids.size() - 2); j++)
		{
            int counter = 0;
            for (int k = j + 1; k < static_cast<int>(ids.size() - 1); k++)
			{
				std::vector<int> indices;
				auto it = significantValue2.begin();
                while ((it = std::find_if(it, significantValue2.end(), [&](PVALUECOORD_KW const& obj) { return obj.elec == i && obj.condit1 == j && obj.condit2 == counter; })) != significantValue2.end())
				{
					indices.push_back(std::distance(significantValue2.begin(), it));
					it++;
				}

                std::vector<double> signif = std::vector<double>(v_stat_K4[0][0][0].size(), 0);
				for (int m = 0; m < indices.size(); m++)
				{
					int ind1 = significantValue2[indices[m]].elec;
					int ind2 = significantValue2[indices[m]].condit1;
					int ind3 = significantValue2[indices[m]].condit2;
					float indP = significantValue2[indices[m]].pValue;

					for (int l = 0; l < signif.size(); l++)
					{
                        signif[l] = 30 * v_stat_K4[ind1][ind2][ind3][l] * indP;
					}
				}

				for (int n = 0; n < 3; n++)
				{
					for (int l = 0; l < signif.size(); l++)
					{
                        statToWrite.push_back(signif[l]);
					}

					int sample = std::abs(windowBegin) + sampleAlreadyWritten;
                    int code = 10000 * (1000 + codesPairs[bigCounter].first) + (1000 + codesPairs[bigCounter].second);
					posSampleCode.push_back(std::make_pair(sample, code));
					sampleAlreadyWritten += signif.size();
				}

                counter++;
                bigCounter++;
			}
		}

		//eeg data
		ChannelDataToWrite.push_back(statToWrite);

		//copy pos data only once
		if (posSampleCodeToWrite.size() == 0)
		{
			posSampleCodeToWrite = std::vector<std::pair<int, int>>(posSampleCode);
		}
	}

    EEGFormat::ElanFile *outputFile = new EEGFormat::ElanFile();
    outputFile->ElectrodeCount((int)ChannelDataToWrite.size());
    outputFile->SamplingFrequency(myeegContainer->DownsampledFrequency());
    //Load electrodes list according to container
	std::vector<EEGFormat::IElectrode*> bipolesList;
	int BipoleCount = myeegContainer->BipoleCount();
	for (int i = 0; i < BipoleCount; i++)
	{
		std::pair<int, int> currentBipole = myeegContainer->Bipole(i);
		bipolesList.push_back(myeegContainer->Electrode(currentBipole.first));
	}
    outputFile->Electrodes(bipolesList);
    //Define type of elec : label + "EEG" + "uV"
    outputFile->Data(EEGFormat::DataConverterType::Digital).resize((int)bipolesList.size(), std::vector<float>(ChannelDataToWrite[0].size()));
	for (int i = 0; i < ChannelDataToWrite.size(); i++)
	{
		for (int j = 0; j < ChannelDataToWrite[i].size(); j++)
		{
            outputFile->Data(EEGFormat::DataConverterType::Digital)[i][j] = ChannelDataToWrite[i][j];
		}
	}

    //then save eegdata
    std::string rootFileFolder = EEGFormat::Utility::GetDirectoryPath(myeegContainer->elanFrequencyBand[0]->DefaultFilePath());
    std::string fileNameRoot = EEGFormat::Utility::GetFileName(myeegContainer->elanFrequencyBand[0]->DefaultFilePath(), false);

    std::string entFile = rootFileFolder + fileNameRoot + "_stats.eeg.ent";
    std::string eegFile = rootFileFolder + fileNameRoot + "_stats.eeg";
    outputFile->SaveAs(entFile,eegFile, "","");

    //and trigger in pos
    std::vector<EEGFormat::ITrigger> iTriggers(posSampleCodeToWrite.size());
    for (int i = 0; i < posSampleCodeToWrite.size(); i++)
    {
        int code = posSampleCodeToWrite[i].second;
        long sample = posSampleCodeToWrite[i].first;
        iTriggers[i] = EEGFormat::ElanTrigger(code, sample);
    }
    std::string fileNameBase = myeegContainer->RootFileFolder() + myeegContainer->RootFileName();
    std::string posFile = fileNameBase + "_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + "_stats.pos";
    EEGFormat::ElanFile::SaveTriggers(posFile, iTriggers);

    //and delete pointer
    DeleteGenericFile(outputFile);
}

//Temp : need to be put in some class once algorithm is validated

std::vector<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::loadPValues(vec3<double>& pValues3D)
{
	int count = 0;
    PVALUECOORD tempPValue;
    std::vector<PVALUECOORD> pValues;

	for (int i = 0; i < static_cast<int>(pValues3D.size()); i++)
	{
		for (int j = 0; j < static_cast<int>(pValues3D[i].size()); j++)
		{
			for (int k = 0; k < static_cast<int>(pValues3D[i][j].size()); k++)
			{
				tempPValue.elec = i;
				tempPValue.condit = j;
				tempPValue.window = k;
				tempPValue.vectorpos = count;
				tempPValue.pValue = pValues3D[i][j][k];
				pValues.push_back(tempPValue);
				count++;
			}
		}
	}

	return pValues;
}

std::vector<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::loadPValues(InsermLibrary::vec3<double>& pValues3D, float pLimit)
{
	int count = 0;
	InsermLibrary::PVALUECOORD tempPValue;
	std::vector<InsermLibrary::PVALUECOORD> pValues;

	for (int i = 0; i < static_cast<int>(pValues3D.size()); i++)
	{
		for (int j = 0; j < static_cast<int>(pValues3D[i].size()); j++)
		{
			for (int k = 0; k < static_cast<int>(pValues3D[i][j].size()); k++)
			{
				if (pValues3D[i][j][k] < pLimit)
				{
					tempPValue.elec = i;
					tempPValue.condit = j;
					tempPValue.window = k;
					tempPValue.vectorpos = count;
					tempPValue.pValue = pValues3D[i][j][k];
					//tempPValue.weight = pSign3D[i][j][k];

					pValues.push_back(tempPValue);
				}
				count++;
			}
		}
	}

	return pValues;
}

std::vector<InsermLibrary::PVALUECOORD_KW> InsermLibrary::LOCA::loadPValues_KW(vec4<double>& pValues4D)
{
    int count = 0;
    InsermLibrary::PVALUECOORD_KW tempPValue;
    std::vector<InsermLibrary::PVALUECOORD_KW> pValues;

    for (int i = 0; i < static_cast<int>(pValues4D.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(pValues4D[i].size()); j++)
        {
            for (int k = 0; k < static_cast<int>(pValues4D[i][j].size()); k++)
            {
                for (int l = 0; l < static_cast<int>(pValues4D[i][j][k].size()); l++)
                {
                    tempPValue.elec = i;
                    tempPValue.condit1 = j;
                    tempPValue.condit2 = k;
                    tempPValue.window = l;
                    tempPValue.vectorpos = count;
                    tempPValue.pValue = pValues4D[i][j][k][l];
                    pValues.push_back(tempPValue);
                    count++;
                }
            }
        }
    }

    return pValues;
}

std::vector<InsermLibrary::PVALUECOORD_KW> InsermLibrary::LOCA::loadPValues_KW(vec4<double>& pValues4D, float pLimit)
{
    int count = 0;
    InsermLibrary::PVALUECOORD_KW tempPValue;
    std::vector<InsermLibrary::PVALUECOORD_KW> pValues;

    for (int i = 0; i < static_cast<int>(pValues4D.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(pValues4D[i].size()); j++)
        {
            for (int k = 0; k < static_cast<int>(pValues4D[i][j].size()); k++)
            {
                for (int l = 0; l < static_cast<int>(pValues4D[i][j][k].size()); l++)
                {
                    if (pValues4D[i][j][k][l] < pLimit)
                    {
                        tempPValue.elec = i;
                        tempPValue.condit1 = j;
                        tempPValue.condit2 = k;
                        tempPValue.window = l;
                        tempPValue.vectorpos = count;
                        tempPValue.pValue = pValues4D[i][j][k][l];
                        pValues.push_back(tempPValue);
                        count++;
                    }
                }
            }
        }
    }

    return pValues;
}
