#include "Utility.h"

vector<string> InsermLibrary::readTxtFile(string pathFile)
{
	stringstream buffer;
	ifstream file(pathFile);
	if (file)
	{
		buffer << file.rdbuf();
		file.close();
		return(split<string>(buffer.str(), "\r\n"));
	}
	else
	{
		cout << " Error opening : " << pathFile << endl;
		return vector<string>();
	}
}

void InsermLibrary::saveTxtFile(vector<QString> data, string pathFile)
{
	ofstream fichier(pathFile, ios::out);
	for (int i = 0; i < data.size(); i++)
	{
		fichier << data[i].toStdString() << endl;
	}
	fichier.close();
}

void InsermLibrary::deblankString(std::string &myString)
{
	myString.erase(remove_if(myString.begin(), myString.end(), isspace), myString.end());
}

void InsermLibrary::uiUserElement::analysis(vector<locaAnalysisOption> &analysisToRun, int nbLoca)
{
	if (analysisToRun.size() > 0)
		analysisToRun.clear();

	vector<analysisOption> dd;
	for (int i = 0; i < eeg2envCheckBox.size(); i++)
	{
		analysisOption analysisOneFrequency;
		if(eeg2envCheckBox[i]->isChecked())
			analysisOneFrequency.eeg2env = true;
		else
			analysisOneFrequency.eeg2env = false;

		if (env2plotCheckBox[i]->isChecked())
			analysisOneFrequency.env2plot = true;
		else
			analysisOneFrequency.env2plot = false;

		if (trialmapCheckBox[i]->isChecked())
			analysisOneFrequency.trialmat = true;
		else
			analysisOneFrequency.trialmat = false;

		dd.push_back(analysisOneFrequency);
	}

	for (int i = 0; i < nbLoca; i++)
	{
		locaAnalysisOption newLoca;
		newLoca.anaOpt = vector<analysisOption>(dd);
		analysisToRun.push_back(newLoca);
	}
}

InsermLibrary::freqOption::freqOption(string pathFreqFile)
{
	char buffer[256];
	int count = 0;
	frequency f;

	ifstream fichierFreq(pathFreqFile, ios::out);
	while (fichierFreq.getline(buffer, 256))
	{
		switch (count % 2)
		{
		case 0:
			f = frequency();
			f.freqName = buffer;
			count++;
			break;
		case 1:
			vector<string> splitValue = split<string>(buffer, ":");
			int fMin = atoi(splitValue[0].c_str());
			int step = atoi(splitValue[1].c_str());
			int fMax = atoi(splitValue[2].c_str());

			for (int i = 0; i <= ((fMax - fMin) / step); i++)
			{
				f.freqBandValue.push_back(fMin + (i * step));
			}
			count++;
			
			f.freqFolderName.append("f").append(splitValue[0].c_str()).append("f").append(splitValue[2].c_str());
			frequencyBands.push_back(f);
			break;
		}
	}
}

InsermLibrary::freqOption::~freqOption()
{

}