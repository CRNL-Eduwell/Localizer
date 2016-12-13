#include "CRperf.h"

InsermLibrary::CRData::CRData(int nbConditions)
{
	numberOfConditions = nbConditions;
	conditionPerCent = new double[numberOfConditions]();
}

InsermLibrary::CRData::~CRData()
{
	delete[] conditionPerCent;
}

void InsermLibrary::CRperf::LEC1CR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double visualOK = 0, phonoOK = 0, semanOK = 0;
	double visualNOK = 0, phonoNOK = 0, semanNOK = 0;
	double percentVISU = 0, percentPHONO = 0, percentSEMAN = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 10:
			if (triggers->trigg[i].rt_code == 1)
				semanOK++;
			else if (triggers->trigg[i].rt_code == 2)
				semanNOK++;
			break;
		case 20:
			if (triggers->trigg[i].rt_code == 1)
				phonoOK++;
			else if (triggers->trigg[i].rt_code == 2)
				phonoNOK++;
			break;
		case 30:
			if (triggers->trigg[i].rt_code == 1)
				visualOK++;
			else if (triggers->trigg[i].rt_code == 2)
				visualNOK++;
			break;
		}
	}

	percentSEMAN = (semanOK / triggers->numberTrigg) * 100;
	percentPHONO = (phonoOK / triggers->numberTrigg) * 100;
	percentVISU = (visualOK / triggers->numberTrigg) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Seman OK : " << percentSEMAN << endl;
	fichierCR << "PhonoOK : " << percentPHONO << endl;
	fichierCR << "VisualOK : " << percentVISU << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << percentPHONO + percentSEMAN + percentVISU << endl;

	fichierCR.close();
}

void InsermLibrary::CRperf::MCSECR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double popOK = 0, searchOK = 0;
	double popNOK = 0, searchNOK = 0;
	double percentPOP = 0, percentSEARCH = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 10:
			if (triggers->trigg[i].rt_code == 100)
				popOK++;
			else if (triggers->trigg[i].rt_code == 200)
				popNOK++;
			break;
		case 60:
			if (triggers->trigg[i].rt_code == 100)
				searchOK++;
			else if (triggers->trigg[i].rt_code == 200)
				searchNOK++;
			break;
		}
	}

	percentPOP = (popOK / triggers->numberTrigg) * 100;
	percentSEARCH = (searchOK / triggers->numberTrigg) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Pop OK : " << percentPOP << endl;
	fichierCR << "Search OK : " << percentSEARCH << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << percentPOP + percentSEARCH << endl;

	fichierCR.close();
}

void InsermLibrary::CRperf::MVISMVEBCR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double load2OK = 0, load4OK = 0, load6OK = 0, cntrl1OK = 0, cntrl2OK = 0, cntrl3OK = 0;
	double load2NOK = 0, load4NOK = 0, load6NOK = 0, cntrl1NOK = 0, cntrl2NOK = 0, cntrl3NOK = 0;
	double percentLOAD2 = 0, percentLOAD4 = 0, percentLOAD6 = 0, percentCTRL1 = 0, percentCTRL2 = 0, percentCTRL3 = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 20:
			if (triggers->trigg[i].rt_code == 1)
				load2OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load2NOK++;
			break;
		case 40:
			if (triggers->trigg[i].rt_code == 1)
				load4OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load4NOK++;
			break;
		case 60:
			if (triggers->trigg[i].rt_code == 1)
				load6OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load6NOK++;
			break;
		case 21:
			if (triggers->trigg[i].rt_code == 1)
				cntrl1OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl1NOK++;
			break;
		case 41:
			if (triggers->trigg[i].rt_code == 1)
				cntrl2OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl2NOK++;
			break;
		case 61:
			if (triggers->trigg[i].rt_code == 1)
				cntrl3OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl3NOK++;
			break;
		}
	}

	percentLOAD2 = (load2OK / triggers->numberTrigg) * 100;
	percentLOAD4 = (load4OK / triggers->numberTrigg) * 100;
	percentLOAD6 = (load6OK / triggers->numberTrigg) * 100;
	percentCTRL1 = (cntrl1OK / triggers->numberTrigg) * 100;
	percentCTRL2 = (cntrl2OK / triggers->numberTrigg) * 100;
	percentCTRL3 = (cntrl3OK / triggers->numberTrigg) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Load2 OK : " << percentLOAD2 << endl;
	fichierCR << "Load4 OK : " << percentLOAD4 << endl;
	fichierCR << "Load6 OK : " << percentLOAD6 << endl;
	fichierCR << "Ctrl1 OK : " << percentCTRL1 << endl;
	fichierCR << "Ctrl2 OK : " << percentCTRL2 << endl;
	fichierCR << "Ctrl3 OK : " << percentCTRL3 << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << percentLOAD2 + percentLOAD4 + percentLOAD6 + percentCTRL1 + percentCTRL2 + percentCTRL3 << endl;

	fichierCR.close();
}

vector<string> InsermLibrary::CRperf::getAllFolderNames(string rootPath)
{
	vector<string> directoryList;

	QDir currentDir(QString::fromStdString(rootPath));
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			directoryList.push_back(dirname.toStdString());
		}
	}

	return directoryList;
}

