#include "TRCFunctions.h"

string MicromedLibrary::TRCFunctions::signalMeasurementUnitText(int value)
{
	std::string descriptionValue = "";

	switch (value)
	{
	case -1: descriptionValue = "nanoV";
		break;
	case 0: descriptionValue = "microV";
		break;
	case 1: descriptionValue = "milliV";
		break;
	case 2: descriptionValue = "Volt";
		break;
	case 100: descriptionValue = "%\0";
		break;
	case 101: descriptionValue = "bpm";
		break;
	case 102: descriptionValue = "Adim";
		break;
	}

	return descriptionValue;
}

//===[ Data Modif ]===

void MicromedLibrary::TRCFunctions::deleteOneOrSeveralElectrodesAndData(TRCFile *myTRCFile, vector<int> indexToDelete)
{
	if (indexToDelete.size() > 0)
	{
		if (myTRCFile->eegAllChanels.size() == 0)
		{
			for (int i = (int)indexToDelete.size() - 1; i >= 0; i--)
			{
				myTRCFile->electrodesList.erase(myTRCFile->electrodesList.begin() + indexToDelete[i]);
			}
		}
		else
		{
			for (int i = (int)indexToDelete.size() - 1; i >= 0; i--)
			{
				myTRCFile->electrodesList.erase(myTRCFile->electrodesList.begin() + indexToDelete[i]);
				myTRCFile->eegAllChanels.erase(myTRCFile->eegAllChanels.begin() + indexToDelete[i]);
			}
		}
	}
}

void MicromedLibrary::TRCFunctions::convertAnalogDataToDigital(TRCFile *myTRCFile)
{
	for (int i = 0; i < myTRCFile->electrodes().size(); i++)
	{
		for (int j = 0; j < myTRCFile->nbSample(); j++)
		{
			long mul = (myTRCFile->electrodes()[i].logicMaximum - myTRCFile->electrodes()[i].logicMinimum) + 1;
			long div = (myTRCFile->electrodes()[i].physicMaximum - myTRCFile->electrodes()[i].physicMinimum);
			myTRCFile->eegAllChanels[i][j] = ((myTRCFile->eegAllChanels[i][j] * mul) / div) + myTRCFile->electrodes()[i].logicGround;
		}
	}
}

//===[ Data Read ]===

/******************************************************************************************************************************/
/*					Extract the Data of ONE eeg chanel from a TRC File.								  					  	  */
/*				EEG Data(2bytes) : 00 - 11 - 22 - 33 - 00 - 11 - 22 - 33 - 00 - 11 - 22 - 33								  */
/*									^                   ^           ^														  */
/*									|			        |			|														  */
/*								  pos 1               pos 2       pos 3														  */
/*	== > pos 1 = position of first byte + (number of bytes * index chanel to analyse)										  */
/*	== > You browse every group of sample via a for loop(Data Size / Number of Chanels)										  */
/*	== > increment the position for retrieving a full chanel : pos 2 = pos 1 + (number of bytes * number of chanels stored)	  */
/******************************************************************************************************************************/
void MicromedLibrary::TRCFunctions::readTRCDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, int posInVector)
{
	char *binaryDataTRC = new char[myTRCFile->dataSize / myTRCFile->header4.numberStoredChannels];
	readBinaryDataOneChanel(myTRCFile, indexChanelToExtract, &binaryDataTRC[0]);
	binaryToDigitalDataOneChanel(myTRCFile, posInVector, &binaryDataTRC[0]);
	delete[] binaryDataTRC;
}

/*****************************************************************************/
/*						Extract Multiple Chanels							 */
/*	 For speed purpose since accessing the disk is the slowest part we get	 */
/*	all the binary data in one pass and we convert only the chanels we want  */
/*****************************************************************************/
void MicromedLibrary::TRCFunctions::readTRCDataMultipleChanel(TRCFile *myTRCFile)
{
	char *binaryDataTRC = new char[myTRCFile->dataSize];
	readBinaryDataAllChanels(myTRCFile, &binaryDataTRC[0]);
	binaryToDigitalDataMultipleChanels(myTRCFile, &binaryDataTRC[0]);
	delete[] binaryDataTRC;
}

/*****************************************************************************/
/*							Extract All Chanels								 */
/*	 Since we don't care about the multiplexing of data we juste retrieve	 */
/*	  the binary data in one read and then convert it to digitals value		 */
/*****************************************************************************/
void MicromedLibrary::TRCFunctions::readTRCDataAllChanels(TRCFile *myTRCFile)
{
	char *binaryDataTRC = new char[myTRCFile->dataSize];
	readBinaryDataAllChanels(myTRCFile, &binaryDataTRC[0]);
	binaryToDigitalDataAllChanels(myTRCFile, &binaryDataTRC[0]);
	delete[] binaryDataTRC;
}

//===[ Data Write ]===

void MicromedLibrary::TRCFunctions::writeTRCFileFromOriginal(ofstream &outputFileStream, TRCFile *myTRCFile)
{
	std::ifstream infile;
	infile.open(myTRCFile->myFilePath.c_str(), std::ios::binary | std::ios::in);

	copyBinaryHeader(outputFileStream, infile, myTRCFile->header4.adressFirstData);
	readAndWriteBinaryDataMultiChanels(outputFileStream, infile, myTRCFile);
	modifyBinaryHeaderValue(outputFileStream, infile, myTRCFile);
	//===========
	outputFileStream.close();
	infile.close();
}

