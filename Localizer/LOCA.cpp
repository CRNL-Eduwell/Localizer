#include "LOCA.h"

#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

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
void InsermLibrary::LOCA::Eeg2erp(eegContainer* myeegContainer, ProvFile* myprovFile)
{
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->SamplingFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->SamplingFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

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

	delete[] windowSam;
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
    ProvFile* task = LoadProvForTask(m_currentLoca->localizerName());
    ProvFile* taskInverted = LoadProvForTask(m_currentLoca->localizerName(), "INVERTED");
    ProvFile* taskBarPlot = LoadProvForTask(m_currentLoca->localizerName(), "BARPLOT");
    ProvFile* taskStatistics = LoadProvForTask(m_currentLoca->localizerName(), "STATISTICS");

    if (task != nullptr)
    {
        CreateEventsFile(a, myeegContainer, m_triggerContainer, task);
        CreateConfFile(myeegContainer);
    }

    //Process Env2Plot => LOCA
    if (a.env2plot)
    {
        if(task != nullptr)
        {
            m_triggerContainer->ProcessEventsForExperiment(task, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting Env2Plot generation");
            }
            else
            {
                Env2plot(myeegContainer, task, freqFolder);
            }
            emit incrementAdavnce(1);
        }
    }

    //Process Trialmatrices => LOCA and LOCA_INVERTED and LOCA_BARPLOT (only AUDI)
   if (a.trialmat)
   {
       if(task != nullptr)
       {
            m_triggerContainer->ProcessEventsForExperiment(task, 99);
            if (m_triggerContainer->ProcessedTriggerCount() == 0)
            {
                emit sendLogInfo("No Trigger found for this experiment, aborting trialmats generation");
            }
            else
            {
                TimeTrialMatrices(myeegContainer, task, freqFolder);
            }
       }
       if(taskInverted != nullptr)
       {
			m_triggerContainer->ProcessEventsForExperiment(taskInverted, 99);
			if (m_triggerContainer->ProcessedTriggerCount() == 0)
			{
				emit sendLogInfo("No Trigger found for this experiment, aborting inverted trialmats generation");
			}
			else
			{
				TimeTrialMatrices(myeegContainer, taskInverted, freqFolder);
			}
       }
       if(taskBarPlot != nullptr)
       {
           m_triggerContainer->ProcessEventsForExperiment(taskBarPlot, 99);
           if (m_triggerContainer->ProcessedTriggerCount() == 0)
           {
               emit sendLogInfo("No Trigger found for this experiment, aborting Barplot generation");
           }
           else
           {
               Barplot(myeegContainer, taskBarPlot, freqFolder);
           }
       }
       emit incrementAdavnce(1);
   }

    //Process Correlation Maps
    if (a.correMaps)
    {
        CorrelationMaps(myeegContainer, freqFolder);
        emit incrementAdavnce(1);
    }

    //Process Statistical Files
    if(a.statFiles && taskStatistics != nullptr)
    {
        m_triggerContainer->ProcessEventsForExperiment(taskStatistics, 99);
        if (m_triggerContainer->ProcessedTriggerCount() == 0)
        {
            emit sendLogInfo("No Trigger found for this experiment, aborting trialmats generation");
        }
        else
        {
            StatisticalFilesProcessor sfp;
            sfp.Process(m_triggerContainer, myeegContainer, taskStatistics, freqFolder, m_statOption);
        }
        emit incrementAdavnce(1);
    }

    deleteAndNullify1D(m_triggerContainer);
    EEGFormat::Utility::DeleteAndNullify(task);
    EEGFormat::Utility::DeleteAndNullify(taskInverted);
    EEGFormat::Utility::DeleteAndNullify(taskBarPlot);
    EEGFormat::Utility::DeleteAndNullify(taskStatistics);
}

