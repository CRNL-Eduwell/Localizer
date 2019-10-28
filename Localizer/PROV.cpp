#include "PROV.h"

InsermLibrary::PROV::PROV()
{

}

InsermLibrary::PROV::PROV(std::string provFilePath)
{
	m_filePath = provFilePath;
	extractProvBloc(m_filePath);
	std::sort(visuBlocs.begin(), visuBlocs.end(),
		[](BLOC a, BLOC b) {
		return ((a.dispBloc.row() < b.dispBloc.row()) && (a.dispBloc.column() == b.dispBloc.column()));
	});

	getRightOrderBloc();
}

InsermLibrary::PROV::~PROV()
{

}

int InsermLibrary::PROV::nbCol()
{
	int max = 0;

	for (int i = 0; i < visuBlocs.size(); i++)
	{
		if (visuBlocs[i].dispBloc.column() > max)
		{
			max = visuBlocs[i].dispBloc.column();
		}
	}

	return max;
}

int InsermLibrary::PROV::nbRow()
{
	int max = 0;

	for (int i = 0; i < visuBlocs.size(); i++)
	{
		if (visuBlocs[i].dispBloc.row() > max)
		{
			max = visuBlocs[i].dispBloc.row();
		}
	}

	return max;
}

std::vector<int> InsermLibrary::PROV::getMainCodes()
{
    std::vector<int> mainEventsCode;
	for (int m = 0; m < visuBlocs.size(); m++)
	{
		if (find(mainEventsCode.begin(), mainEventsCode.end(), visuBlocs[m].mainEventBloc.eventCode[0]) == mainEventsCode.end())
		{
			mainEventsCode.push_back(visuBlocs[m].mainEventBloc.eventCode[0]);
		}
	}
	return mainEventsCode;
}

std::vector<std::vector<int>> InsermLibrary::PROV::getSecondaryCodes()
{
    std::vector<std::vector<int>> respEventsCode;
	for (int m = 0; m < visuBlocs.size(); m++)
	{
        std::vector<int> blocCodes;
		for (int j = 0; j < visuBlocs[m].secondaryEvents.size(); j++)
		{
			for (int n = 0; n < visuBlocs[m].secondaryEvents[j].eventCode.size(); n++)
			{
				if (find(blocCodes.begin(), blocCodes.end(), visuBlocs[m].secondaryEvents[j].eventCode[n]) == blocCodes.end())
				{
					blocCodes.push_back(visuBlocs[m].secondaryEvents[j].eventCode[n]); //If multiple secondary code, just use the first one of the list
				}
			}
		}
		respEventsCode.push_back(blocCodes);
	}
	return respEventsCode;
}

int *InsermLibrary::PROV::getBiggestWindowMs()
{
	int window_ms[2]{ 0,0 }; 
	for (int i = 0; i < visuBlocs.size(); i++)
	{
        window_ms[0] = std::min(window_ms[0], visuBlocs[i].dispBloc.windowMin());
        window_ms[1] = std::max(window_ms[1], visuBlocs[i].dispBloc.windowMax());
	}

	return new int[2]{ window_ms[0], window_ms[1] };
}

int *InsermLibrary::PROV::getBiggestWindowSam(int samplingFreq)
{
	int window_sam[2]{ 0,0 };
	int *window_ms = getBiggestWindowMs();
    window_sam[0] = round((samplingFreq * window_ms[0]) / 1000);
	window_sam[1] = round((samplingFreq * window_ms[1]) / 1000);
	delete window_ms;

	return new int[2]{ window_sam[0], window_sam[1] };
}

int *InsermLibrary::PROV::getWindowMs(int idBloc)
{
	return new int[2]{ visuBlocs[idBloc].dispBloc.windowMin(), visuBlocs[idBloc].dispBloc.windowMax() };
}

int *InsermLibrary::PROV::getWindowSam(int samplingFreq, int idBloc)
{
	int window_sam[2]{ 0,0 };
	int *window_ms = getWindowMs(idBloc);
	window_sam[0] = round((samplingFreq * window_ms[0]) / 1000);
	window_sam[1] = round((samplingFreq * window_ms[1]) / 1000);
	delete window_ms;

	return new int[2]{ window_sam[0], window_sam[1] };
}