void MicromedLibrary::TRCFunctions::writeTRCFileFromData(std::string outputPath, TRCFile *myTRCFile)
{
	std::ofstream outputFileStream;
	outputFileStream.open(outputPath.c_str(), std::ios::binary);

	writeHeader(outputFileStream, &myTRCFile->header4);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorCode, 176);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorElectrode, 192);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorNote, 208);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorFlag, 224);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorReduction, 240);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorBeginImpedance, 256);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorEndImpedance, 272);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorMontages, 288);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorCompression, 304);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorAverage, 320);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorHistory, 336);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorDVideo, 352);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorEventA, 368);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorEventB, 384);
	writeDescriptor(outputFileStream, &myTRCFile->header4.descriptorTrigger, 400);
	outputFileStream.seekp(416, std::ios::beg);
	outputFileStream << myTRCFile->header4.reservedFurtherDEV2;

	writeOrderStorageElectrodes(outputFileStream, myTRCFile->orderStorageElec, myTRCFile->header4.descriptorCode.startOffset);
	writeElectrodes(outputFileStream, myTRCFile);
	writeNotes(outputFileStream, myTRCFile->notesList, myTRCFile->header4.descriptorNote.startOffset);
	writeFlags(outputFileStream, myTRCFile->flagsList, myTRCFile->header4.descriptorFlag.startOffset);
	writeReductionInfo(outputFileStream, myTRCFile->reductionLists, myTRCFile->header4.descriptorReduction.startOffset);
	writeBeginImpedance(outputFileStream, myTRCFile->startingImpedance, myTRCFile->header4.descriptorBeginImpedance.startOffset);
	writeEndImpedance(outputFileStream, myTRCFile->endingImpedance, myTRCFile->header4.descriptorEndImpedance.startOffset);
	writeMontages(outputFileStream, myTRCFile->montagesList, myTRCFile->header4.descriptorMontages.startOffset);
	writeCompression(outputFileStream, &myTRCFile->compression, myTRCFile->header4.descriptorCompression.startOffset);
	writeAverage(outputFileStream, &myTRCFile->averageParameters, myTRCFile->header4.descriptorAverage.startOffset);
	writeHistoryInfo(outputFileStream, myTRCFile->historySample, myTRCFile->montagesHistoryList, myTRCFile->header4.descriptorHistory.startOffset);
	writeDVideo(outputFileStream, myTRCFile->digitalVideos, myTRCFile->header4.descriptorDVideo.startOffset);
	writeEventA(outputFileStream, myTRCFile->eventA, myTRCFile->header4.descriptorEventA.startOffset);
	writeEventB(outputFileStream, myTRCFile->eventB, myTRCFile->header4.descriptorEventB.startOffset);
	writeDigitalTriggers(outputFileStream, myTRCFile->triggersList, myTRCFile->header4.descriptorTrigger.startOffset);
	writeDataChanels(outputFileStream, myTRCFile);

	outputFileStream.close();
}

void MicromedLibrary::TRCFunctions::concatenateTRCFile(TRCFile *myTRCFile, TRCFile *myOtherTRCFile, string outputFilePath)
{
	if (myTRCFile->header4.samplingRate != myOtherTRCFile->header4.samplingRate)
	{
		cerr << "Error : Not the same Sampling Frequency" << endl;
		return;
	}

	if (myTRCFile->header4.numberStoredChannels != myOtherTRCFile->header4.numberStoredChannels)
	{
		cerr << "Error : Not the same number of channels" << endl;
		return;
	}

	if (myTRCFile->reductionLists.size() > 0 || myOtherTRCFile->reductionLists.size() > 0)
	{
		cerr << "Error : One of the file is a reduced one" << endl;
		return;
	}

	if (myTRCFile->eegAllChanels.size() == 0)
		TRCFunctions::readTRCDataAllChanels(myTRCFile);

	if (myOtherTRCFile->eegAllChanels.size() == 0)
		TRCFunctions::readTRCDataAllChanels(myOtherTRCFile);

	TRCFile *concatenateFile = new TRCFile(*myTRCFile);
	concatenateFile->myFilePath = outputFilePath;

	concatNotes(myOtherTRCFile, concatenateFile);
	concatFlags(myOtherTRCFile, concatenateFile);
	concatEventA(myOtherTRCFile, concatenateFile);
	concatEventB(myOtherTRCFile, concatenateFile);
	concatTriggers(myOtherTRCFile, concatenateFile);
	concatDataChanels(myOtherTRCFile, concatenateFile);

	TRCFunctions::writeTRCFileFromData(outputFilePath, concatenateFile);
	deleteAndNullify1D(concatenateFile);
}

void MicromedLibrary::TRCFunctions::stapleTRCFile(TRCFile *myTRCFile, TRCFile *myOtherTRCFile, string outputFilePath)
{
	if (myTRCFile->header4.samplingRate != myOtherTRCFile->header4.samplingRate)
	{
		cerr << "Error : Not the same Sampling Frequency" << endl;
		return;
	}

	if (myTRCFile->reductionLists.size() > 0 || myOtherTRCFile->reductionLists.size() > 0)
	{
		cerr << "Error : One of the file is a reduced one" << endl;
		return;
	}

	if (myTRCFile->eegAllChanels.size() == 0)
		TRCFunctions::readTRCDataAllChanels(myTRCFile);

	if (myOtherTRCFile->eegAllChanels.size() == 0)
		TRCFunctions::readTRCDataAllChanels(myOtherTRCFile);

	TRCFile *concatenateFile = new TRCFile(*myTRCFile);
	concatenateFile->myFilePath = outputFilePath;

	for (int i = 0; i < myOtherTRCFile->electrodesList.size(); i++)
	{
		concatenateFile->orderStorageElec.push_back(myOtherTRCFile->orderStorageElec[i]);
		concatenateFile->electrodesList.push_back(myOtherTRCFile->electrodesList[i]);
		concatenateFile->eegAllChanels.push_back(move(myOtherTRCFile->eegAllChanels[i]));
	}
	concatenateFile->header4.numberStoredChannels = concatenateFile->electrodesList.size();
	concatenateFile->header4.multiplexer = concatenateFile->header4.numberBytes * concatenateFile->header4.numberStoredChannels;
	concatenateFile->setDataSize(concatenateFile->eegAllChanels.size() * concatenateFile->eegAllChanels[0].size() * concatenateFile->header4.numberBytes);
	for (int i = 0; i < concatenateFile->electrodesList.size(); i++)
		concatenateFile->electrodesList[i].position = i;

	TRCFunctions::writeTRCFileFromData(outputFilePath, concatenateFile);
	deleteAndNullify1D(concatenateFile);
}

