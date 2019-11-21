#include "patientFolder.h"

using namespace InsermLibrary;

patientFolder::patientFolder(patientFolder *pat)
{
	this->m_rootFolder = pat->m_rootFolder;
	this->m_hospital = pat->m_hospital;
	this->m_year = pat->m_year;
	this->m_patientName = pat->m_patientName;

	m_localizerFolder = std::vector<locaFolder>(pat->m_localizerFolder);
}

patientFolder::patientFolder(std::string rootPath)
{
	std::vector<std::string> splitExt = split<std::string>(rootPath, "\\/");
	if (rootPath[0] == '/' && rootPath[1] == '/') //In case this is a newtork ressource
	{
		m_rootFolder = "//";
	}
	else
	{
		if (rootPath[0] == '/') //In case this is a linux path
			m_rootFolder = "/";
	}

	for (int i = 0; i < splitExt.size(); i++)
		m_rootFolder += splitExt[i] + "/";
	getPatientInfo(m_rootFolder);
	findLocaFolders(m_rootFolder);
}

patientFolder::~patientFolder()
{
	
}

std::string patientFolder::patientName()
{
	return (m_hospital + "_" + m_year + "_" + m_patientName);
}

void patientFolder::getPatientInfo(std::string rootPath)
{
	std::vector<std::string> splitPath = split<std::string>(rootPath, "_\\/");
	m_hospital = splitPath[splitPath.size() - 3];
	m_year = splitPath[splitPath.size() - 2];
	m_patientName = splitPath[splitPath.size() - 1];
}

void patientFolder::findLocaFolders(std::string rootPath)
{
	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(rootPath.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			m_localizerFolder.push_back(locaFolder(this, rootPath + dirname.toStdString() + "/"));
		}
	}
}

//===

locaFolder::locaFolder(patientFolder *pat, std::string rootLocaFolder)
{
	parent = pat;
	m_rootLocaFolder = rootLocaFolder;
	getLocaName(m_rootLocaFolder);
	retrieveFiles(m_rootLocaFolder);
	retrieveFrequencyFolders(m_rootLocaFolder);
}

locaFolder::~locaFolder()
{

}

std::string locaFolder::fullLocalizerName()
{
	return parent->patientName() + "_" + m_locaName;
}

FileExt locaFolder::fileExtention()
{
	if (m_trcFile != "")
		return TRC;
	else if (m_eegFile != "")
		return EEG_ELAN;
	else if (m_edfFile != "")
		return EDF;
	else
		return NO_EXT;
}

std::string locaFolder::filePath(FileExt wantedFile)
{
	switch (wantedFile)
	{
	case TRC:
		return m_trcFile;
	case EEG_ELAN:
		return m_eegFile;
	case ENT_ELAN:
		return m_eegEntFile;
	case POS_ELAN:
		return m_posFile;
	case POS_DS_ELAN:
		return m_dsPosFile;
	case EDF:
		return m_edfFile;
	case NO_EXT:
		return "";
	default:
		return "";
	}
	//return "";
}

void locaFolder::getLocaName(std::string rootLocaFolder)
{
	std::vector<std::string> splitPath = split<std::string>(rootLocaFolder, "\\/_");
	m_locaName = splitPath[splitPath.size() - 1];
}