//=== SETTER

void InsermLibrary::PROV::row(int idBloc, std::string rowValue)
{
	try
	{
		int value = stoi(rowValue);
		visuBlocs[idBloc].dispBloc.row(value);
	}
	catch (const std::exception&)
	{
		visuBlocs[idBloc].dispBloc.row(-1);
	}
}

void InsermLibrary::PROV::column(int idBloc, std::string columnValue)
{
	try
	{
		int value = stoi(columnValue);
		visuBlocs[idBloc].dispBloc.column(value);
	}
	catch (const std::exception&)
	{
		visuBlocs[idBloc].dispBloc.column(-1);
	}
}

void InsermLibrary::PROV::name(int idBloc, std::string nameValue)
{
	try
	{
		visuBlocs[idBloc].dispBloc.name(nameValue);
	}
	catch (const std::exception&)
	{
		visuBlocs[idBloc].dispBloc.name("");
	}
}

void InsermLibrary::PROV::path(int idBloc, std::string pathValue)
{
	try
	{
		visuBlocs[idBloc].dispBloc.path(pathValue);
	}
	catch (const std::exception&)
	{
		visuBlocs[idBloc].dispBloc.path("");
	}
}

void InsermLibrary::PROV::sort(int idBloc, std::string sortValue)
{
	try
	{
		visuBlocs[idBloc].dispBloc.sort(sortValue);
	}
	catch (const std::exception&)
	{
		visuBlocs[idBloc].dispBloc.sort("C0");
	}
}

//===

void InsermLibrary::PROV::saveFile()
{
	if (m_filePath != "")
	{
        std::ofstream provStream(m_filePath, std::ios::out);

		provStream << "ROW" << ";" << "COL" << ";" << "Name" << ";" << "Path" << ";" << "Window" << ";"
				   << "Baseline" << ";" << "Main Event" << ";" << "Main Event Label" << ";"
                   << "Secondary Events" << ";" << "Secondary Events Label" << ";" << "Sort" << std::endl;
		for (int i = 0; i < visuBlocs.size(); i++)
		{
			provStream << visuBlocs[i].dispBloc.row() << ";"
				<< visuBlocs[i].dispBloc.column() << ";"
				<< visuBlocs[i].dispBloc.name() << ";"
				<< visuBlocs[i].dispBloc.relativPath(GetCurrentWorkingDir()) << ";"
				<< visuBlocs[i].dispBloc.windowMin() << ":" << visuBlocs[i].dispBloc.windowMax() << ";"
				<< visuBlocs[i].dispBloc.baseLineMin() << ":" << visuBlocs[i].dispBloc.baseLineMax() << ";"
				<< visuBlocs[i].mainEventBloc.eventCode[0] << ";"
				<< visuBlocs[i].mainEventBloc.eventLabel << ";"
				<< visuBlocs[i].secondaryEvents[0].eventCode[0] << ";"
				<< visuBlocs[i].secondaryEvents[0].eventLabel << ";"
                << visuBlocs[i].dispBloc.sort() << std::endl;
		}

		if (changeCodeFilePath == "")
		{
            provStream << "NO_CHANGE_CODE" << std::endl;
		}
		else
		{
            provStream << changeCodeFilePath << std::endl;
		}

		if (invertmaps.epochWindow[0] == invertmaps.epochWindow[1] == invertmaps.baseLineWindow[0] == invertmaps.baseLineWindow[1] == 0)
		{
            provStream << "NO_INVERT" << std::endl;
		}
		else
		{
            provStream << invertmaps.epochWindow[0] << ":" << invertmaps.epochWindow[1] << "|" << invertmaps.baseLineWindow[0] << ":" << invertmaps.baseLineWindow[1] << std::endl;
		}

		provStream.close();
	}
}