//===[ Debug / Tools ]===
void MicromedLibrary::TRCFunctions::exportTRCDataCSV(string csvFilePath, TRCFile *myTRCFile)
{
	ofstream fichierCsv(csvFilePath, ios::out);

	for (int j = 0; j < myTRCFile->eegAllChanels.size(); j++)
	{
		fichierCsv << myTRCFile->electrodes()[j].positiveInputLabel.c_str() << ";";
	}
	fichierCsv << endl;

	for (int i = 0; i < myTRCFile->header().samplingRate * 10; i++)
	{
		for (int j = 0; j < myTRCFile->eegAllChanels.size(); j++)
		{
			fichierCsv << myTRCFile->eegAllChanels[j][i] << ";";
		}
		fichierCsv << endl;
	}
	fichierCsv.close();
}

void MicromedLibrary::TRCFunctions::exportTRCDataCSVOneChanel(string csvFilePath, TRCFile *myTRCFile, int index)
{
	ofstream fichierCsv(csvFilePath, ios::out);
	fichierCsv << myTRCFile->electrodes()[index].positiveInputLabel.c_str() << ";" << endl;
	for (int i = 0; i < myTRCFile->header().samplingRate * 10; i++)
	{
		fichierCsv << myTRCFile->eegOneChanel[i] << ";" << endl;
	}
	fichierCsv.close();
}

//=========================== Private ===========================
//===[ Data Read ]===
void MicromedLibrary::TRCFunctions::readBinaryDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, char *binaryEEGData)
{
	int numberSample = myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels *  myTRCFile->header4.numberBytes);
	int pos = myTRCFile->header4.adressFirstData + (myTRCFile->header4.numberBytes * indexChanelToExtract);
	int compteur = 0;

	try
	{
		ifstream fileStream(myTRCFile->myFilePath, ios::binary);

		for (int i = 0; i < numberSample; i++)  //taille des données/nombre canal
		{
			fileStream.seekg(pos, std::ios::beg);
			fileStream.read(&binaryEEGData[compteur], myTRCFile->header4.numberBytes);
			pos = pos + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes);
			compteur = compteur + myTRCFile->header4.numberBytes;
		}

		fileStream.close();

	}
	catch (system_error& e)
	{
		cerr << "Error Reading Binary Data of One Chanel" << endl;
		cerr << e.code().message().c_str() << endl;
	}

}

void MicromedLibrary::TRCFunctions::binaryToDigitalDataOneChanel(TRCFile *myTRCFile, int posInVector, char *binaryEEGData)
{
	float numerator, denominator, multiplicator;
	int numberSample = myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels *  myTRCFile->header4.numberBytes);
	myTRCFile->eegOneChanel.resize(numberSample);

	denominator = (float)(myTRCFile->electrodesList[posInVector].logicMaximum - myTRCFile->electrodesList[posInVector].logicMinimum) + 1;
	multiplicator = (float)myTRCFile->electrodesList[posInVector].physicMaximum - myTRCFile->electrodesList[posInVector].physicMinimum;

	if (myTRCFile->header4.numberBytes == 2)
	{
		unsigned char LSB, MSB;
		for (int i = 0; i < myTRCFile->header4.numberBytes * numberSample; i += myTRCFile->header4.numberBytes)  //taille des données/nombre canal
		{
			MSB = (unsigned char)binaryEEGData[i + 1]; //<< 8;
			LSB = (unsigned char)binaryEEGData[i];

			myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = float(LSB | MSB << 8);
			numerator = myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] - myTRCFile->electrodesList[posInVector].logicGround;
			myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = (numerator / denominator) * multiplicator;
		}
	}
	else if (myTRCFile->header4.numberBytes == 4)
	{
		unsigned char LSB, lsb, msb, MSB;
		for (int i = 0; i < myTRCFile->header4.numberBytes * numberSample; i += myTRCFile->header4.numberBytes)  //taille des données/nombre canal
		{
			MSB = (unsigned char)binaryEEGData[i + 3];
			msb = (unsigned char)binaryEEGData[i + 2];
			lsb = (unsigned char)binaryEEGData[i + 1];
			LSB = (unsigned char)binaryEEGData[i];

			myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = float(LSB | lsb << 8 | msb << 16 | MSB << 24);
			numerator = myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] - myTRCFile->electrodesList[posInVector].logicGround;
			myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = (numerator / denominator) * multiplicator;
		}
	}
}

void MicromedLibrary::TRCFunctions::binaryToDigitalDataMultipleChanels(TRCFile *myTRCFile, char *binaryEEGData)
{
	float numerator, denominator, multiplicator;
	int numberSample = (myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes));

	myTRCFile->eegAllChanels.resize(myTRCFile->electrodesList.size(), std::vector<float>(numberSample));

	if (myTRCFile->header4.numberBytes == 2)
	{
		unsigned char LSB, MSB;
		for (int i = 0; i < numberSample; i++)
		{
			for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
			{
				if ((j / myTRCFile->header4.numberBytes) < (myTRCFile->electrodesList.size()))
				{
					int currentPos = myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].idExtractFile * myTRCFile->header4.numberBytes;

					LSB = (unsigned char)binaryEEGData[currentPos + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
					MSB = (unsigned char)binaryEEGData[currentPos + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];//<< 8;

					myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = float(LSB | MSB << 8);
					numerator = myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i]
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicGround;
					denominator = (float)(myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMaximum
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMinimum) + 1;
					multiplicator = (float)myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMaximum
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMinimum;
					myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (numerator / denominator) * multiplicator;
				}
			}
		}
	}
	else if (myTRCFile->header4.numberBytes == 4)
	{
		unsigned char LSB, lsb, msb, MSB;
		for (int i = 0; i < numberSample; i++)
		{
			for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
			{
				if ((j / myTRCFile->header4.numberBytes) < (myTRCFile->electrodesList.size()))
				{
					int currentPos = myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].idExtractFile * myTRCFile->header4.numberBytes;

					LSB = (unsigned char)binaryEEGData[currentPos + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
					lsb = (unsigned char)binaryEEGData[currentPos + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
					msb = (unsigned char)binaryEEGData[currentPos + 2 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
					MSB = (unsigned char)binaryEEGData[currentPos + 3 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];

					myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = float(LSB | lsb << 8 | msb << 16 | MSB << 24);
					numerator = myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i]
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicGround;
					denominator = (float)(myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMaximum
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMinimum) + 1;
					multiplicator = (float)myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMaximum
						- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMinimum;
					myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (numerator / denominator) * multiplicator;
				}
			}
		}
	}
}