void InsermLibrary::LOCA::CreateEventsFile(FrequencyBandAnalysisOpt analysisOpt, eegContainer* myeegContainer, TriggerContainer* triggerContainer, ProvFile* myprovFile)
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
        std::string rawEventsDownsampledFilePath = fileNameBase + "_raw_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + ".pos";
		std::string downsampledEventsFilePath = fileNameBase + "_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + ".pos";
        std::vector<Trigger> triggers = triggerContainer->GetTriggerForExperiment(myprovFile, 99);
        std::vector<Trigger> triggersRawDs = triggerContainer->GetTriggerForExperiment(nullptr, -1, myeegContainer->DownsamplingFactor());
		std::vector<Trigger> triggersDownsampled = triggerContainer->GetTriggerForExperiment(myprovFile, 99, myeegContainer->DownsamplingFactor());
		CreateFile(outputType, eventFilePath, triggers);
        CreateFile(outputType, rawEventsDownsampledFilePath, triggersRawDs);
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

InsermLibrary::ProvFile* InsermLibrary::LOCA::LoadProvForTask(std::string taskName, std::string analysisName)
{
    //m_currentLoca->localizerName()
    std::string taskLabel = analysisName == "" ? taskName : taskName + "_" + analysisName;
    std::string MainProvPath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/Prov/" + taskLabel + ".prov";
    if (EEGFormat::Utility::DoesFileExist(MainProvPath))
    {
        return new ProvFile(MainProvPath);
    }
    else
    {
        return nullptr;
    }
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
void InsermLibrary::LOCA::Barplot(eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = GetBarplotMapsFolder(freqFolder, myprovFile);
	std::string mapPath = PrepareFolderAndPathsBar(mapsFolder, myeegContainer->DownsamplingFactor());
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	vec1<PVALUECOORD> significantValue = ProcessKruskallStatistic(eegData3D, myeegContainer, myprovFile, mapsFolder);

	//==
	InsermLibrary::DrawbarsPlots::drawBars b = InsermLibrary::DrawbarsPlots::drawBars(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, significantValue, myeegContainer);

	delete[] windowSam;
}

std::string InsermLibrary::LOCA::GetBarplotMapsFolder(std::string freqFolder, ProvFile* myprovFile)
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

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
	mapsFolder.append(" - " + myProv[myProv.size() - 1]);

	return mapsFolder;
}

std::string InsermLibrary::LOCA::PrepareFolderAndPathsBar(std::string mapsFolder, int dsSampFreq)
{
    if(!std::filesystem::exists(mapsFolder))
    {
        std::filesystem::create_directory(mapsFolder);
    }

	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	return std::string(mapsFolder + "/" + dd[dd.size() - 2] + "_ds" + std::to_string(dsSampFreq) + "_sm0_bar_");
}

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::ProcessKruskallStatistic(vec3<float>& bigData, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder)
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
void InsermLibrary::LOCA::Env2plot(eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = GetEnv2PlotMapsFolder(freqFolder, myprovFile);
	std::string mapPath = PrepareFolderAndPathsPlot(mapsFolder, myeegContainer->DownsamplingFactor());
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(m_triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//==
	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, mapPath, m_picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, m_triggerContainer, myeegContainer, 2);

	delete[] windowSam;
}

