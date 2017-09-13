#include "patientFolder.h"

patientFolder::patientFolder(patientFolder *pat)
{
	this->pathToFolder = pat->pathToFolder;
	this->hospital = pat->hospital;
	this->year = pat->year;
	this->patientName = pat->patientName;

	localizerFolder = vector<locaFolder>(pat->localizerFolder);
}

patientFolder::patientFolder(string rootPath)
{
	pathToFolder = rootPath;
	getPatientInfo(pathToFolder);
	findLocaFolders(pathToFolder);
}

patientFolder::~patientFolder()
{
	
}

QString patientFolder::patientFullName()
{
	return QString((hospital + "_" + year + "_" + patientName).c_str());
}

void patientFolder::getPatientInfo(string path)
{
	vector<string> splitPath = split<string>(path, "/");
	vector<string> splitInfo = split<string>(splitPath[splitPath.size() - 1], "_\\/");
	hospital = splitInfo[splitInfo.size() - 3];
	year = splitInfo[splitInfo.size() - 2];
	patientName = splitInfo[splitInfo.size() - 1];
}

void patientFolder::findLocaFolders(string pathToBrowse)
{
	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(pathToBrowse.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			localizerFolder.push_back(locaFolder(this, pathToBrowse + "/" + dirname.toStdString()));
		}
	}
}

//===

locaFolder::locaFolder(patientFolder *pat, string rootPath)
{
	parent = pat;
	pathToFolder = rootPath;
	getLocaName(pathToFolder);
	retrieveFiles(pathToFolder);
	retrieveFrequencyFolders(pathToFolder);
}

locaFolder::~locaFolder()
{

}

QString locaFolder::locaFullName()
{
	return QString(parent->patientFullName() + "_" + locaName.c_str());
}

void locaFolder::getLocaName(string path)
{
	vector<string> splitPath = split<string>(path, "\\_");
	locaName = splitPath[splitPath.size() - 1];
}

void locaFolder::retrieveFiles(string path)
{
	QDir currentDir(path.c_str());
	currentDir.setNameFilters(QStringList() << "*.TRC" << "*.eeg" << "*.eeg.ent" << "*.pos");

	QRegExp rxTRC(locaFullName() + ".TRC");
	QRegExp rxEeg(locaFullName() + ".eeg");
	QRegExp rxEnt(locaFullName() + ".eeg.ent");
	QRegExp rxPos(locaFullName() + "_raw.pos");
	QRegExp rxDsPos(locaFullName() + "_ds" + "(\\d+)" + ".pos");

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{	
		if (rxTRC.exactMatch(fileFound[i]))
			trcFile = path + "/" + fileFound[i].toStdString();

		if (rxEeg.exactMatch(fileFound[i]))
			eegFile = path + "/" + fileFound[i].toStdString();

		if (rxEnt.exactMatch(fileFound[i]))
			eegEntFile = path + "/" + fileFound[i].toStdString();

		if (rxPos.exactMatch(fileFound[i]))
			posFile = path + "/" + fileFound[i].toStdString();

		if (rxDsPos.exactMatch(fileFound[i]))
			dsPosFile = path + "/" + fileFound[i].toStdString();
	}
}

void locaFolder::retrieveFrequencyFolders(string path)
{
	QRegExp rxFreqFold(locaFullName() + "_f(\\d+)f(\\d+)");

	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(path.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr("..") && rxFreqFold.exactMatch(dirname))
		{
			freqFolder.push_back(frequencyFolder(this, path + "/" + dirname.toStdString()));
		}
	}
}

//===

frequencyFolder::frequencyFolder(locaFolder *loca, string rootPath)
{
	parent = loca;
	pathToFolder = rootPath;
	getFreqBandName(pathToFolder);
	retrieveSMFile(pathToFolder);
	retrieveDataFolders(pathToFolder);
}

frequencyFolder::~frequencyFolder()
{

}

QString frequencyFolder::freqFullName()
{
	return QString(parent->locaFullName() + "_" + frequencyName.c_str());
}

bool frequencyFolder::hasTrialMap()
{
	for (int k = 0; k < dataFolders.size(); k++)
	{
		if (dataFolders[k].typeDrawing == "trials_stim" || 
			dataFolders[k].typeDrawing == "trials_resp")
		{
			return true;
		}
	}

	return false;
}

bool frequencyFolder::hasEnvBar()
{
	for (int k = 0; k < dataFolders.size(); k++)
	{
		if (dataFolders[k].typeDrawing == "bar" || dataFolders[k].typeDrawing == "plots")
		{
			return true;
		}
	}

	return false;
}