void MicromedLibrary::TRCFunctions::readBinaryDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData)
{
	try
	{
		ifstream fileStream(myTRCFile->myFilePath, ios::binary);
		fileStream.seekg(myTRCFile->header4.adressFirstData, std::ios::beg);
		fileStream.read(&binaryEEGData[0], myTRCFile->dataSize);
		fileStream.close();
	}
	catch (system_error& e)
	{
		cerr << "Error Reading Binary Data of All Chanels" << endl;
		cerr << e.code().message().c_str() << endl;
	}
}

void MicromedLibrary::TRCFunctions::binaryToDigitalDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData)
{
	float numerator, denominator, multiplicator;
	int numberSample = (myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes));

	myTRCFile->eegAllChanels.resize(myTRCFile->header4.numberStoredChannels, std::vector<float>(numberSample));

	if (myTRCFile->header4.numberBytes == 2)
	{
		unsigned char LSB, MSB;
		for (int i = 0; i < numberSample; i++)
		{
			for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
			{
				LSB = (unsigned char)binaryEEGData[j + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
				MSB = (unsigned char)binaryEEGData[j + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];

				myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = float(LSB | MSB << 8);
				numerator = myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i]
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicGround;
				denominator = (float)(myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMaximum
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMinimum) + 1;
				multiplicator = (float)myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMaximum
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMinimum;
				myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (numerator / denominator) * multiplicator;
			}
		}
	}
	else if(myTRCFile->header4.numberBytes == 4)
	{
		unsigned char LSB, lsb, msb, MSB;
		for (int i = 0; i < numberSample; i++)
		{
			for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
			{
				LSB = (unsigned char)binaryEEGData[j + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
				lsb = (unsigned char)binaryEEGData[j + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
				msb = (unsigned char)binaryEEGData[j + 2 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
				MSB = (unsigned char)binaryEEGData[j + 3 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];

				myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = float(LSB | lsb << 8 | msb << 16 | MSB << 24);
				numerator = myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i]
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicGround;
				denominator = (float)(myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMaximum
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].logicMinimum) + 1;
				multiplicator = (float)myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMaximum
					- myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].physicMinimum;
				myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (numerator / denominator) * multiplicator;
			}
		}
	}
}

//===[ Data Write ]===
//= write from orig
void MicromedLibrary::TRCFunctions::copyBinaryHeader(std::ofstream &writeStream, std::ifstream &readStream, int adressFirstData)
{
	int buffer[2];
	while (readStream.read((char *)&buffer, sizeof(buffer)) && readStream.tellg() <= adressFirstData)
	{
		writeStream.write((char *)&buffer, sizeof(buffer));
	}
}

void MicromedLibrary::TRCFunctions::modifyBinaryHeaderValue(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile)
{
	std::string tempLabelPos, tempLabelNeg;
	unsigned char zero = 0, one = 1;
	int count = 0, found = false;

	myTRCFile->header4.numberStoredChannels = myTRCFile->electrodesList.size();
	outputFileStream.seekp(142, std::ios::beg);
	outputFileStream << (char *)&myTRCFile->header4.numberStoredChannels;

	myTRCFile->header4.multiplexer = myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes;
	outputFileStream.seekp(144, std::ios::beg);
	outputFileStream << (char *)&myTRCFile->header4.multiplexer;

	//Time to change the status value of electrodes that might have been removed
	//First every status flag at 0
	for (int i = 0; i < MAX_LAB; i++)
	{
		outputFileStream.seekp(myTRCFile->header4.descriptorElectrode.startOffset + (i * 128), std::ios::beg);
		outputFileStream << zero;
	}

	//Then we reread the PositiveLabel of each elec and if it still in the data struct then status = 1
	for (int i = 0; i < MAX_LAB; i++)
	{
		tempLabelPos = myTRCFile->binaryStringExtraction(myTRCFile->header4.descriptorElectrode.startOffset + 2 + (i * 128), 6, readStream);
		tempLabelNeg = myTRCFile->binaryStringExtraction(myTRCFile->header4.descriptorElectrode.startOffset + 8 + (i * 128), 6, readStream);

		outputFileStream.seekp(myTRCFile->header4.descriptorElectrode.startOffset + (i * 128), std::ios::beg);

		while (count < myTRCFile->electrodes().size() && found == false)
		{
			if (strcmp(tempLabelPos.c_str(), "G2") == 0 && strcmp(tempLabelNeg.c_str(), "G2") == 0)
			{
				outputFileStream << zero;
				found = true;
			}
			else if (strcmp(tempLabelPos.c_str(), myTRCFile->electrodes()[count].positiveInputLabel.c_str()) == 0 &&
				strcmp(tempLabelNeg.c_str(), myTRCFile->electrodes()[count].negativeInputLabel.c_str()) == 0)
			{
				outputFileStream << one;
				found = true;
			}
			count++;
		}
		found = false;
		count = 0;
	}
}

void MicromedLibrary::TRCFunctions::readAndWriteBinaryDataMultiChanels(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile)
{
	char *buffer = new char[(int)myTRCFile->dataSize];

	readStream.clear();
	readStream.seekg(myTRCFile->header4.adressFirstData, std::ios::beg);
	readStream.read(&buffer[0], myTRCFile->dataSize);

	outputFileStream.seekp(myTRCFile->header4.adressFirstData, std::ios::beg);
	for (int i = 0; i < (int)(myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes)); i++)
	{
		for (int j = 0; j < myTRCFile->electrodes().size(); j++)
		{
			for (int k = 0; k < myTRCFile->header4.numberBytes; k++)
			{
				outputFileStream << (char)buffer[((myTRCFile->electrodes()[j].idExtractFile * myTRCFile->header4.numberBytes) + k) +
					(i * myTRCFile->header4.numberBytes * myTRCFile->header4.numberStoredChannels)];
			}
		}
	}

	delete[] buffer;
}

