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
	PROV *myprovFile = new PROV("./Resources/Config/Prov/" + locaFold->locaName + ".prov");
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
		myTRC = new TRCFile(locaFold->trcFile);
	else
		myTRC = new TRCFile(files[idFile].filePath());

	TRCFunctions::readTRCDataAllChanels(myTRC);
	ELANFile *myELAN = ELANFunctions::micromedToElan(myTRC);
	string posPath = myELAN->filePath();
	posPath.replace(posPath.end() - 4, posPath.end(), ".pos");
	ELANFunctions::writePosFile(myELAN, posPath);
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

	for (int i = 0; i < currentPatient->localizerFolder.size(); i++)
	{
		if (optionUser->anaOption[i].localizer)
		{
			emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + currentPatient->localizerFolder[i].pathToFolder + " ==="));			
			myContainer = extractEEGData(currentPatient->localizerFolder[i], i, optionUser->freqOption.frequencyBands.size());

			if (myContainer != nullptr)
			{
				emit sendLogInfo("Number of Bip : " + QString::number(myContainer->bipoles.size()));
				//==
				stringstream().swap(TimeDisp);
				GetLocalTime(&LocalTime);
				TimeDisp << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << "\n";
				emit sendLogInfo(QString::fromStdString(TimeDisp.str()));
				//==
				loca->LocaSauron(myContainer, i, &currentPatient->localizerFolder[i]);
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
		emit sendLogInfo(QString::fromStdString("=== PROCESSING : " + currentFiles[i].rootFolder + " ==="));
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

eegContainer *Worker::extractEEGData(locaFolder currentLoca, int idFile, int nbFreqBand)
{
	if (currentLoca.trcFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.trcFile));
		TRCFile *myTRC = new TRCFile(currentLoca.trcFile);
		extractOriginalData(myTRC, optionUser->anaOption[idFile]);

		eegContainer *myContainer = new eegContainer(myTRC, 64, nbFreqBand);
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
	else if (currentLoca.eegFile != "" && currentLoca.eegEntFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.eegFile));
		ELANFile *myElan = new ELANFile(currentLoca.eegFile);
		ELANFunctions::readHeader(myElan, myElan->filePath());
		if(currentLoca.posFile != "")
			ELANFunctions::readPosFile(myElan, currentLoca.posFile);
		extractOriginalData(myElan, optionUser->anaOption[idFile]);

		eegContainer *myContainer = new eegContainer(myElan, 64, nbFreqBand);
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
	else
	{
		return nullptr;
	}
}

eegContainer *Worker::extractEEGData(locaFolder currentLoca)
{
	if (currentLoca.trcFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.trcFile));
		TRCFile *myTRC = new TRCFile(currentLoca.trcFile);
		TRCFunctions::readTRCDataAllChanels(myTRC);

		eegContainer *myContainer = new eegContainer(myTRC, 64, 0);
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
	else if (currentLoca.eegFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentLoca.eegFile));
		ELANFile *myElan = new ELANFile(currentLoca.eegFile);
		ELANFunctions::readHeader(myElan, myElan->filePath());
		if (currentLoca.posFile != "")
			ELANFunctions::readPosFile(myElan, currentLoca.posFile);
		ELANFunctions::readDataAllChannels(myElan, myElan->filePath());

		eegContainer *myContainer = new eegContainer(myElan, 64, 0);

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
	else
	{
		return nullptr;
	}
}

eegContainer *Worker::extractEEGData(singleFile currentFile, int idFile, int nbFreqBand)
{
	if (currentFile.trcFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.trcFile));
		TRCFile *myTRC = new TRCFile(currentFile.trcFile);
		extractOriginalData(myTRC, optionUser->anaOption[idFile]);

		eegContainer *myContainer = new eegContainer(myTRC, 64, nbFreqBand);
		emit sendContainerPointer(myContainer);
		while (bipCreated == -1) //While bipole not created 
		{
			QCoreApplication::processEvents(QEventLoop:: WaitForMoreEvents);	//check if list of elec validated
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
	else if (currentFile.eegFile != "")// && currentFile.eegEntFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.eegFile));
		ELANFile *myElan = new ELANFile(currentFile.eegFile);
		ELANFunctions::readHeader(myElan, myElan->filePath());
		if(currentFile.posFile != "")
			ELANFunctions::readPosFile(myElan, currentFile.posFile);
		extractOriginalData(myElan, optionUser->anaOption[idFile]);

		eegContainer *myContainer = new eegContainer(myElan, 64, nbFreqBand);
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
	else
	{
		return nullptr;
	}
}

eegContainer *Worker::extractEEGData(singleFile currentFile)
{
	if (currentFile.trcFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.trcFile));
		TRCFile *myTRC = new TRCFile(currentFile.trcFile);
		TRCFunctions::readTRCDataAllChanels(myTRC);

		eegContainer *myContainer = new eegContainer(myTRC, 64, 0);
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
	else if (currentFile.eegFile != "")// && currentFile.eegEntFile != "")
	{
		emit sendLogInfo(QString::fromStdString("  => Reading : " + currentFile.eegFile));
		ELANFile *myElan = new ELANFile(currentFile.eegFile);
		ELANFunctions::readHeader(myElan, myElan->filePath());
		if (currentFile.posFile != "")
			ELANFunctions::readPosFile(myElan, currentFile.posFile);
		ELANFunctions::readDataAllChannels(myElan, myElan->filePath());

		eegContainer *myContainer = new eegContainer(myElan, 64, 0);

		emit sendContainerPointer(myContainer);
		//while (bipCreated == false) //While bipole not created 
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
	else
	{
		return nullptr;
	}
}

void Worker::extractOriginalData(TRCFile *myTRC, locaAnalysisOption anaOpt)
{
	for (int i = 0; i < anaOpt.anaOpt.size(); i++)
	{
		if (anaOpt.anaOpt[i].eeg2env)
		{
			TRCFunctions::readTRCDataAllChanels(myTRC);
			return;
		}
	}
}

void Worker::extractOriginalData(ELANFile *myElan, locaAnalysisOption anaOpt)
{
	for (int i = 0; i < anaOpt.anaOpt.size(); i++)
	{
		if (anaOpt.anaOpt[i].eeg2env)
		{
			ELANFunctions::readDataAllChannels(myElan, myElan->filePath());
			return;
		}
	}
}