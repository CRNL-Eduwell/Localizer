#include "PROV.h"

InsermLibrary::PROV::PROV(string provFilePath)
{
	filePath = provFilePath;
	extractProvBloc(filePath);
	sort(visuBlocs.begin(), visuBlocs.end(),
		[](BLOC a, BLOC b) {
		return ((a.dispBloc.row < b.dispBloc.row) && (a.dispBloc.col == b.dispBloc.col));
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
		if (visuBlocs[i].dispBloc.col > max)
		{
			max = visuBlocs[i].dispBloc.col;
		}
	}

	return max;
}

int InsermLibrary::PROV::nbRow()
{
	int max = 0;

	for (int i = 0; i < visuBlocs.size(); i++)
	{
		if (visuBlocs[i].dispBloc.row > max)
		{
			max = visuBlocs[i].dispBloc.row;
		}
	}

	return max;
}

vector<int> InsermLibrary::PROV::getMainCodes()
{
	vector<int> mainEventsCode;
	for (int m = 0; m < visuBlocs.size(); m++)
	{
		if (find(mainEventsCode.begin(), mainEventsCode.end(), visuBlocs[m].mainEventBloc.eventCode[0]) == mainEventsCode.end())
		{
			mainEventsCode.push_back(visuBlocs[m].mainEventBloc.eventCode[0]);
		}
	}
	return mainEventsCode;
}

vector<int> InsermLibrary::PROV::getSecondaryCodes()
{
	vector<int> respEventsCode;
	for (int m = 0; m < visuBlocs.size(); m++)
	{
		for (int j = 0; j < visuBlocs[m].secondaryEvents.size(); j++)
		{
			if (find(respEventsCode.begin(), respEventsCode.end(), visuBlocs[m].secondaryEvents[j].eventCode[0]) == respEventsCode.end())
			{
				respEventsCode.push_back(visuBlocs[m].secondaryEvents[j].eventCode[0]); //If multiple secondary code, just use the first one of the list
			}
		}
	}
	return respEventsCode;
}

int *InsermLibrary::PROV::getBiggestWindowMs()
{
	int window_ms[2]{ 0,0 }; 
	for (int i = 0; i < visuBlocs.size(); i++)
	{
		window_ms[0] = min(window_ms[0], visuBlocs[i].dispBloc.epochWindow[0]);
		window_ms[1] = max(window_ms[1], visuBlocs[i].dispBloc.epochWindow[1]);
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
	return new int[2]{ visuBlocs[idBloc].dispBloc.epochWindow[0], visuBlocs[idBloc].dispBloc.epochWindow[1] };
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

void InsermLibrary::PROV::extractProvBloc(string provFilePath)
{
	string rootFolder = GetCurrentWorkingDir();

	vector<string> myAsciiData = asciiDataProv(provFilePath);
	int nbVisuBloc = (int)myAsciiData.size() - 3; //First line is legend, last line is possible changePath 

	for (int i = 0; i < nbVisuBloc; i++)
	{
		BLOC currentBloc;
		vector<string> currentAsciiBloc = split<string>(myAsciiData[i + 1], ";"); // First line is legend, don't care
		for (int j = 0; j < currentAsciiBloc.size(); j++)
		{
			switch (j)
			{
			case 0:
				currentBloc.dispBloc.row = atoi(&(currentAsciiBloc[0])[0]);
				break;
			case 1:
				currentBloc.dispBloc.col = atoi(&(currentAsciiBloc[1])[0]);
				break;
			case 2:
				currentBloc.dispBloc.name = currentAsciiBloc[2];
				break;
			case 3:
				currentBloc.dispBloc.path = string(rootFolder + currentAsciiBloc[3]);
				break;
			case 4:
			{
				vector<string> currentEpochWin = split<string>(currentAsciiBloc[4], ":");
				if (currentEpochWin.size() == 2)
				{
					currentBloc.dispBloc.epochWindow[0] = atoi(&(currentEpochWin[0])[0]);
					currentBloc.dispBloc.epochWindow[1] = atoi(&(currentEpochWin[1])[0]);
				}
				else
				{
					currentBloc.dispBloc.epochWindow[0] = 0;
					currentBloc.dispBloc.epochWindow[1] = 0;
					cout << "Attention, probleme de fenetre" << endl;
				}
				break;
			}
			case 5:
			{
				vector<string> currentBaseLineWin = split<string>(currentAsciiBloc[5], ":");
				if (currentBaseLineWin.size() == 2)
				{
					currentBloc.dispBloc.baseLineWindow[0] = atoi(&(currentBaseLineWin[0])[0]);
					currentBloc.dispBloc.baseLineWindow[1] = atoi(&(currentBaseLineWin[1])[0]);
				}
				else
				{
					currentBloc.dispBloc.baseLineWindow[0] = 0;
					currentBloc.dispBloc.baseLineWindow[1] = 0;
					cout << "Attention, probleme de fenetre baseline" << endl;
				}
				break;
			}
			case 6:
			{
				vector<string> mainEventSplit = split<string>(currentAsciiBloc[6], "_");
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
				vector<string> secondaryEventsSplit = split<string>(currentAsciiBloc[8], ":");
				for (int k = 0; k < secondaryEventsSplit.size(); k++)
				{
					EventBLOC currentSecondaryBloc;
					vector<string> secondaryEventsSecondSplit = split<string>(secondaryEventsSplit[k], ":");
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
				currentBloc.dispBloc.sort = currentAsciiBloc[10];
				break;
			default:
				cout << "Attention, probleme" << endl;
				break;
			}
		}

		visuBlocs.push_back(currentBloc);
	}

	if (myAsciiData[nbVisuBloc + 1] != "NO_CHANGE_CODE")
	{
		changeCodeFilePath = string(rootFolder + myAsciiData[nbVisuBloc + 1]);
	}
	else
	{
		changeCodeFilePath = "";
	}

	if (myAsciiData[nbVisuBloc + 2] != "NO_INVERT")
	{
		invertmapsinfo = myAsciiData[nbVisuBloc + 2];
		vector<string>splitInvertWin = split<string>(invertmapsinfo, "|");

		vector<string>splitInvertEpochWin = split<string>(splitInvertWin[0], ":");
		invertmaps.epochWindow[0] = atoi(&splitInvertEpochWin[0][0]);
		invertmaps.epochWindow[1] = atoi(&splitInvertEpochWin[1][0]);
		vector<string>splitInvertBaseLineWin = split<string>(splitInvertWin[1], ":");
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

vector<string> InsermLibrary::PROV::asciiDataProv(string provFilePath)
{
	stringstream buffer;																																																      //
	ifstream provFile(provFilePath, ios::binary);																						   											                                                  //
	if (provFile)																																																						      //
	{																																																									      //
		buffer << provFile.rdbuf();																																																		      //
		provFile.close();																																																				      //
	}																																																									      //
	else																																																								      //
	{ 																																																									      //
		cout << " Error opening Prov File @ " << provFilePath << endl;																																					      //
	}
	return split<string>(buffer.str(), "\r\n");
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
				if ((visuBlocs[k].dispBloc.col == z + 1) && (visuBlocs[k].dispBloc.row == y + 1))
				{
					rightOrderBlocs.push_back(y);
				}
			}
		}
	}
}