InsermLibrary::CRData* InsermLibrary::CRperf::getCRInfo(string filePath, string rootName, int numberConditions)
{
	int count = 0;
	string line;
	CRData *currentDataFile = new CRData(numberConditions);

	fstream crfile(filePath, ios::beg);
	while (getline(crfile, line))
	{
		vector<string> sortSplit = split<string>(line, ":");
		if (count == 0)
		{
			currentDataFile->numberOfTrigger = atoi(&sortSplit[1][0]);
		}
		else if (count == currentDataFile->numberOfConditions + 1)
		{
			currentDataFile->totalGoodAnswerPercent = atof(&sortSplit[1][0]);
		}
		else
		{
			currentDataFile->conditionPerCent[count - 1] = atof(&sortSplit[1][0]);
		}
		count++;
	}

	return currentDataFile;
}

int InsermLibrary::CRperf::whichOneAmI(string compareMe)
{
	vector<string> dirSplit = split<string>(compareMe, "_");

	if (dirSplit[dirSplit.size() - 1] == "LEC1")
	{
		return 3;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MCSE")
	{
		return 2;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVIS")
	{
		return 6;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVEB")
	{
		return 6;
	}
	else
	{
		return -1;
	}
}

int InsermLibrary::CRperf::whereAmI(string compareMe)
{
	vector<string> dirSplit = split<string>(compareMe, "_");

	if (dirSplit[dirSplit.size() - 1] == "LEC1")
	{
		return 0;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MCSE")
	{
		return 1;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVIS")
	{
		return 2;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVEB")
	{
		return 3;
	}
	else
	{
		return -1;
	}
}

void InsermLibrary::CRperf::createCSVPerformanceReport(string rootFolder, InsermLibrary::CRData** files)
{
	ofstream fichierCR(rootFolder, ios::out);

	fichierCR << ";" << "LEC1" << ";" << "MCSE" << ";" << "MVEB" << ";" << "MVIS" << ";\n";
	if (files[0] != nullptr)
	{
		fichierCR << "Seman" << ";" << files[0]->conditionPerCent[0] << ";" << ";" << ";" << ";\n";
		fichierCR << "Phono" << ";" << files[0]->conditionPerCent[1] << ";" << ";" << ";" << ";\n";
		fichierCR << "Visual" << ";" << files[0]->conditionPerCent[2] << ";" << ";" << ";" << ";\n";
	}
	else
	{
		fichierCR << "Seman" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "Phono" << ";" << ";" << ";" << ";" << ";" << ";\n";
		fichierCR << "Visual" << ";" << ";" << ";" << ";" << ";" << ";\n";
	}
	if (files[1] != nullptr)
	{
		fichierCR << "Pop" << ";" << ";" << files[1]->conditionPerCent[0] << "; " << "; " << "; \n";
		fichierCR << "Search" << ";" << ";" << files[1]->conditionPerCent[1] << "; " << "; " << "; \n";
	}
	else
	{
		fichierCR << "Pop" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "Search" << ";" << ";" << ";" << ";" << ";" << ";\n";
	}
	if (files[2] != nullptr)
	{
		fichierCR << "Load2" << ";" << ";" << ";" << files[2]->conditionPerCent[0] << "; \n";
		fichierCR << "Load4" << ";" << ";" << ";" << files[2]->conditionPerCent[1] << "; \n";
		fichierCR << "Load6" << ";" << ";" << ";" << files[2]->conditionPerCent[2] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << files[2]->conditionPerCent[3] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << files[2]->conditionPerCent[4] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << files[2]->conditionPerCent[5] << "; \n";
	}
	else
	{
		fichierCR << "Load2" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "Load4" << ";" << ";" << ";" << ";" << ";" << ";\n";
		fichierCR << "Load6" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << ";\n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << ";\n";
	}
	if (files[3] != nullptr)
	{
		fichierCR << "Load2" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[0] << "; \n";
		fichierCR << "Load4" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[1] << "; \n";
		fichierCR << "Load6" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[2] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[3] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[4] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << files[3]->conditionPerCent[5] << "; \n";
	}
	else
	{
		fichierCR << "Load2" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "Load4" << ";" << ";" << ";" << ";" << ";" << ";\n";
		fichierCR << "Load6" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << ";\n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << ";\n";
	}

	fichierCR << "Total OK" << ";";
	
	if (files[0] != nullptr)
		fichierCR << files[0]->totalGoodAnswerPercent << "%" << "; ";
	else
		fichierCR << " " << ";";
	if (files[1] != nullptr)
		fichierCR << files[1]->totalGoodAnswerPercent << "%" << "; ";
	else
		fichierCR << " " << ";";
	if (files[2] != nullptr)
		fichierCR << files[2]->totalGoodAnswerPercent << "%" << "; ";
	else
		fichierCR << " " << ";";
	if (files[3] != nullptr)
		fichierCR << files[3]->totalGoodAnswerPercent << "%" << "; ";
	else
		fichierCR << " " << ";";

	fichierCR.close();
}