void InsermLibrary::PROV::saveFile(std::string rootFolder, std::string fileName)
{
	std::string filePath = rootFolder + "/" + fileName + ".prov";
	if (filePath != "")
	{
        std::ofstream provStream(filePath, std::ios::out);

		provStream << "ROW" << ";" << "COL" << ";" << "Name" << ";" << "Path" << ";" << "Window" << ";"
			<< "Baseline" << ";" << "Main Event" << ";" << "Main Event Label" << ";"
            << "Secondary Events" << ";" << "Secondary Events Label" << ";" << "Sort" << std::endl;
		for (int i = 0; i < visuBlocs.size(); i++)
		{
			provStream << visuBlocs[i].dispBloc.row() << ";"
				<< visuBlocs[i].dispBloc.column() << ";"
				<< visuBlocs[i].dispBloc.name() << ";"
				<< visuBlocs[i].dispBloc.relativPath(rootFolder) << ";"
				<< visuBlocs[i].dispBloc.windowMin() << ":" << visuBlocs[i].dispBloc.windowMax() << ";"
				<< visuBlocs[i].dispBloc.baseLineMin() << ":" << visuBlocs[i].dispBloc.baseLineMax() << ";"
				<< visuBlocs[i].mainEventBloc.eventCode[0] << ";"
				<< visuBlocs[i].mainEventBloc.eventLabel << ";"
				<< visuBlocs[i].secondaryEvents[0].eventCode[0] << ";"
				<< visuBlocs[i].secondaryEvents[0].eventLabel << ";"
                << visuBlocs[i].dispBloc.sort() << std::endl;
		}

		if (changeCodeFilePath == "")
		{
            provStream << "NO_CHANGE_CODE" << std::endl;
		}
		else
		{
            provStream << changeCodeFilePath << std::endl;
		}

		if (invertmaps.epochWindow[0] == invertmaps.epochWindow[1] == invertmaps.baseLineWindow[0] == invertmaps.baseLineWindow[1] == 0)
		{
            provStream << "NO_INVERT" << std::endl;
		}
		else
		{
            provStream << invertmaps.epochWindow[0] << ":" << invertmaps.epochWindow[1] << "|" << invertmaps.baseLineWindow[0] << ":" << invertmaps.baseLineWindow[1] << std::endl;
		}

		provStream.close();
	}
}

//=== PRIVATE