//= write from data
void MicromedLibrary::TRCFunctions::writeHeader(ofstream &outputFileStream, headerType4 *header)
{
	//== Title
	outputFileStream.seekp(0, std::ios::beg);
	outputFileStream << (char *)header->title.c_str();
	//== Lab
	outputFileStream.seekp(32, std::ios::beg);
	outputFileStream << (char *)header->laboratory.c_str();
	//== Patient Data
	outputFileStream.seekp(64, std::ios::beg);
	outputFileStream << (char *)header->patient.surname.c_str();
	outputFileStream.seekp(86, std::ios::beg);
	outputFileStream << (char *)header->patient.name.c_str();
	outputFileStream.seekp(106, std::ios::beg);
	outputFileStream << header->patient.month;
	outputFileStream.seekp(107, std::ios::beg);
	outputFileStream << header->patient.day;
	outputFileStream.seekp(108, std::ios::beg);
	outputFileStream << header->patient.year;
	outputFileStream.seekp(109, std::ios::beg);
	outputFileStream << header->patient.reserved;
	//== Recording Date
	outputFileStream.seekp(128, std::ios::beg);
	outputFileStream << header->recordDate.day;
	outputFileStream.seekp(129, std::ios::beg);
	outputFileStream << header->recordDate.month;
	outputFileStream.seekp(130, std::ios::beg);
	outputFileStream << header->recordDate.year;
	//== Recording Time
	outputFileStream.seekp(131, std::ios::beg);
	outputFileStream << header->beginTime.hour;
	outputFileStream.seekp(132, std::ios::beg);
	outputFileStream << header->beginTime.min;
	outputFileStream.seekp(133, std::ios::beg);
	outputFileStream << header->beginTime.sec;
	//== Acquisition Equipement
	outputFileStream.seekp(134, std::ios::beg);
	outputFileStream.write((char const *)&header->acquisitionUnit, sizeof(unsigned short));
	//==File Type
	outputFileStream.seekp(136, std::ios::beg);
	outputFileStream.write((char const *)&header->fileType, sizeof(unsigned short));
	//==Adress of first Data
	outputFileStream.seekp(138, std::ios::beg);
	outputFileStream.write((char const *)&header->adressFirstData, sizeof(unsigned long));
	//==Number Stored Chanels
	outputFileStream.seekp(142, std::ios::beg);
	outputFileStream.write((char const *)&header->numberStoredChannels, sizeof(unsigned short));
	//==Multiplexer
	outputFileStream.seekp(144, std::ios::beg);
	outputFileStream.write((char const *)&header->multiplexer, sizeof(unsigned short));
	//==Sampling Rate
	outputFileStream.seekp(146, std::ios::beg);
	outputFileStream.write((char const *)&header->samplingRate, sizeof(unsigned short));
	//==Number of bytes
	outputFileStream.seekp(148, std::ios::beg);
	outputFileStream.write((char const *)&header->numberBytes, sizeof(unsigned short));
	//==Compression
	outputFileStream.seekp(150, std::ios::beg);
	outputFileStream.write((char const *)&header->compression, sizeof(unsigned short));
	//==Number of montages stored
	outputFileStream.seekp(152, std::ios::beg);
	outputFileStream.write((char const *)&header->numberMontagesStored, sizeof(unsigned short));
	//==Digital video start
	outputFileStream.seekp(154, std::ios::beg);
	outputFileStream.write((char const *)&header->digitalVideoStartSample, sizeof(unsigned long));
	//==mpeg vid sync
	outputFileStream.seekp(158, std::ios::beg);
	outputFileStream.write((char const *)&header->mpegDelay, sizeof(unsigned short));
	//==Reserved further
	outputFileStream.seekp(160, std::ios::beg);
	outputFileStream << header->reservedFurtherDEV;
	//==Header type
	outputFileStream.seekp(175, std::ios::beg);
	outputFileStream << header->headerType;
}

void MicromedLibrary::TRCFunctions::writeDescriptor(ofstream &outputFileStream, descriptorArea *descriptor, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	outputFileStream << descriptor->name.c_str();
	outputFileStream.seekp(positionFile + 8, std::ios::beg);
	outputFileStream.write((char const *)&descriptor->startOffset, sizeof(unsigned long));
	outputFileStream.seekp(positionFile + 12, std::ios::beg);
	outputFileStream.write((char const *)&descriptor->length, sizeof(unsigned long));
}

void MicromedLibrary::TRCFunctions::writeOrderStorageElectrodes(ofstream &outputFileStream, vector<unsigned short int> & orderStorageElec, int positionFile)
{
	unsigned char zero = 0;

	for (int i = 0; i < MAX_CAN; i++)
	{
		outputFileStream.seekp(positionFile + (i * 2), std::ios::beg);
		if (i < orderStorageElec.size())
			outputFileStream << (char *)&orderStorageElec[i];
		else
			outputFileStream << zero;
	}
}

