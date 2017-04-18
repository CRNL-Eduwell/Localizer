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
	int count10 = 0, count20 = 0, count30 = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 10:
			if (triggers->trigg[i].rt_code == 1)
				semanOK++;
			else if (triggers->trigg[i].rt_code == 2)
				semanNOK++;

			count10++;
			break;
		case 20:
			if (triggers->trigg[i].rt_code == 1)
				phonoOK++;
			else if (triggers->trigg[i].rt_code == 2)
				phonoNOK++;

			count20++;
			break;
		case 30:
			if (triggers->trigg[i].rt_code == 1)
				visualOK++;
			else if (triggers->trigg[i].rt_code == 2)
				visualNOK++;
			
			count30++;
			break;
		}
	}

	percentSEMAN = (semanOK / count10) * 100;
	percentPHONO = (phonoOK / count20) * 100;
	percentVISU = (visualOK / count30) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Seman OK : " << percentSEMAN << endl;
	fichierCR << "PhonoOK : " << percentPHONO << endl;
	fichierCR << "VisualOK : " << percentVISU << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << (percentPHONO + percentSEMAN + percentVISU) / 3 << endl;

	fichierCR.close();
}

void InsermLibrary::CRperf::MCSECR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double popOK = 0, searchOK = 0;
	double popNOK = 0, searchNOK = 0;
	double percentPOP = 0, percentSEARCH = 0;
	int count10 = 0, count60 = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 10:
			if (triggers->trigg[i].rt_code == 100)
				popOK++;
			else if (triggers->trigg[i].rt_code == 200)
				popNOK++;

			count10++;
			break;
		case 60:
			if (triggers->trigg[i].rt_code == 100)
				searchOK++;
			else if (triggers->trigg[i].rt_code == 200)
				searchNOK++;

			count60++;
			break;
		}
	}

	percentPOP = (popOK / count10) * 100;
	percentSEARCH = (searchOK / count60) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Pop OK : " << percentPOP << endl;
	fichierCR << "Search OK : " << percentSEARCH << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << (percentPOP + percentSEARCH) / 2 << endl;

	fichierCR.close();
}

void InsermLibrary::CRperf::MVISMVEBCR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double load2OK = 0, load4OK = 0, load6OK = 0, cntrl1OK = 0, cntrl2OK = 0, cntrl3OK = 0;
	double load2NOK = 0, load4NOK = 0, load6NOK = 0, cntrl1NOK = 0, cntrl2NOK = 0, cntrl3NOK = 0;
	double percentLOAD2 = 0, percentLOAD4 = 0, percentLOAD6 = 0, percentCTRL1 = 0, percentCTRL2 = 0, percentCTRL3 = 0;
	int count20 = 0, count40 = 0, count60 = 0, count21 = 0, count41 = 0, count61 = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 20:
			if (triggers->trigg[i].rt_code == 1)
				load2OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load2NOK++;

			count20++;
			break;
		case 40:
			if (triggers->trigg[i].rt_code == 1)
				load4OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load4NOK++;

			count40++;
			break;
		case 60:
			if (triggers->trigg[i].rt_code == 1)
				load6OK++;
			else if (triggers->trigg[i].rt_code == 2)
				load6NOK++;

			count60++;
			break;
		case 21:
			if (triggers->trigg[i].rt_code == 1)
				cntrl1OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl1NOK++;

			count21++;
			break;
		case 41:
			if (triggers->trigg[i].rt_code == 1)
				cntrl2OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl2NOK++;

			count41++;
			break;
		case 61:
			if (triggers->trigg[i].rt_code == 1)
				cntrl3OK++;
			else if (triggers->trigg[i].rt_code == 2)
				cntrl3NOK++;

			count61++;
			break;
		}
	}

	percentLOAD2 = (load2OK / count20) * 100;
	percentLOAD4 = (load4OK / count40) * 100;
	percentLOAD6 = (load6OK / count60) * 100;
	percentCTRL1 = (cntrl1OK / count21) * 100;
	percentCTRL2 = (cntrl2OK / count41) * 100;
	percentCTRL3 = (cntrl3OK / count61) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "Load2 OK : " << percentLOAD2 << endl;
	fichierCR << "Load4 OK : " << percentLOAD4 << endl;
	fichierCR << "Load6 OK : " << percentLOAD6 << endl;
	fichierCR << "Ctrl1 OK : " << percentCTRL1 << endl;
	fichierCR << "Ctrl2 OK : " << percentCTRL2 << endl;
	fichierCR << "Ctrl3 OK : " << percentCTRL3 << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << (percentLOAD2 + percentLOAD4 + percentLOAD6 + percentCTRL1 + percentCTRL2 + percentCTRL3) / 6 << endl;

	fichierCR.close();
}