void InsermLibrary::PROV::extractProvBloc(std::string provFilePath)
{
    std::string rootFolder = GetCurrentWorkingDir();

    std::vector<std::string> myAsciiData = asciiDataProv(provFilePath);
	int nbVisuBloc = (int)myAsciiData.size() - 3; //First line is legend, last line is possible changePath 

	for (int i = 0; i < nbVisuBloc; i++)
	{
		BLOC currentBloc;
        std::vector<std::string> currentAsciiBloc = split<std::string>(myAsciiData[i + 1], ";"); // First line is legend, don't care
		for (int j = 0; j < currentAsciiBloc.size(); j++)
		{
			switch (j)
			{
			case 0:
				currentBloc.dispBloc.row(atoi(&(currentAsciiBloc[0])[0]));
				break;
			case 1:
				currentBloc.dispBloc.column(atoi(&(currentAsciiBloc[1])[0]));
				break;
			case 2:
				currentBloc.dispBloc.name(currentAsciiBloc[2]);
				break;
			case 3:
                currentBloc.dispBloc.path(std::string(rootFolder + currentAsciiBloc[3]));
				break;
			case 4:
			{
                std::vector<std::string> currentEpochWin = split<std::string>(currentAsciiBloc[4], ":");
				if (currentEpochWin.size() == 2)
				{
					currentBloc.dispBloc.window(atoi(&(currentEpochWin[0])[0]), atoi(&(currentEpochWin[1])[0]));
				}
				else
				{
					currentBloc.dispBloc.window(0, 0);
                    std::cout << "Attention, probleme de fenetre" << std::endl;
				}
				break;
			}
			case 5:
			{
                std::vector<std::string> currentBaseLineWin = split<std::string>(currentAsciiBloc[5], ":");
				if (currentBaseLineWin.size() == 2)
				{
					currentBloc.dispBloc.baseLine(atoi(&(currentBaseLineWin[0])[0]), atoi(&(currentBaseLineWin[1])[0]));
				}
				else
				{
					currentBloc.dispBloc.baseLine(0, 0);
                    std::cout << "Attention, probleme de fenetre baseline" << std::endl;
				}
				break;
			}
			case 6:
			{
                std::vector<std::string> mainEventSplit = split<std::string>(currentAsciiBloc[6], "_");
				for (int k = 0; k < mainEventSplit.size(); k++)
				{
					currentBloc.mainEventBloc.eventCode.push_back(atoi(&(mainEventSplit[k])[0]));
				}
				break;
			}
			case 7:
				currentBloc.mainEventBloc.eventLabel = currentAsciiBloc[7];
				break;
			case 8:
			{
                std::vector<std::string> secondaryEventsSplit = split<std::string>(currentAsciiBloc[8], ":");
				for (int k = 0; k < secondaryEventsSplit.size(); k++)
				{
					EventBLOC currentSecondaryBloc;
                    std::vector<std::string> secondaryEventsSecondSplit = split<std::string>(secondaryEventsSplit[k], ":");
					for (int l = 0; l < secondaryEventsSecondSplit.size(); l++)
					{
						currentSecondaryBloc.eventCode.push_back(atoi(&(secondaryEventsSecondSplit[l])[0]));
					}
					currentBloc.secondaryEvents.push_back(currentSecondaryBloc);
				}
				break;
			}
			case 9:
			{
				currentBloc.secondaryEvents[currentBloc.secondaryEvents.size() - 1].eventLabel = currentAsciiBloc[9];
				break;
			}
			case 10:
				currentBloc.dispBloc.sort(currentAsciiBloc[10]);
				break;
			default:
                std::cout << "Attention, probleme" << std::endl;
				break;
			}
		}

		visuBlocs.push_back(currentBloc);
	}

	if (myAsciiData[nbVisuBloc + 1] != "NO_CHANGE_CODE")
	{
        changeCodeFilePath = std::string(rootFolder + myAsciiData[nbVisuBloc + 1]);
	}
	else
	{
		changeCodeFilePath = "";
	}

	if (myAsciiData[nbVisuBloc + 2] != "NO_INVERT")
	{
		invertmapsinfo = myAsciiData[nbVisuBloc + 2];
        std::vector<std::string>splitInvertWin = split<std::string>(invertmapsinfo, "|");

        std::vector<std::string>splitInvertEpochWin = split<std::string>(splitInvertWin[0], ":");
		invertmaps.epochWindow[0] = atoi(&splitInvertEpochWin[0][0]);
		invertmaps.epochWindow[1] = atoi(&splitInvertEpochWin[1][0]);
        std::vector<std::string>splitInvertBaseLineWin = split<std::string>(splitInvertWin[1], ":");
		invertmaps.baseLineWindow[0] = atoi(&splitInvertBaseLineWin[0][0]);
		invertmaps.baseLineWindow[1] = atoi(&splitInvertBaseLineWin[1][0]);
	}
	else
	{
		invertmapsinfo = "";
		invertmaps.epochWindow[0] = 0;
		invertmaps.epochWindow[1] = 0;
		invertmaps.baseLineWindow[0] = 0;
		invertmaps.baseLineWindow[1] = 0;
	}
}

std::vector<std::string> InsermLibrary::PROV::asciiDataProv(std::string provFilePath)
{
    std::stringstream buffer;																																																      //
    std::ifstream provFile(provFilePath, std::ios::binary);																						   											                                                  //
	if (provFile)																																																						      //
	{																																																									      //
		buffer << provFile.rdbuf();																																																		      //
		provFile.close();																																																				      //
	}																																																									      //
	else																																																								      //
	{ 																																																									      //
        std::cout << " Error opening Prov File @ " << provFilePath << std::endl;																																					      //
	}
    return split<std::string>(buffer.str(), "\r\n");
}

//Get the right order of bloc in case prov file is in disorder
void InsermLibrary::PROV::getRightOrderBloc()
{
	for (int z = 0; z < nbCol(); z++)
	{
		for (int y = 0; y < nbRow(); y++)
		{
			for (int k = 0; k < visuBlocs.size(); k++)
			{
				if ((visuBlocs[k].dispBloc.column() == z + 1) && (visuBlocs[k].dispBloc.row() == y + 1))
				{
					rightOrderBlocs.push_back(y);
				}
			}
		}
	}
}
