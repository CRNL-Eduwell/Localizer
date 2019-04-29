#include "Worker.h"

Worker::Worker(patientFolder *currentPatient, userOption *userOpt)
{
	patient = currentPatient;
	optionUser = userOpt;
	loca = new LOCA(optionUser);
}

Worker::Worker(locaFolder *locaFold, userOption *userOpt)
{
	this->locaFold = locaFold;
	optionUser = userOpt;
	loca = new LOCA(optionUser);
}

Worker::Worker(vector<singleFile> currentFiles, userOption *userOpt, int idFile)
{
	files = vector<singleFile>(currentFiles);
	this->idFile = idFile;
	optionUser = userOpt;
	loca = new LOCA(optionUser);
}

Worker::Worker(string myFirstTRC, string mySecondTRC, string myOutputTRC)
{
	if (trcFiles.size() > 0)
		trcFiles.clear();

	trcFiles.push_back(myFirstTRC);
	trcFiles.push_back(mySecondTRC);
	trcFiles.push_back(myOutputTRC);
}

Worker::~Worker()
{
	deleteAndNullify1D(loca);
}

LOCA* Worker::getLoca()
{
	return loca;
}

void Worker::processAnalysis()
{
	if (patient != nullptr)
		analysePatientFolder(patient);
	else if (files.size() > 0)
		analyseSingleFiles(files);

	emit finished();
}

void Worker::processERP()
{
	eegContainer *myContainer = extractEEGData(*locaFold);
	PROV *myprovFile = new PROV("./Resources/Config/Prov/" + locaFold->localizerName() + ".prov");
	if (myprovFile != nullptr)
		loca->eeg2erp(myContainer, myprovFile);
	deleteAndNullify1D(myContainer);
	deleteAndNullify1D(myprovFile);

	emit sendLogInfo("End of ERP Analysis");
	emit finished();
}

void Worker::processToELAN()
{
	emit sendLogInfo("Starting conversion to ELAN file format");

	EEGFormat::MicromedFile *myTRC = nullptr;

	if (idFile == -1)
		myTRC = new EEGFormat::MicromedFile(locaFold->filePath(TRC));
	else
		myTRC = new EEGFormat::MicromedFile(files[idFile].filePath(TRC));
	myTRC->LoadDataAllChannels();

	EEGFormat::ElanFile *myELAN = new EEGFormat::ElanFile(*myTRC);

	std::string headerPath = myTRC->FilePath();
	headerPath.replace(headerPath.end() - 4, headerPath.end(), ".eeg.ent");
	std::string dataPath = myTRC->FilePath();
	dataPath.replace(dataPath.end() - 4, dataPath.end(), ".eeg");
	std::string posPath = myTRC->FilePath();
	posPath.replace(posPath.end() - 4, posPath.end(), "_raw.pos");
	string notePath = myTRC->FilePath();
	notePath.replace(notePath.end() - 4, notePath.end(), ".notes.txt");

	myELAN->SaveAs(headerPath, dataPath, notePath, posPath);

	deleteAndNullify1D(myELAN);
	deleteAndNullify1D(myTRC);
	emit sendLogInfo("End of conversion to ELAN file format");
	emit finished();
}

void Worker::processConcatenation()
{
	emit sendLogInfo("Starting concatenation");
	//TRCFile *myTRC = new TRCFile(trcFiles[0]);
	//TRCFile *myTRC2 = new TRCFile(trcFiles[1]);
	//TRCFunctions::concatenateTRCFile(myTRC, myTRC2, trcFiles[2]);
	//TRCFunctions::stapleTRCFile(myTRC, myTRC2, trcFiles[2]);
	emit sendLogInfo("End of concatenation");
	emit finished();
}

