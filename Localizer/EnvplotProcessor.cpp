#include "EnvplotProcessor.h"
#include "mapsGenerator.h"
#include "barsPlotsGenerator.h"
#include <QDir>

/************/
/* Env2Plot */
/************/
void InsermLibrary::EnvplotProcessor::Process(TriggerContainer* triggerContainer, eegContainer* myeegContainer, ProvFile* myprovFile, std::string freqFolder, picOption* picOption)
{
	std::string mapsFolder = GetEnv2PlotMapsFolder(freqFolder, myprovFile);
    std::string mapPath = PrepareFolderAndPathsPlot(mapsFolder, myeegContainer);
	// Get biggest window possible, for now we use the assumption that every bloc has the same window
	// TODO : deal with possible different windows
	int StartInSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().Start() * myeegContainer->DownsampledFrequency()) / 1000;
	int EndinSam = (myprovFile->Blocs()[0].MainSubBloc().MainWindow().End() * myeegContainer->DownsampledFrequency()) / 1000;
	int* windowSam = new int[2]{ StartInSam, EndinSam };

	//== get Bloc of eeg data we want to display center around events
	vec3<float> eegData3D = vec3<float>(triggerContainer->ProcessedTriggerCount(), vec2<float>(myeegContainer->BipoleCount(), vec1<float>(windowSam[1] - windowSam[0])));
	myeegContainer->GetFrequencyBlocDataEvents(eegData3D, 0, triggerContainer->ProcessedTriggers(), windowSam);

	//==
	InsermLibrary::DrawbarsPlots::drawPlots b = InsermLibrary::DrawbarsPlots::drawPlots(myprovFile, mapPath, picOption->sizePlotmap);
	b.drawDataOnTemplate(eegData3D, triggerContainer, myeegContainer, 2);

	delete[] windowSam;
}

std::string InsermLibrary::EnvplotProcessor::GetEnv2PlotMapsFolder(std::string freqFolder, ProvFile* myprovFile)
{
	std::string mapsFolder = freqFolder;
	vec1<std::string> dd = split<std::string>(mapsFolder, "/");
	mapsFolder.append(dd[dd.size() - 1]);

	std::vector<std::string> myProv = split<std::string>(myprovFile->FilePath(), "/");
	return mapsFolder.append("_plots").append(" - " + myProv[myProv.size() - 1]);
}

std::string InsermLibrary::EnvplotProcessor::PrepareFolderAndPathsPlot(std::string mapsFolder, eegContainer* myeegContainer)
{
    if(!std::filesystem::exists(mapsFolder))
    {
        std::filesystem::create_directory(mapsFolder);
    }

    vec1<std::string> pathSplit = split<std::string>(mapsFolder, "/");
    if(myeegContainer->IsBids())
    {
        vec1<std::string> frequencySuffixSplit =  split<std::string>(pathSplit[pathSplit.size() - 1], "_");
        std::string labelName = myeegContainer->RootFileName(false, true) + "_acq-" + frequencySuffixSplit[0] + "ds" +   std::to_string(myeegContainer->DownsamplingFactor());
        return std::string(mapsFolder + "/" + labelName + "sm0plot_");
    }
    else
    {
        return std::string(mapsFolder + "/" + pathSplit[pathSplit.size() - 2] + "_ds" + std::to_string(myeegContainer->DownsamplingFactor()) + "_sm0_plot_");
    }
}