std::string InsermLibrary::LOCA::GetEnv2PlotMapsFolder(std::string freqFolder, ProvFile* myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
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
void InsermLibrary::LOCA::TimeTrialMatrices(eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder)
{
	std::vector<PVALUECOORD> significantValue;
	//== get some useful information
	std::string mapsFolder = GetTrialmatFolder(myprovFile, freqFolder);
	std::string mapPath = PrepareFolderAndPathsTrial(mapsFolder, myeegContainer->DownsamplingFactor());
	
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> bigData;
	bigData.resize(myeegContainer->BipoleCount(), vec2<float>(m_triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocData(bigData, 0, m_triggerContainer->ProcessedTriggers(), windowSam);

	//== calculate stat
	if (ShouldPerformTrialmatStats(m_currentLoca->localizerName()))
		significantValue = ProcessWilcoxonStatistic(bigData, myeegContainer, myprovFile, mapsFolder);

	//== Draw for each plot and according to a template to reduce drawing time
	std::vector<std::tuple<int, int, int>> CodeAndTrialsIndexes = m_triggerContainer->CodeAndTrialsIndexes();
	InsermLibrary::DrawCard::mapsGenerator mGen(m_picOption->sizeTrialmap.width(), m_picOption->sizeTrialmap.height());
	mGen.trialmatTemplate(CodeAndTrialsIndexes, myprovFile);

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

		int conditionCount = static_cast<int>(CodeAndTrialsIndexes.size());
		for (int j = 0; j < conditionCount; j++)
		{
			int index = -1;
			for (int k = 0; k < myprovFile->Blocs().size(); k++)
			{
				if (myprovFile->Blocs()[k].MainSubBloc().MainEvent().Codes()[0] == std::get<0>(CodeAndTrialsIndexes[j]))
				{
					index = k;
					break;
				}
			}

			//TODO : probably check that we find an index, we should but who knows
			int StartInMs = myprovFile->Blocs()[index].MainSubBloc().MainWindow().Start();
			int EndInMs = myprovFile->Blocs()[index].MainSubBloc().MainWindow().End();
			int* currentWinMs = new int[2]{ StartInMs, EndInMs };
			//=====
			int StartInSam = (StartInMs * myeegContainer->DownsampledFrequency()) / 1000;
			int EndinSam = (EndInMs * myeegContainer->DownsampledFrequency()) / 1000;
			int* currentWinSam = new int[2]{ StartInSam, EndinSam };
			//=====
			int nbSampleWindow = currentWinSam[1] - currentWinSam[0];
			int indexBegTrigg = std::get<1>(CodeAndTrialsIndexes[j]);
			int numberSubTrial = std::get<2>(CodeAndTrialsIndexes[j]) - indexBegTrigg;
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

			auto itttt = std::find_if(mGen.subMatrixesCodes.begin(), mGen.subMatrixesCodes.end(), [&](const int& c)
			{
				return c == std::get<0>(CodeAndTrialsIndexes[j]);
			}); 
			int indexPos = std::distance(mGen.subMatrixesCodes.begin(), itttt);
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

			delete[] currentWinMs;
			delete[] currentWinSam;
		}

		mGen.drawVerticalZeroLine(painterChanel, myprovFile);

		//Display Stat
		if (m_statOption->wilcoxon)
		{
			std::vector<int> allIdCurrentMap = mGen.checkIfNeedDisplayStat(significantValue, i);
			if (allIdCurrentMap.size() > 0)
			{
				mGen.displayStatsOnMap(painterChanel, significantValue, i, myprovFile);
			}
		}

		//Display title on map and then Save
		std::string outputPicPath = mapPath;
		std::string elecName = myeegContainer->flatElectrodes[myeegContainer->Bipole(i).first];
		outputPicPath.append(elecName.c_str()).append(".jpg");
		mGen.drawMapTitle(painterChanel, outputPicPath);
		pixmapChanel->save(outputPicPath.c_str(), "JPG");
	}

    delete[] windowSam;
	deleteAndNullify1D(painterChanel);
	deleteAndNullify1D(pixmapChanel);
	deleteAndNullify1D(painterSubSubMatrix);
	deleteAndNullify1D(pixmapSubSubMatrix);

	emit sendLogInfo("Time Trials Matrices generated");
}

std::string InsermLibrary::LOCA::GetTrialmatFolder(ProvFile* myprovFile, std::string freqFolder)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	if (myprovFile->FilePath().find("INVERTED") != std::string::npos)
	{
		mapsFolder.append("_trials_resp");	//if inverted is found in the name
	}
	else
	{
		mapsFolder.append("_trials_stim");
	}

	std::stringstream streamPValue;
	streamPValue << std::fixed << std::setprecision(2) << m_statOption->pWilcoxon;
	if (m_statOption->wilcoxon)
	{
		if (m_statOption->FDRwilcoxon)
			mapsFolder.append("_FDR" + streamPValue.str());
		else
			mapsFolder.append("_P" + streamPValue.str());
	}

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
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

InsermLibrary::vec1<InsermLibrary::PVALUECOORD> InsermLibrary::LOCA::ProcessWilcoxonStatistic(vec3<float>& bigData, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder)
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