void MicromedLibrary::TRCFunctions::writeElectrodes(ofstream &outputFileStream, TRCFile *myTRCFile)
{
	//==Write Elec
	//Always g2 first
	outputFileStream.seekp(1154, std::ios::beg);
	outputFileStream << (char *)"G2";
	outputFileStream.seekp(1160, std::ios::beg);
	outputFileStream << (char *)"G2";
	outputFileStream.seekp(1166, std::ios::beg);
	long logMin = 0;
	outputFileStream.write((char const *)&logMin, sizeof(long));
	outputFileStream.seekp(1170, std::ios::beg);
	long logMax = 65535;
	outputFileStream.write((char const *)&logMax, sizeof(long));
	long logGround = 32768;
	outputFileStream.seekp(1174, std::ios::beg);
	outputFileStream.write((char const *)&logGround, sizeof(long));
	long phyMin = -3200;
	outputFileStream.seekp(1178, std::ios::beg);
	outputFileStream.write((char const *)&phyMin, sizeof(long));
	long phyMax = 3200;
	outputFileStream.seekp(1182, std::ios::beg);
	outputFileStream.write((char const *)&phyMax, sizeof(long));
	long preHiPassLi = 150;
	outputFileStream.seekp(1188, std::ios::beg);
	outputFileStream.write((char const *)&preHiPassLi, sizeof(unsigned short));
	unsigned short rateCoeff = 1;
	outputFileStream.seekp(1196, std::ios::beg);
	outputFileStream.write((char const *)&rateCoeff, sizeof(unsigned short));
	unsigned short posit = 128;
	outputFileStream.seekp(1198, std::ios::beg);
	outputFileStream.write((char const *)&posit, sizeof(unsigned short));
	unsigned short isInAvg = 1;
	outputFileStream.seekp(1209, std::ios::beg);
	outputFileStream.write((char const *)&isInAvg, sizeof(unsigned short));
	outputFileStream.seekp(1256, std::ios::beg);
	outputFileStream << myTRCFile->electrodesList[0].freeFurtherUse.c_str();

	//==Write order storage Elec
	for (int i = 0; i < MAX_LAB; i++)
	{
		if (i < myTRCFile->electrodesList.size())
		{
			unsigned short int offsetElec = 1152 + (myTRCFile->orderStorageElec[i] * 128);
			outputFileStream.seekp(offsetElec, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].status;
			outputFileStream.seekp(offsetElec + 1, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].type;
			outputFileStream.seekp(offsetElec + 2, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].positiveInputLabel.c_str();
			outputFileStream.seekp(offsetElec + 8, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].negativeInputLabel.c_str();
			outputFileStream.seekp(offsetElec + 14, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].logicMinimum, sizeof(long));
			outputFileStream.seekp(offsetElec + 18, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].logicMaximum, sizeof(long));
			outputFileStream.seekp(offsetElec + 22, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].logicGround, sizeof(long));
			outputFileStream.seekp(offsetElec + 26, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].physicMinimum, sizeof(long));
			outputFileStream.seekp(offsetElec + 30, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].physicMaximum, sizeof(long));
			outputFileStream.seekp(offsetElec + 34, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].measurementUnit, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 36, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].prefilteringHighPassLimit, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 38, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].prefilteringHighPassType, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 40, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].prefilteringLowPassLimit, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 42, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].prefilteringLowPassType, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 44, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].rateCoefficient, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 46, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].position, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 48, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].lattitude, sizeof(float));
			outputFileStream.seekp(offsetElec + 52, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].longitude, sizeof(float));
			outputFileStream.seekp(offsetElec + 56, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].presentInMap, sizeof(unsigned char));
			outputFileStream.seekp(offsetElec + 57, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].isInAvg, sizeof(unsigned char));
			outputFileStream.seekp(offsetElec + 58, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].description.c_str();
			outputFileStream.seekp(offsetElec + 90, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].coordinate3DX, sizeof(float));
			outputFileStream.seekp(offsetElec + 94, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].coordinate3DY, sizeof(float));
			outputFileStream.seekp(offsetElec + 98, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].coordinate3DZ, sizeof(float));
			outputFileStream.seekp(offsetElec + 102, std::ios::beg);
			outputFileStream.write((char const *)&myTRCFile->electrodesList[i].corrdinateType, sizeof(unsigned short));
			outputFileStream.seekp(offsetElec + 104, std::ios::beg);
			outputFileStream << myTRCFile->electrodesList[i].freeFurtherUse.c_str();
		}
	}
}

void MicromedLibrary::TRCFunctions::writeNotes(ofstream &outputFileStream, vector<operatorNote> &notesList, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_NOTE; i++)
	{
		if (i < notesList.size())
		{
			outputFileStream.seekp(positionFile + (44 * i), std::ios::beg);
			outputFileStream.write((char const *)&notesList[i].sample, sizeof(unsigned long));
			outputFileStream << notesList[i].comment.c_str();
		}
	}
}