void locaFolder::retrieveFiles(std::string rootLocaFolder)
{
	QDir currentDir(rootLocaFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.trc" << "*.eeg" << "*.eeg.ent" << "*.pos" << "*.edf");

	QRegExp rxTRC((fullLocalizerName() + ".trc").c_str(), Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEeg((fullLocalizerName() + ".eeg").c_str(), Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEnt((fullLocalizerName() + ".eeg.ent").c_str(), Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxPos((fullLocalizerName() + "_raw.pos").c_str(), Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxDsPos((fullLocalizerName() + "_ds" + "(\\d+)" + ".pos").c_str(), Qt::CaseSensitivity::CaseInsensitive);
	QRegExp rxEdf((fullLocalizerName() + ".edf").c_str(), Qt::CaseSensitivity::CaseInsensitive);

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{	
		if (rxTRC.exactMatch(fileFound[i]))
			m_trcFile = rootLocaFolder + fileFound[i].toStdString();

		if (rxEeg.exactMatch(fileFound[i]))
			m_eegFile = rootLocaFolder + fileFound[i].toStdString();

		if (rxEnt.exactMatch(fileFound[i]))
			m_eegEntFile = rootLocaFolder + fileFound[i].toStdString();

		if (rxPos.exactMatch(fileFound[i]))
			m_posFile = rootLocaFolder + fileFound[i].toStdString();

		if (rxDsPos.exactMatch(fileFound[i]))
			m_dsPosFile = rootLocaFolder + fileFound[i].toStdString();

		if (rxEdf.exactMatch(fileFound[i]))
			m_edfFile = rootLocaFolder + fileFound[i].toStdString();
	}
}

void locaFolder::retrieveFrequencyFolders(std::string rootLocaFolder)
{
	QRegExp rxFreqFold((fullLocalizerName() + "_f(\\d+)f(\\d+)").c_str());

	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(rootLocaFolder.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr("..") && rxFreqFold.exactMatch(dirname))
		{
			m_freqFolder.push_back(frequencyFolder(this, rootLocaFolder + dirname.toStdString() + "/"));
		}
	}
}

//===

frequencyFolder::frequencyFolder(locaFolder *loca, std::string rootFrequencyPath)
{
	parent = loca;
	m_rootFrequencyFolder = rootFrequencyPath;
	getFreqBandName(m_rootFrequencyFolder);
	retrieveSMFile(m_rootFrequencyFolder);
	retrieveDataFolders(m_rootFrequencyFolder);
}

frequencyFolder::~frequencyFolder()
{

}

std::string frequencyFolder::fullFrequencyName()
{
	return parent->fullLocalizerName() + "_" + m_frequencyName;
}

std::string frequencyFolder::filePath(FileExt wantedFile)
{
	switch (wantedFile)
	{
	case SM0_ELAN:
		return m_sm0eeg;
	case SM250_ELAN:
		return m_sm250eeg;
	case SM500_ELAN:
		return m_sm500eeg;
	case SM1000_ELAN:
		return m_sm1000eeg;
	case SM2500_ELAN:
		return m_sm2500eeg;
	case SM5000_ELAN:
		return m_sm5000eeg;
	default:
		return "";
	}
}

//In the case where the user has generated ElanFile and then BrainVision File
//or the other way around , we will have two header files and one data, for now 
//there is no way to tell which one is the correct one so we send back
//no paths , in the future, we might try to open the files to tell which one 
//is the correct one
std::vector<std::string> frequencyFolder::FilePaths(FileExt wantedFile)
{
	switch (wantedFile)
	{
	case SM0_ELAN:
		return m_smXFiles.size() > 0 ? GetFirstFullDataSet(m_smXFiles[0]) : std::vector<std::string>();
	case SM250_ELAN:
		return m_smXFiles.size() > 1 ? GetFirstFullDataSet(m_smXFiles[1]) : std::vector<std::string>();
	case SM500_ELAN:
		return m_smXFiles.size() > 2 ? GetFirstFullDataSet(m_smXFiles[2]) : std::vector<std::string>();
	case SM1000_ELAN:
		return m_smXFiles.size() > 3 ? GetFirstFullDataSet(m_smXFiles[3]) : std::vector<std::string>();
	case SM2500_ELAN:
		return m_smXFiles.size() > 4 ? GetFirstFullDataSet(m_smXFiles[4]) : std::vector<std::string>();
	case SM5000_ELAN:
		return m_smXFiles.size() > 5 ? GetFirstFullDataSet(m_smXFiles[5]) : std::vector<std::string>();
	default:
		return std::vector<std::string>();
	}
}

bool frequencyFolder::hasTrialMap()
{
	for (int k = 0; k < m_dataFolders.size(); k++)
	{
		if (m_dataFolders[k].typeDrawing() == "trials_stim" ||
			m_dataFolders[k].typeDrawing() == "trials_resp")
		{
			return true;
		}
	}

	return false;
}

bool frequencyFolder::hasEnvBar()
{
	for (int k = 0; k < m_dataFolders.size(); k++)
	{
		if (m_dataFolders[k].typeDrawing() == "bar" || m_dataFolders[k].typeDrawing() == "plots")
		{
			return true;
		}
	}

	return false;
}

void frequencyFolder::getFreqBandName(std::string rootFreqFolder)
{
	std::vector<std::string> splitPath = split<std::string>(rootFreqFolder, "\\/_");
	m_frequencyName = splitPath[splitPath.size() - 1];
}

void frequencyFolder::retrieveSMFile2(std::string rootFreqFolder)
{
	QDir currentDir(rootFreqFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent");

	QRegExp rxEeg0((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm0.eeg").c_str());
	QRegExp rxEeg250((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm250.eeg").c_str());
	QRegExp rxEeg500((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm500.eeg").c_str());
	QRegExp rxEeg1000((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm1000.eeg").c_str());
	QRegExp rxEeg2500((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm2500.eeg").c_str());
	QRegExp rxEeg5000((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm5000.eeg").c_str());

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		if (rxEeg0.exactMatch(fileFound[i]))
			m_sm0eeg = rootFreqFolder + fileFound[i].toStdString();

		if (rxEeg250.exactMatch(fileFound[i]))
			m_sm250eeg = rootFreqFolder + fileFound[i].toStdString();

		if (rxEeg500.exactMatch(fileFound[i]))
			m_sm500eeg = rootFreqFolder + fileFound[i].toStdString();

		if (rxEeg1000.exactMatch(fileFound[i]))
			m_sm1000eeg = rootFreqFolder + fileFound[i].toStdString();

		if (rxEeg2500.exactMatch(fileFound[i]))
			m_sm2500eeg = rootFreqFolder + fileFound[i].toStdString();

		if (rxEeg5000.exactMatch(fileFound[i]))
			m_sm5000eeg = rootFreqFolder + fileFound[i].toStdString();
	}
}

void frequencyFolder::retrieveSMFile(std::string rootFreqFolder)
{
	QDir currentDir(rootFreqFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent" << "*.vhdr");

	std::vector<QRegExp> rxEegSmX;
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm0") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm250") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm500") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm1000") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm2500") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegExp(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm5000") + "(.eeg|.eeg.ent|.vhdr)").c_str()));

	m_smXFiles.resize(6, std::vector<std::string>());
	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (rxEegSmX[j].exactMatch(fileFound[i]))
			{
				m_smXFiles[j].push_back(rootFreqFolder + fileFound[i].toStdString());
			}
		}
	}
}

void frequencyFolder::retrieveDataFolders(std::string rootFreqFolder)
{
	QDir currentDir(rootFreqFolder.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			m_dataFolders.push_back(analyzedDataFolder(this, rootFreqFolder + dirname.toStdString() + "/"));
		}
	}
}

std::vector<std::string> frequencyFolder::GetFirstFullDataSet(const std::vector<std::string>& rawFilePaths)
{
	std::vector<std::string> FileExtensions;
	for (int i = 0; i < rawFilePaths.size(); i++)
	{
		FileExtensions.push_back(EEGFormat::Utility::GetFileExtension(rawFilePaths[i]));
	}

	if(FileExtensions.size() > 0 && FileExtensions.size() < 3)
	{
		std::vector<std::string> DataSet;
		auto itBV = std::find_if(FileExtensions.begin(), FileExtensions.end(), [&](const std::string& str) { return str.compare("vhdr") == 0; });
		if (itBV != FileExtensions.end())
		{
			int id = std::distance(FileExtensions.begin(), itBV);
			DataSet.push_back(rawFilePaths[id]);
			return DataSet;
		}
		else
		{
			for (int i = 0; i < FileExtensions.size(); i++)
			{
				DataSet.push_back(rawFilePaths[i]);
			}
			return DataSet;
		}

	}
	else
	{
		return std::vector<std::string>();
	}
}

//===

analyzedDataFolder::analyzedDataFolder(frequencyFolder *freq, std::string rootPath)
{
	parent = freq;
	m_rootDataFolder = rootPath;
	getStatsInfo(m_rootDataFolder);
	getPicturesFolder(m_rootDataFolder);
}

analyzedDataFolder::~analyzedDataFolder()
{

}

void analyzedDataFolder::getStatsInfo(std::string rootDataFolder)
{
	std::vector<std::string> splitPath = split<std::string>(rootDataFolder, "/");
	std::vector<std::string> splitFolder = split<std::string>(splitPath[splitPath.size() - 1], "_");

	if (splitFolder[5] == "trials")
	{
		m_typeDrawing = splitFolder[5];

		if (splitFolder.size() - 1 == 6)
		{
			m_typeDrawing = splitFolder[5] + "_" + splitFolder[6];
		}

		if (splitFolder.size() - 1 == 7)
		{
			m_typeDrawing = splitFolder[5] + "_" + splitFolder[6];
			if (splitFolder[7] != "")
				m_StatAnalysis = splitFolder[7];
		}
	}
	else if (splitFolder[5] == "bar")
	{
		m_typeDrawing = splitFolder[5];

		if (splitFolder.size() - 1 == 6)
		{
			if (splitFolder[6] != "")
			{
				m_StatAnalysis = splitFolder[6];
			}
		}
	}
	else if (splitFolder[5] == "plots")
	{
		m_typeDrawing = splitFolder[5];
	}
}

void analyzedDataFolder::getPicturesFolder(std::string rootDataFolder)
{
	QDir currentDir(rootDataFolder.c_str());
	currentDir.setFilter(QDir::Files);
	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			std::vector<std::string> splitVal = split<std::string>(dirname.toStdString(), "_.");
			picData currentPic;
			currentPic.sortingWeight = strToWeightInt(splitVal[splitVal.size() - 2]);
			currentPic.pathToPic = rootDataFolder + "/" + dirname.toStdString();
			m_pictures.push_back(currentPic);
		}
	}

	std::sort(m_pictures.begin(), m_pictures.end(),
		[](picData firstValue, picData secondValue) {
		return (firstValue.sortingWeight < secondValue.sortingWeight);
	});
}

int analyzedDataFolder::strToWeightInt(std::string myStr)
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