void Worker::analysePatientFolder(patientFolder *currentPatient)
{
	stringstream TimeDisp;
	SYSTEMTIME LocalTime;
	eegContainer *myContainer = nullptr;

	for (int i = 0; i < currentPatient->localizerFolder().size(); i++)
	{
		if (optionUser->anaOption[i].localizer)
		{
			emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + currentPatient->localizerFolder()[i].rootLocaFolder() + " ==="));
			myContainer = extractEEGData(currentPatient->localizerFolder()[i], i, optionUser->freqOption.frequencyBands.size());

			if (myContainer != nullptr)
			{
				emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
				//==
				stringstream().swap(TimeDisp);
				GetLocalTime(&LocalTime);
				TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
				emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
				//==
				loca->LocaSauron(myContainer, i, &currentPatient->localizerFolder()[i]);
				//==
				stringstream().swap(TimeDisp);
				GetLocalTime(&LocalTime);
				TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
				emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
				//==
				sendLogInfo("End of Loca number " + QString::number(i) + "\n");
				deleteAndNullify1D(myContainer);
			}
		}
	}

	//Generate comportemental perf report


}

void Worker::analyseSingleFiles(vector<singleFile> currentFiles)
{
	stringstream TimeDisp;
	SYSTEMTIME LocalTime;
	eegContainer *myContainer = nullptr;

	for (int i = 0; i < currentFiles.size(); i++)
	{
		emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + currentFiles[i].rootFolder() + " ==="));
		myContainer = extractEEGData(currentFiles[i], i, optionUser->freqOption.frequencyBands.size());

		if (myContainer != nullptr)
		{
			emit sendLogInfo("Number of Bip : " + QString::number(myContainer->BipoleCount()));
			//==
			stringstream().swap(TimeDisp);
			GetLocalTime(&LocalTime);
			TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			loca->LocaFrequency(myContainer, i);
			//==
			stringstream().swap(TimeDisp);
			GetLocalTime(&LocalTime);
			TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
			emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
			//==
			sendLogInfo("End of Loca number " + QString::number(i) + "\n");
			deleteAndNullify1D(myContainer);
		}
	}
}