void MicromedLibrary::TRCFunctions::writeFlags(ofstream &outputFileStream, vector<selectionFlag> &flagList, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_FLAG; i++)
	{
		if (i < flagList.size())
		{
			outputFileStream.write((char const *)&flagList[i].begin, sizeof(long));
			outputFileStream.write((char const *)&flagList[i].end, sizeof(long));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeReductionInfo(ofstream &outputFileStream, vector<reducedFileDescription> & reductionLists, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_SEGM; i++)
	{
		if (i < reductionLists.size())
		{
			outputFileStream.write((char const *)&reductionLists[i].time, sizeof(unsigned long));
			outputFileStream.write((char const *)&reductionLists[i].sample, sizeof(unsigned long));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeBeginImpedance(ofstream &outputFileStream, vector<impedanceFile> &beginImpedances, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_CAN; i++)
	{
		if (i < beginImpedances.size())
		{
			outputFileStream.write((char const *)&beginImpedances[i].positive, sizeof(unsigned char));
			outputFileStream.write((char const *)&beginImpedances[i].negative, sizeof(unsigned char));
		}
		else
		{
			unsigned char charNo = 255;
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeEndImpedance(ofstream &outputFileStream, vector<impedanceFile> &endImpedances, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_CAN; i++)
	{
		if (i < endImpedances.size())
		{
			outputFileStream.write((char const *)&endImpedances[i].positive, sizeof(unsigned char));
			outputFileStream.write((char const *)&endImpedances[i].negative, sizeof(unsigned char));
		}
		else
		{
			unsigned char charNo = 255;
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeMontages(ofstream &outputFileStream, vector<montagesOfTrace> &montageList, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_MONT; i++)
	{
		if (i < montageList.size())
		{
			outputFileStream.seekp(positionFile + (4096 * i), std::ios::beg);
			outputFileStream.write((char const *)&montageList[i].lines, sizeof(unsigned short));
			outputFileStream.write((char const *)&montageList[i].sectors, sizeof(unsigned short));
			outputFileStream.write((char const *)&montageList[i].baseTime, sizeof(unsigned short));
			outputFileStream.write((char const *)&montageList[i].notch, sizeof(unsigned short));
			outputFileStream.write((char const *)&montageList[i].colour, sizeof(unsigned char[128]));
			outputFileStream.write((char const *)&montageList[i].selection, sizeof(unsigned char[128]));
			outputFileStream.write((char const *)&montageList[i].description, sizeof(char[64]));
			for (int j = 0; j < MAX_CAN_VIEW; j++)
			{
				outputFileStream.write((char const *)&montageList[i].inputs[j].nonInverting, sizeof(unsigned short));
				outputFileStream.write((char const *)&montageList[i].inputs[j].inverting, sizeof(unsigned short));
			}
			for (int j = 0; j < MAX_CAN_VIEW; j++)
			{
				outputFileStream.write((char const *)&montageList[i].highPassFilter[j], sizeof(unsigned long));
			}
			for (int j = 0; j < MAX_CAN_VIEW; j++)
			{
				outputFileStream.write((char const *)&montageList[i].lowPassFilter[j], sizeof(unsigned long));
			}
			for (int j = 0; j < MAX_CAN_VIEW; j++)
			{
				outputFileStream.write((char const *)&montageList[i].reference[j], sizeof(unsigned long));
			}
			outputFileStream.write((char const *)&montageList[i].free, sizeof(unsigned char[1720]));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeCompression(ofstream &outputFileStream, compressionDescription *compression, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	outputFileStream.write((char const *)&compression->compressionStuff, sizeof(char[10]));
}

void MicromedLibrary::TRCFunctions::writeAverage(ofstream &outputFileStream, offlineAverageProcess *averageParameters, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	outputFileStream.write((char const *)&averageParameters->meanTrace, sizeof(unsigned long));
	outputFileStream.write((char const *)&averageParameters->meanFile, sizeof(unsigned long));
	outputFileStream.write((char const *)&averageParameters->meanPrestim, sizeof(unsigned long));
	outputFileStream.write((char const *)&averageParameters->meanPoststim, sizeof(unsigned long));
	outputFileStream.write((char const *)&averageParameters->meanType, sizeof(unsigned long));
	outputFileStream.write((char const *)&averageParameters->freeFurtherUse, sizeof(unsigned char[108]));
}

void MicromedLibrary::TRCFunctions::writeDVideo(ofstream &outputFileStream, vector<dVideoFiles> &dvidTRC, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_FILE; i++)
	{
		if (i < dvidTRC.size())
			outputFileStream.write((char const *)&dvidTRC[i].DV_Begin, sizeof(unsigned long));
	}
}

void MicromedLibrary::TRCFunctions::writeEventA(ofstream &outputFileStream, eventsMarker &eventA, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	outputFileStream.write((char const *)&eventA.description, sizeof(char[64]));

	for (int i = 0; i < MAX_EVENT; i++)
	{
		outputFileStream.seekp(positionFile + 64 + (8 * i), std::ios::beg);
		outputFileStream.write((char const *)&eventA.selection[i].begin, sizeof(long));
		outputFileStream.seekp(positionFile + 68 + (8 * i), std::ios::beg);
		outputFileStream.write((char const *)&eventA.selection[i].end, sizeof(long));
	}
}

void MicromedLibrary::TRCFunctions::writeEventB(ofstream &outputFileStream, eventsMarker &eventB, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	outputFileStream.write((char const *)&eventB.description, sizeof(char[64]));

	for (int i = 0; i < MAX_EVENT; i++)
	{
		outputFileStream.seekp(positionFile + 64 + (8 * i), std::ios::beg);
		outputFileStream.write((char const *)&eventB.selection[i].begin, sizeof(long));
		outputFileStream.seekp(positionFile + 68 + (8 * i), std::ios::beg);
		outputFileStream.write((char const *)&eventB.selection[i].end, sizeof(long));
	}
}

void MicromedLibrary::TRCFunctions::writeHistoryInfo(ofstream &outputFileStream, vector<unsigned long int> & historySam, vector<montagesOfTrace> &montagesHistoryList, int positionFile)
{
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_SAMPLE; i++)
	{
		if (i < historySam.size())
		{
			outputFileStream.write((char const *)&historySam[i], sizeof(unsigned long int));
		}
		else
		{
			unsigned char charNo = 255;
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
			outputFileStream.write((char const *)&charNo, sizeof(unsigned char));
		}
	}

	int positionOffset = positionFile + (MAX_SAMPLE * sizeof(unsigned long int));
	outputFileStream.seekp(positionOffset, std::ios::beg);
	for (int i = 0; i < MAX_MONT; i++)
	{
		outputFileStream.seekp(positionOffset + (4096 * i), std::ios::beg);
		outputFileStream.write((char const *)&montagesHistoryList[i].lines, sizeof(unsigned short));
		outputFileStream.write((char const *)&montagesHistoryList[i].sectors, sizeof(unsigned short));
		outputFileStream.write((char const *)&montagesHistoryList[i].baseTime, sizeof(unsigned short));
		outputFileStream.write((char const *)&montagesHistoryList[i].notch, sizeof(unsigned short));
		outputFileStream.seekp(positionOffset + 8 + (4096 * i), std::ios::beg);
		outputFileStream.write((char const *)&montagesHistoryList[i].colour, sizeof(unsigned char[128]));
		outputFileStream.seekp(positionOffset + 136 + (4096 * i), std::ios::beg);
		outputFileStream.write((char const *)&montagesHistoryList[i].selection, sizeof(unsigned char[128]));
		outputFileStream.write((char const *)&montagesHistoryList[i].description, sizeof(char[64]));
		for (int j = 0; j < MAX_CAN_VIEW; j++)
		{
			outputFileStream.seekp(positionOffset + 328 + (4 * j) + (4096 * i), std::ios::beg);
			outputFileStream.write((char const *)&montagesHistoryList[i].inputs[j].nonInverting, sizeof(unsigned short));
			outputFileStream.write((char const *)&montagesHistoryList[i].inputs[j].inverting, sizeof(unsigned short));
		}
		outputFileStream.seekp(positionOffset + 840 + (4096 * i), std::ios::beg);
		outputFileStream.write((char const *)&montagesHistoryList[i].highPassFilter, sizeof(unsigned long[128]));
		outputFileStream.write((char const *)&montagesHistoryList[i].lowPassFilter, sizeof(unsigned long[128]));
		outputFileStream.write((char const *)&montagesHistoryList[i].reference, sizeof(unsigned long[128]));
		outputFileStream.write((char const *)&montagesHistoryList[i].free, sizeof(unsigned char[1720]));
	}
}

void MicromedLibrary::TRCFunctions::writeDigitalTriggers(ofstream &outputFileStream, std::vector<digitalTriggers> & digiTriggers, int positionFile)
{
	std::cout << positionFile << std::endl;
	outputFileStream.seekp(positionFile, std::ios::beg);
	for (int i = 0; i < MAX_TRIGGER; i++)
	{
		if (i < digiTriggers.size())
		{
			outputFileStream.write((char const *)&digiTriggers[i].triggerSample, sizeof(unsigned long));
			outputFileStream.write((char const *)&digiTriggers[i].triggerValue, sizeof(unsigned short));
		}
		else
		{
			unsigned long sampNo = -1;
			unsigned short valNo = 65535;
			outputFileStream.write((char const *)&sampNo, sizeof(unsigned long));
			outputFileStream.write((char const *)&valNo, sizeof(unsigned short));
		}
	}
}

void MicromedLibrary::TRCFunctions::writeDataChanels(ofstream &outputFileStream, TRCFile *myTRCFile)
{
	//==Write Data
	convertAnalogDataToDigital(myTRCFile);

	char *buffer = new char[myTRCFile->dataSize];

	if (myTRCFile->header4.numberBytes == 2)
	{
		for (int i = 0; i < myTRCFile->eegAllChanels[0].size(); i++)
		{
			for (int j = 0; j < myTRCFile->eegAllChanels.size(); j++)
			{
				unsigned short int val = (unsigned short)myTRCFile->eegAllChanels[j][i];
				char *valBuff = (char *)&val;
				for (int k = 0; k < myTRCFile->header4.numberBytes; k++)
					buffer[(myTRCFile->header4.numberBytes * j) + k + (i * myTRCFile->eegAllChanels.size() * myTRCFile->header4.numberBytes)] = valBuff[k];
			}
		}
	}
	else if (myTRCFile->header4.numberBytes == 4)
	{
		for (int i = 0; i < myTRCFile->eegAllChanels[0].size(); i++)
		{
			for (int j = 0; j < myTRCFile->eegAllChanels.size(); j++)
			{
				unsigned int val = (unsigned int)myTRCFile->eegAllChanels[j][i];
				char *valBuff = (char *)&val;
				for (int k = 0; k < myTRCFile->header4.numberBytes; k++)
					buffer[(myTRCFile->header4.numberBytes * j) + k + (i * myTRCFile->eegAllChanels.size() * myTRCFile->header4.numberBytes)] = valBuff[k];
			}
		}
	}

	outputFileStream.seekp(myTRCFile->header4.adressFirstData, std::ios::beg);
	outputFileStream.write(buffer, myTRCFile->dataSize);
	delete buffer;
}

void MicromedLibrary::TRCFunctions::concatNotes(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	for (int i = 0; i < myOtherTRCFile->notesList.size(); i++)
	{
		operatorNote newNote;
		newNote.comment = myOtherTRCFile->notesList[i].comment;
		newNote.sample = myOtherTRCFile->notesList[i].sample + concatFile->nbSample();

		concatFile->notesList.push_back(newNote);
	}
}

void MicromedLibrary::TRCFunctions::concatFlags(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	for (int i = 0; i < myOtherTRCFile->flagsList.size(); i++)
	{
		selectionFlag newFlag;
		newFlag.begin = myOtherTRCFile->flagsList[i].begin + concatFile->nbSample();
		newFlag.end = myOtherTRCFile->flagsList[i].end + concatFile->nbSample();

		concatFile->flagsList.push_back(newFlag);
	}
}

void MicromedLibrary::TRCFunctions::concatEventA(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	if (strcmp(concatFile->eventA.description, myOtherTRCFile->eventA.description) == 0)
		cout << "Same Event A" << endl;
}

void MicromedLibrary::TRCFunctions::concatEventB(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	if (strcmp(concatFile->eventB.description, myOtherTRCFile->eventB.description) == 0)
		cout << "Same Event B" << endl;
}

void MicromedLibrary::TRCFunctions::concatTriggers(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	for (int i = 0; i < myOtherTRCFile->triggersList.size(); i++)
	{
		digitalTriggers newTrigger;
		newTrigger.triggerValue = myOtherTRCFile->triggersList[i].triggerValue;
		newTrigger.triggerSample = myOtherTRCFile->triggersList[i].triggerSample + concatFile->nbSample();
		concatFile->triggersList.push_back(newTrigger);
	}
}

void MicromedLibrary::TRCFunctions::concatDataChanels(TRCFile *myOtherTRCFile, TRCFile *concatFile)
{
	int sam1 = concatFile->nbSample();
	int sam2 = myOtherTRCFile->nbSample();
	int newSizeSample = sam1 + sam2;

	for (int i = 0; i < concatFile->eegAllChanels.size(); i++)
	{
		concatFile->eegAllChanels[i].resize(newSizeSample);
		move(myOtherTRCFile->eegAllChanels[i].begin(), myOtherTRCFile->eegAllChanels[i].end(),
			concatFile->eegAllChanels[i].begin() + sam1);
	}

	concatFile->setDataSize(concatFile->eegAllChanels.size() * concatFile->eegAllChanels[0].size() * concatFile->header4.numberBytes);
}