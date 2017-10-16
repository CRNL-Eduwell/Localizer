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

Worker::Worker(vector<singleFile> currentFiles, userOption *userOpt, int idFile = -1)
{
	files = vector<singleFile>(currentFiles);
	this->idFile = idFile;
	optionUser = userOpt;
	loca = new LOCA(optionUser);
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
	{
		analysePatientFolder(patient);
	}
	else if (files.size() > 0)
	{
		analyseSingleFiles(files);
	}

	emit finished();
}

void Worker::processERP()
{
	eegContainer *myContainer = extractEEGData(*locaFold);
	PROV *myprovFile = new PROV("./Resources/Config/Prov/" + locaFold->localizerName() + ".prov");
	if(myprovFile != nullptr)
		loca->eeg2erp(myContainer, myprovFile);
	deleteAndNullify1D(myContainer);
	deleteAndNullify1D(myprovFile);

	emit sendLogInfo("End of ERP Analysis");
	emit finished();
}

void Worker::processToELAN()
{
	TRCFile *myTRC = nullptr;
	
	if (idFile == -1)
		myTRC = new TRCFile(locaFold->filePath(TRC));
	else
		myTRC = new TRCFile(files[idFile].filePath(TRC));

	TRCFunctions::readTRCDataAllChanels(myTRC);
	ELANFile *myELAN = ELANFunctions::micromedToElan(myTRC);
	string posPath = myELAN->filePath();
	posPath.replace(posPath.end() - 4, posPath.end(), "_raw.pos");
	ELANFunctions::writePosFile(myELAN, posPath);
	string notePath = myELAN->filePath();
	notePath.replace(notePath.end() - 4, notePath.end(), ".notes.txt");
	ELANFunctions::writeNotesFile(myELAN, notePath);

	ELANFunctions::writeFile(myELAN, myELAN->filePath());

	deleteAndNullify1D(myELAN);
	deleteAndNullify1D(myTRC);
	emit sendLogInfo("End of conversion to ELAN file format");
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
				emit sendLogInfo("Number of Bip : " + QString::number(myContainer->bipoles.size()));
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
		emit sendLogInfo("Number of Bip : " + QString::number(myContainer->bipoles.size()));
		//==
		stringstream().swap(TimeDisp);
		GetLocalTime(&LocalTime);
		TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
		emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
		//==
		for (int j = 0; j < optionUser->freqOption.frequencyBands.size(); j++)
		{
			if(optionUser->anaOption[i].anaOpt[j].eeg2env)
				myContainer->ToHilbert(myContainer->elanFrequencyBand[j], optionUser->freqOption.frequencyBands[j].freqBandValue);

			//HOTFIX TRES SALE
			//LES POS DOIVENT ETRE SORTI DANS UN CAS DANALYSE SIMPLE AUSSI (EX BTV)
			if (myContainer->triggEeg != nullptr)
			{
				ofstream posFile(myContainer->originalFilePath + ".pos", ios::out);
				ofstream posFileX(myContainer->originalFilePath + "_ds" + to_string(myContainer->sampInfo.downsampFactor) + ".pos", ios::out);

				for (int k = 0; k < myContainer->triggEeg->triggers.size(); k++)
				{
					posFile << myContainer->triggEeg->triggers[k].trigger.sample << setw(10)
						<< myContainer->triggEeg->triggers[k].trigger.code << setw(10) << "0" << endl;
					posFileX << myContainer->triggEegDownsampled->triggers[k].trigger.sample << setw(10)
						<< myContainer->triggEegDownsampled->triggers[k].trigger.code << setw(10) << "0" << endl;
				}

				posFile.close();
				posFileX.close();
			}
		}
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

//===================       en dessous ok

eegContainer *Worker::extractEEGData(locaFolder currentLoca, int idFile, int nbFreqBand)
{
	eegContainer *myContainer = nullptr;

	switch (currentLoca.fileExtention())
	{
	case TRC:
		myContainer = createFromTRC(currentLoca, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case EEG_ELAN:
		myContainer = createFromELAN(currentLoca, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case EDF:
		myContainer = createFromEDF(currentLoca, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case NO_EXT:
		return nullptr;
		break;
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

	myContainer->deleteElectrodes(elecToDeleteMem);
	myContainer->getElectrodes();
	myContainer->bipolarizeData();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::extractEEGData(locaFolder currentLoca)
{
	eegContainer *myContainer = nullptr;

	switch (currentLoca.fileExtention())
	{
	case TRC:
		myContainer = createFromTRC(currentLoca, true);
		break;
	case EEG_ELAN:
		myContainer = createFromELAN(currentLoca, true);
		break;
	case EDF:
		myContainer = createFromEDF(currentLoca, true);
		break;
	case NO_EXT:
		return nullptr;
		break;
	}

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	elecToDeleteMem = vector<int>(myContainer->idElecToDelete);

	myContainer->deleteElectrodes(elecToDeleteMem);
	myContainer->getElectrodes();
	myContainer->bipolarizeData();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::createFromTRC(locaFolder currentLoca, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(TRC)));
	TRCFile *myTRC = new TRCFile(currentLoca.filePath(TRC));
	if (extractOrigData)
		TRCFunctions::readTRCDataAllChanels(myTRC);

	return new eegContainer(myTRC, 64, nbFreqBand);
}

eegContainer *Worker::createFromELAN(locaFolder currentLoca, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EEG_ELAN)));
	ELANFile *myElan = new ELANFile(currentLoca.filePath(EEG_ELAN));
	ELANFunctions::readHeader(myElan, myElan->filePath());
	if (currentLoca.filePath(POS_ELAN) != "")
		ELANFunctions::readPosFile(myElan, currentLoca.filePath(POS_ELAN));
	if (extractOrigData)
		ELANFunctions::readDataAllChannels(myElan, myElan->filePath());

	return new eegContainer(myElan, 64, nbFreqBand);
}

eegContainer *Worker::createFromEDF(locaFolder currentLoca, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.filePath(EDF)));
	EDFFile *myEdf = new EDFFile(currentLoca.filePath(EDF));
	myEdf->readEvents();
	if (extractOrigData)
		myEdf->readEEGData();

	return new eegContainer(myEdf, 64, nbFreqBand);
}