void frequencyFolder::getFreqBandName(string path)
{
	vector<string> splitPath = split<string>(path, "\\_");
	frequencyName = splitPath[splitPath.size() - 1];
}

void frequencyFolder::retrieveSMFile(string path)
{
	QDir currentDir(path.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent");

	QRegExp rxEeg0(freqFullName() + "_ds" + "(\\d+)" + "_sm0.eeg");
	QRegExp rxEeg250(freqFullName() + "_ds" + "(\\d+)" + "_sm250.eeg");
	QRegExp rxEeg500(freqFullName() + "_ds" + "(\\d+)" + "_sm500.eeg");
	QRegExp rxEeg1000(freqFullName() + "_ds" + "(\\d+)" + "_sm1000.eeg");
	QRegExp rxEeg2500(freqFullName() + "_ds" + "(\\d+)" + "_sm2500.eeg");
	QRegExp rxEeg5000(freqFullName() + "_ds" + "(\\d+)" + "_sm5000.eeg");

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		if (rxEeg0.exactMatch(fileFound[i]))
			sm0eeg = path + "/" + fileFound[i].toStdString();

		if (rxEeg250.exactMatch(fileFound[i]))
			sm250eeg = path + "/" + fileFound[i].toStdString();

		if (rxEeg500.exactMatch(fileFound[i]))
			sm500eeg = path + "/" + fileFound[i].toStdString();

		if (rxEeg1000.exactMatch(fileFound[i]))
			sm1000eeg = path + "/" + fileFound[i].toStdString();

		if (rxEeg2500.exactMatch(fileFound[i]))
			sm2500eeg = path + "/" + fileFound[i].toStdString();

		if (rxEeg5000.exactMatch(fileFound[i]))
			sm5000eeg = path + "/" + fileFound[i].toStdString();
	}
}

void frequencyFolder::retrieveDataFolders(string path)
{
	QDir currentDir(path.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			dataFolders.push_back(analyzedDataFolder(this, path + "/" + dirname.toStdString()));
		}
	}
}

//===

analyzedDataFolder::analyzedDataFolder(frequencyFolder *freq, string rootPath)
{
	parent = freq;
	pathToFolder = rootPath;
	getStatsInfo(pathToFolder);
	getPicturesFolder(pathToFolder);
}

analyzedDataFolder::~analyzedDataFolder()
{

}

void analyzedDataFolder::getStatsInfo(string path)
{
	vector<string> splitPath = split<string>(path, "/");
	vector<string> splitFolder = split<string>(splitPath[splitPath.size() - 1], "_");

	if (splitFolder[5] == "trials")
	{
		typeDrawing = splitFolder[5];

		if (splitFolder.size() - 1 == 6)
		{
			typeDrawing = splitFolder[5] + "_" + splitFolder[6];
		}

		if (splitFolder.size() - 1 == 7)
		{
			typeDrawing = splitFolder[5] + "_" + splitFolder[6];
			if (splitFolder[7] != "")
				StatAnalysis = splitFolder[7];
		}
	}
	else if (splitFolder[5] == "bar")
	{
		typeDrawing = splitFolder[5];

		if (splitFolder.size() - 1 == 6)
		{
			if (splitFolder[6] != "")
			{
				StatAnalysis = splitFolder[6];
			}
		}
	}
	else if (splitFolder[5] == "plots")
	{
		typeDrawing = splitFolder[5];
	}
}

void analyzedDataFolder::getPicturesFolder(string path)
{
	QDir currentDir(path.c_str());
	currentDir.setFilter(QDir::Files);
	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			vector<string> splitVal = split<string>(dirname.toStdString(), "_.");
			picData currentPic;
			currentPic.sortingWeight = strToWeightInt(splitVal[splitVal.size() - 2]);
			currentPic.pathToPic = path + "/" + dirname.toStdString();
			pictures.push_back(currentPic);
		}
	}

	std::sort(pictures.begin(), pictures.end(),
		[](picData firstValue, picData secondValue) {
		return (firstValue.sortingWeight < secondValue.sortingWeight);
	});
}

int analyzedDataFolder::strToWeightInt(string myStr)
{
	int myVal = 0;
	for (int i = 0; i < myStr.length(); i++)
	{
		if (isdigit(myStr[i]))
		{
			myVal += (int)(myStr[i]);
		}
		else
		{
			myVal += (int)(myStr[i]) + (100 * (int)(myStr[i]));
		}
	}
	return myVal;
}