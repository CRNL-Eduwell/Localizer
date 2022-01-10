#include "patientFolder.h"

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
    std::vector<std::string> splitExt = InsermLibrary::split<std::string>(rootPath, "\\/");
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
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(rootPath, "_\\/");
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

InsermLibrary::FileExt locaFolder::fileExtention()
{
	if (m_trcFile != "")
        return InsermLibrary::FileExt::TRC;
	else if (m_eegFile != "")
        return InsermLibrary::FileExt::EEG_ELAN;
	else if (m_edfFile != "")
        return InsermLibrary::FileExt::EDF;
	else
        return InsermLibrary::FileExt::NO_EXT;
}

std::string locaFolder::filePath(InsermLibrary::FileExt wantedFile)
{
	switch (wantedFile)
	{
    case InsermLibrary::FileExt::TRC:
		return m_trcFile;
    case InsermLibrary::FileExt::EEG_ELAN:
		return m_eegFile == "" ? m_freqFolder[0].FilePaths(InsermLibrary::FileExt::SM0_ELAN)[0] : m_eegFile;
    case InsermLibrary::FileExt::ENT_ELAN:
		return m_eegEntFile;
    case InsermLibrary::FileExt::POS_ELAN:
		return m_posFile;
    case InsermLibrary::FileExt::POS_DS_ELAN:
		return m_dsPosFile;
    case InsermLibrary::FileExt::EDF:
		return m_edfFile;
    case InsermLibrary::FileExt::BRAINVISION:
		return m_brainVisionFile;
    case InsermLibrary::FileExt::NO_EXT:
		return "";
	default:
		return "";
	}
	//return "";
}

void locaFolder::getLocaName(std::string rootLocaFolder)
{
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(rootLocaFolder, "\\/_");
	m_locaName = splitPath[splitPath.size() - 1];
}