eegContainer *Worker::extractEEGData(singleFile currentFile, int idFile, int nbFreqBand)
{
	eegContainer *myContainer = nullptr;

	switch (currentFile.fileExtention())
	{
	case TRC:
		myContainer = createFromTRC(currentFile, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case EEG_ELAN:
		myContainer = createFromELAN(currentFile, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case EDF:
		myContainer = createFromEDF(currentFile, extractOriginalData(optionUser->anaOption[idFile]), nbFreqBand);
		break;
	case NO_EXT:
		return nullptr;
		break;
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

	myContainer->deleteElectrodes(elecToDeleteMem);
	myContainer->getElectrodes();
	myContainer->bipolarizeData();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::extractEEGData(singleFile currentFile)
{
	eegContainer *myContainer = nullptr;

	switch (currentFile.fileExtention())
	{
	case TRC:
		myContainer = createFromTRC(currentFile, true);
		break;
	case EEG_ELAN:
		myContainer = createFromELAN(currentFile, true);
		break;
	case EDF:
		myContainer = createFromEDF(currentFile, true);
		break;
	case NO_EXT:
		return nullptr;
		break;
	}

	emit sendContainerPointer(myContainer);
	while (bipCreated == -1) //While bipole not created 
	{
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);	//check if list of elec validated
	}
	if (bipCreated == 0)
		return nullptr;

	elecToDeleteMem = vector<int>(myContainer->idElecToDelete);

	myContainer->deleteElectrodes(elecToDeleteMem);
	myContainer->getElectrodes();
	myContainer->bipolarizeData();

	emit sendLogInfo(QString::fromStdString("Bipole created !"));
	return myContainer;
}

eegContainer *Worker::createFromTRC(singleFile currentFile, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(TRC)));
	TRCFile *myTRC = new TRCFile(currentFile.filePath(TRC));
	if(extractOrigData)
		TRCFunctions::readTRCDataAllChanels(myTRC);

	return new eegContainer(myTRC, 64, nbFreqBand);
}

eegContainer *Worker::createFromELAN(singleFile currentFile, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EEG_ELAN)));
	ELANFile *myElan = new ELANFile(currentFile.filePath(EEG_ELAN));
	ELANFunctions::readHeader(myElan, myElan->filePath());
	if (currentFile.filePath(POS_ELAN) != "")
		ELANFunctions::readPosFile(myElan, currentFile.filePath(POS_ELAN));
	if (extractOrigData)
		ELANFunctions::readDataAllChannels(myElan, myElan->filePath());

	return new eegContainer(myElan, 64, nbFreqBand);
}

eegContainer *Worker::createFromEDF(singleFile currentFile, bool extractOrigData, int nbFreqBand)
{
	emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.filePath(EDF)));
	EDFFile *myEdf = new EDFFile(currentFile.filePath(EDF));
	myEdf->readEvents();
	if (extractOrigData)
		myEdf->readEEGData();

	return new eegContainer(myEdf, 64, nbFreqBand);
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