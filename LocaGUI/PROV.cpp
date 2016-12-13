#include "PROV.h"

InsermLibrary::PROV::PROV(string provFilePath)
{
	filePath = provFilePath;
	extractProvBloc(filePath);
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

void InsermLibrary::PROV::extractProvBloc(string provFilePath)
{
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
				currentBloc.dispBloc.path = currentAsciiBloc[3];
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
		changeCodeFilePath = myAsciiData[nbVisuBloc + 1];
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