void locaFolder::retrieveFiles(std::string rootLocaFolder)
{
	QDir currentDir(rootLocaFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.trc" << "*.eeg" << "*.eeg.ent" << "*.pos" << "*.edf" << "*.vhdr");

	QRegularExpression rxTRC((fullLocalizerName() + ".trc").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxEeg((fullLocalizerName() + ".eeg").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxEnt((fullLocalizerName() + ".eeg.ent").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxPos((fullLocalizerName() + "_raw.pos").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxDsPos((fullLocalizerName() + "_ds" + "(\\d+)" + ".pos").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxEdf((fullLocalizerName() + ".edf").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);
	QRegularExpression rxBrainVision((fullLocalizerName() + ".vhdr").c_str(), QRegularExpression::PatternOption::CaseInsensitiveOption);

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{	
		QRegularExpressionMatch trcMatch = rxTRC.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (trcMatch.hasMatch())
			m_trcFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eegMatch = rxEeg.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eegMatch.hasMatch())
			m_eegFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch entMatch = rxEnt.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (entMatch.hasMatch())
			m_eegEntFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch posMatch = rxPos.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (posMatch.hasMatch())
			m_posFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch dsPosMatch = rxDsPos.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (dsPosMatch.hasMatch())
			m_dsPosFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch edfMatch = rxEdf.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (edfMatch.hasMatch())
			m_edfFile = rootLocaFolder + fileFound[i].toStdString();

		QRegularExpressionMatch bvMatch = rxBrainVision.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (bvMatch.hasMatch())
			m_brainVisionFile = rootLocaFolder + fileFound[i].toStdString();
	}
}

void locaFolder::retrieveFrequencyFolders(std::string rootLocaFolder)
{
	QRegularExpression rxFreqFold((fullLocalizerName() + "_f(\\d+)f(\\d+)").c_str());

	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(rootLocaFolder.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		
		QString dirname = *entry;
		QRegularExpressionMatch dirMatch = rxFreqFold.match(dirname);
		if (dirname != QObject::tr(".") && dirname != QObject::tr("..") && dirMatch.hasMatch())
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

std::string frequencyFolder::filePath(InsermLibrary::FileExt wantedFile)
{
	switch (wantedFile)
	{
    case InsermLibrary::FileExt::SM0_ELAN:
		return m_sm0eeg;
    case InsermLibrary::FileExt::SM250_ELAN:
		return m_sm250eeg;
    case InsermLibrary::FileExt::SM500_ELAN:
		return m_sm500eeg;
    case InsermLibrary::FileExt::SM1000_ELAN:
		return m_sm1000eeg;
    case InsermLibrary::FileExt::SM2500_ELAN:
		return m_sm2500eeg;
    case InsermLibrary::FileExt::SM5000_ELAN:
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
std::vector<std::string> frequencyFolder::FilePaths(InsermLibrary::FileExt wantedFile)
{
	switch (wantedFile)
	{
    case InsermLibrary::FileExt::SM0_ELAN:
		return m_smXFiles.size() > 0 ? GetFirstFullDataSet(m_smXFiles[0]) : std::vector<std::string>();
    case InsermLibrary::FileExt::SM250_ELAN:
		return m_smXFiles.size() > 1 ? GetFirstFullDataSet(m_smXFiles[1]) : std::vector<std::string>();
    case InsermLibrary::FileExt::SM500_ELAN:
		return m_smXFiles.size() > 2 ? GetFirstFullDataSet(m_smXFiles[2]) : std::vector<std::string>();
    case InsermLibrary::FileExt::SM1000_ELAN:
		return m_smXFiles.size() > 3 ? GetFirstFullDataSet(m_smXFiles[3]) : std::vector<std::string>();
    case InsermLibrary::FileExt::SM2500_ELAN:
		return m_smXFiles.size() > 4 ? GetFirstFullDataSet(m_smXFiles[4]) : std::vector<std::string>();
    case InsermLibrary::FileExt::SM5000_ELAN:
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
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(rootFreqFolder, "\\/_");
	m_frequencyName = splitPath[splitPath.size() - 1];
}

void frequencyFolder::retrieveSMFile2(std::string rootFreqFolder)
{
	QDir currentDir(rootFreqFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent");

	QRegularExpression rxEeg0((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm0.eeg").c_str());
	QRegularExpression rxEeg250((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm250.eeg").c_str());
	QRegularExpression rxEeg500((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm500.eeg").c_str());
	QRegularExpression rxEeg1000((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm1000.eeg").c_str());
	QRegularExpression rxEeg2500((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm2500.eeg").c_str());
	QRegularExpression rxEeg5000((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm5000.eeg").c_str());

	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		QRegularExpressionMatch eeg0Match = rxEeg0.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg0Match.hasMatch())
			m_sm0eeg = rootFreqFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eeg250Match = rxEeg250.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg250Match.hasMatch())
			m_sm250eeg = rootFreqFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eeg500Match = rxEeg500.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg500Match.hasMatch())
			m_sm500eeg = rootFreqFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eeg1000Match = rxEeg1000.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg1000Match.hasMatch())
			m_sm1000eeg = rootFreqFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eeg2500Match = rxEeg2500.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg2500Match.hasMatch())
			m_sm2500eeg = rootFreqFolder + fileFound[i].toStdString();

		QRegularExpressionMatch eeg5000Match = rxEeg5000.match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
		if (eeg5000Match.hasMatch())
			m_sm5000eeg = rootFreqFolder + fileFound[i].toStdString();
	}
}

void frequencyFolder::retrieveSMFile(std::string rootFreqFolder)
{
	QDir currentDir(rootFreqFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.eeg" << "*.eeg.ent" << "*.vhdr");

	std::vector<QRegularExpression> rxEegSmX;
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm0") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm250") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm500") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm1000") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm2500") + "(.eeg|.eeg.ent|.vhdr)").c_str()));
	rxEegSmX.push_back(QRegularExpression(((fullFrequencyName() + "_ds" + "(\\d+)" + "_sm5000") + "(.eeg|.eeg.ent|.vhdr)").c_str()));

	m_smXFiles.resize(6, std::vector<std::string>());
	QStringList fileFound = currentDir.entryList();
	for (int i = 0; i < fileFound.size(); i++)
	{
		for (int j = 0; j < 6; j++)
		{
			QRegularExpressionMatch smxMatch = rxEegSmX[j].match(fileFound[i], 0, QRegularExpression::PartialPreferCompleteMatch);
			if (smxMatch.hasMatch())
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
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(rootDataFolder, "/");
    std::vector<std::string> splitFolder = InsermLibrary::split<std::string>(splitPath[splitPath.size() - 1], "_");

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
            std::vector<std::string> splitVal = InsermLibrary::split<std::string>(dirname.toStdString(), "_.");
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