void InsermLibrary::CRperf::MARA(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers)
{
	stringstream crFilePath;
	crFilePath << p_anaopt->patientFolder << "/" << p_anaopt->expTask << "/" << p_anaopt->expTask << ".CR";

	double okOK = 0, okNOK = 0;
	double nokOK = 0, nokNOK = 0;

	double percentOK = 0, percentNOK = 0;

	for (int i = 0; i < triggers->numberTrigg; i++)
	{
		switch (triggers->trigg[i].valueTrigger)
		{
		case 10:
			if (triggers->trigg[i].rt_code == 100)
				okOK++;

			break;
		case 20:
			if (triggers->trigg[i].rt_code == 200)
				nokOK++;

			break;
		}
	}

	percentOK = (okOK / triggers->numberTrigg) * 100;
	percentNOK = (nokOK / triggers->numberTrigg) * 100;

	ofstream fichierCR(crFilePath.str(), ios::out);  // ouverture en écriture avec effacement du fichier ouvert	

	fichierCR << "Nombre de triggers : " << triggers->numberTrigg << endl;
	fichierCR << "OK : " << percentOK << endl;
	fichierCR << "NOK : " << percentNOK << endl;
	fichierCR << "Pourcentage total de bonne réponse : " << percentOK << endl;

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

	stringstream buffer;
	ifstream crfile(filePath, ios::beg);

																																															      //
	if (crfile)																																																						      //
	{																																																									      //
		buffer << crfile.rdbuf();																																																		      //
		crfile.close();																																																				      //
	}																																																									      //
	else																																																								      //
	{ 																																																									      //
		cout << " Error opening CR File @ " << filePath << endl;																																					      //
	}
	vector<string> splitedFile = split<string>(buffer.str(), "\r\n");

	for (int i = 0; i < splitedFile.size(); i++)
	{
		vector<string> lineSplit = split<string>(splitedFile[i], ":");
		if (i == 0)
		{
			currentDataFile->numberOfTrigger = atoi(&lineSplit[1][0]);
		}
		else if (i == currentDataFile->numberOfConditions + 1)
		{
			currentDataFile->totalGoodAnswerPercent = atof(&lineSplit[1][0]);
		}
		else
		{
			currentDataFile->conditionPerCent[i - 1] = atof(&lineSplit[1][0]);
		}
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
	else if (dirSplit[dirSplit.size() - 1] == "MARA")
	{
		return 2;
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
	else if (dirSplit[dirSplit.size() - 1] == "MARA")
	{
		return 1;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MCSE")
	{
		return 2;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVIS")
	{
		return 3;
	}
	else if (dirSplit[dirSplit.size() - 1] == "MVEB")
	{
		return 4;
	}
	else
	{
		return -1;
	}
}

void InsermLibrary::CRperf::createCSVPerformanceReport(string rootFolder, InsermLibrary::CRData** files)
{
	ofstream fichierCR(rootFolder, ios::out);

	fichierCR << ";" << "LEC1" << ";" << "MARA" << ";" << "MCSE" << ";" << "MVIS" << ";" << "MVEB" << ";\n";
	if (files[0] != nullptr)
	{
		fichierCR << "Seman" << ";" << files[0]->conditionPerCent[0] << ";" << ";" << ";" << ";\n";
		fichierCR << "Phono" << ";" << files[0]->conditionPerCent[1] << ";" << ";" << ";" << ";\n";
		fichierCR << "Visual" << ";" << files[0]->conditionPerCent[2] << ";" << ";" << ";" << ";\n";
	}
	else
	{
		fichierCR << "Seman" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Phono" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Visual" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
	}
	if (files[1] != nullptr)
	{
		fichierCR << "OK" << ";" << ";" << files[1]->conditionPerCent[0] << "; " << "; " << "; \n";
		fichierCR << "NOK" << ";" << ";" << files[1]->conditionPerCent[1] << "; " << "; " << "; \n";
	}
	else
	{
		fichierCR << "OK" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "NOK" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
	}
	if (files[2] != nullptr)
	{
		fichierCR << "Pop" << ";" << ";" << ";" << files[2]->conditionPerCent[0] << "; " << "; " << "; \n";
		fichierCR << "Search" << ";" << ";" << ";" << files[2]->conditionPerCent[1] << "; " << "; " << "; \n";
	}
	else
	{
		fichierCR << "Pop" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Search" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
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
		fichierCR << "Load2" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Load4" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Load6" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
	}
	if (files[4] != nullptr)
	{
		fichierCR << "Load2" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[0] << "; \n";
		fichierCR << "Load4" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[1] << "; \n";
		fichierCR << "Load6" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[2] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[3] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[4] << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << ";" << ";" << ";" << files[4]->conditionPerCent[5] << "; \n";
	}
	else
	{	
		fichierCR << "Load2" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Load4" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "Load6" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
		fichierCR << "CnTrl" << ";" << ";" << "; " << "; " << "; " << "; " << "; \n";
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
	if (files[4] != nullptr)
		fichierCR << files[4]->totalGoodAnswerPercent << "%" << "; ";
	else
		fichierCR << " " << ";";
	fichierCR.close();
}