eegContainer *Worker::extractEEGData(locaFolder currentLoca, int idFile, int nbFreqBand)
{
	eegContainer *myContainer = nullptr;

	bool shouldExtractData = extractOriginalData(optionUser->anaOption[idFile]);
	switch (currentLoca.fileExtention())
	{
		case TRC:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(TRC)));
			EEGFormat::MicromedFile *myTRC = new EEGFormat::MicromedFile(currentLoca.filePath(TRC));
			if (shouldExtractData)
				myTRC->LoadDataAllChannels();
			myContainer = new eegContainer(myTRC, 64, nbFreqBand);
			break;
		}
		case EEG_ELAN:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EEG_ELAN)));
			EEGFormat::ElanFile *myElan = new EEGFormat::ElanFile(currentLoca.filePath(ENT_ELAN), currentLoca.filePath(EEG_ELAN), currentLoca.filePath(POS_ELAN));
			if (shouldExtractData)
				myElan->Load();
			myContainer = new eegContainer(myElan, 64, nbFreqBand);
			break;
		}
		case EDF:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EDF)));
			EEGFormat::EdfFile *myEdf = new EEGFormat::EdfFile(currentLoca.filePath(EDF));
			myEdf->LoadEvents();
			if (shouldExtractData)
				myEdf->Load();
			myContainer = new eegContainer(myEdf, 64, nbFreqBand);
			break;
		}
		case NO_EXT:
		{	
			return nullptr;
			break;
		}
	}

	if (idFile == 0)
	{
		emit sendContainerPointer(myContainer);
		while (bipCreated == -1) //While bipole not created 
		{
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
		}
		if (bipCreated == 0)
			return nullptr;

		elecToDeleteMem = vector<int>(myContainer->idElecToDelete);
	}

	myContainer->DeleteElectrodes(elecToDeleteMem);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::extractEEGData(locaFolder currentLoca)
{
	eegContainer *myContainer = nullptr;

	switch (currentLoca.fileExtention())
	{
		case TRC:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(TRC)));
			EEGFormat::MicromedFile *myTRC = new EEGFormat::MicromedFile(currentLoca.filePath(TRC));
			myTRC->LoadDataAllChannels();
			myContainer = new eegContainer(myTRC, 64, 0);
			break;
		}
		case EEG_ELAN:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EEG_ELAN)));
			EEGFormat::ElanFile *myElan = new EEGFormat::ElanFile(currentLoca.filePath(ENT_ELAN), currentLoca.filePath(EEG_ELAN), currentLoca.filePath(POS_ELAN));
			myElan->Load();
			myContainer = new eegContainer(myElan, 64, 0);
			break;
		}
		case EDF:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EDF)));
			EEGFormat::EdfFile *myEdf = new EEGFormat::EdfFile(currentLoca.filePath(EDF));
			myEdf->LoadEvents();
			myEdf->Load();
			myContainer = new eegContainer(myEdf, 64, 0);
			break;
		}
		case NO_EXT:
		{
			return nullptr;
			break;
		}
	}

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	elecToDeleteMem = vector<int>(myContainer->idElecToDelete);

	myContainer->DeleteElectrodes(elecToDeleteMem);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::extractEEGData(singleFile currentFile, int idFile, int nbFreqBand)
{
	eegContainer *myContainer = nullptr;

	bool shouldExtractData = extractOriginalData(optionUser->anaOption[idFile]);
	switch (currentFile.fileExtention())
	{
		case TRC:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(TRC)));
			EEGFormat::MicromedFile *myTRC = new EEGFormat::MicromedFile(currentFile.filePath(TRC));
			if (shouldExtractData)
				myTRC->LoadDataAllChannels();
			myContainer = new eegContainer(myTRC, 64, nbFreqBand);
			break;
		}
		case EEG_ELAN:
		{	
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EEG_ELAN)));
			EEGFormat::ElanFile *myElan = new EEGFormat::ElanFile(currentFile.filePath(ENT_ELAN), currentFile.filePath(EEG_ELAN), currentFile.filePath(POS_ELAN));
			if (shouldExtractData)
				myElan->Load();
			myContainer = new eegContainer(myElan, 64, nbFreqBand);
			break;
		}
		case EDF:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EDF)));
			EEGFormat::EdfFile *myEdf = new EEGFormat::EdfFile(currentFile.filePath(EDF));
			myEdf->LoadEvents();
			if (shouldExtractData)
				myEdf->Load();
			myContainer = new eegContainer(myEdf, 64, nbFreqBand);
			break;
		}
		case NO_EXT:
		{	
			return nullptr;
			break;
		}
	}

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	bipCreated = -1; //Since we loop one or multiple file we need to recheck each time the good/bad elec

	elecToDeleteMem = vector<int>(myContainer->idElecToDelete);

	myContainer->DeleteElectrodes(elecToDeleteMem);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::extractEEGData(singleFile currentFile)
{
	eegContainer *myContainer = nullptr;

	switch (currentFile.fileExtention())
	{
		case TRC:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(TRC)));
			EEGFormat::MicromedFile *myTRC = new EEGFormat::MicromedFile(currentFile.filePath(TRC));
			myTRC->LoadDataAllChannels();
			myContainer = new eegContainer(myTRC, 64, 0);
			break;
		}
		case EEG_ELAN:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EEG_ELAN)));
			EEGFormat::ElanFile *myElan = new EEGFormat::ElanFile(currentFile.filePath(ENT_ELAN), currentFile.filePath(EEG_ELAN), currentFile.filePath(POS_ELAN));
			myElan->Load();
			myContainer = new eegContainer(myElan, 64, 0);
			break;
		}
		case EDF:
		{
			emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EDF)));
			EEGFormat::EdfFile *myEdf = new EEGFormat::EdfFile(currentFile.filePath(EDF));
			myEdf->LoadEvents();
			myEdf->Load();
			myContainer = new eegContainer(myEdf, 64, 0);
			break;
		}
		case NO_EXT:
		{
			return nullptr;
			break;
		}
	}

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	elecToDeleteMem = vector<int>(myContainer->idElecToDelete);

	myContainer->DeleteElectrodes(elecToDeleteMem);
	myContainer->GetElectrodes();
	myContainer->BipolarizeElectrodes();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

bool Worker::extractOriginalData(locaAnalysisOption anaOpt)
{
	for (int i = 0; i < anaOpt.anaOpt.size(); i++)
	{
		if (anaOpt.anaOpt[i].eeg2env)
			return true;
	}
	return false;
}