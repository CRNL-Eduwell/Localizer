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
			myTRCFile->eegAllChanels[i][j] = ((myTRCFile->eegAllChanels[i][j] * mul) / div);
		}
	}
}
//=== Data Extract

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

//=== Data Write

void MicromedLibrary::TRCFunctions::writeTRCFileFromOriginal(ofstream &outputFileStream, TRCFile *myTRCFile)
{
	std::ifstream infile;
	infile.open(myTRCFile->myFilePath.c_str(), std::ios::binary | std::ios::in);

	copyBinaryHeader(outputFileStream, infile, myTRCFile->header4.adressFirstData);
	modifyBinaryHeaderValue(outputFileStream, infile, myTRCFile);
	readAndWriteBinaryDataMultiChanels(outputFileStream, infile, myTRCFile);
	//===========
	outputFileStream.close();
	infile.close();
}

//Debug/Tools

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
//=== Data Extract
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
	unsigned char MSB, LSB;
	float numerator, denominator, multiplicator;
	int numberSample = myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels *  myTRCFile->header4.numberBytes);
	myTRCFile->eegOneChanel.resize(numberSample);

	denominator = (float)(myTRCFile->electrodesList[posInVector].logicMaximum - myTRCFile->electrodesList[posInVector].logicMinimum) + 1;
	multiplicator = (float)myTRCFile->electrodesList[posInVector].physicMaximum - myTRCFile->electrodesList[posInVector].physicMinimum;

	for (int i = 0; i < numberSample; i += myTRCFile->header4.numberBytes)  //taille des données/nombre canal
	{
		MSB = (unsigned char)binaryEEGData[i + 1]; //<< 8;
		LSB = (unsigned char)binaryEEGData[i];
		myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = (float)(LSB | MSB << 8);
		numerator = myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] - myTRCFile->electrodesList[posInVector].logicGround;
		myTRCFile->eegOneChanel[i / myTRCFile->header4.numberBytes] = (numerator / denominator) * multiplicator;
	}
}

void MicromedLibrary::TRCFunctions::binaryToDigitalDataMultipleChanels(TRCFile *myTRCFile, char *binaryEEGData)
{
	unsigned char MSB, LSB;
	float numerator, denominator, multiplicator;
	int numberSample = (myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes));

	myTRCFile->eegAllChanels.resize(myTRCFile->electrodesList.size(), std::vector<float>(numberSample));

	for (int i = 0; i < numberSample; i++)
	{
		for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
		{
			if ((j / myTRCFile->header4.numberBytes) < (myTRCFile->electrodesList.size()))
			{
				int currentPos = myTRCFile->electrodesList[j / myTRCFile->header4.numberBytes].idExtractFile * myTRCFile->header4.numberBytes;
				MSB = (unsigned char)binaryEEGData[currentPos + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];//<< 8;
				LSB = (unsigned char)binaryEEGData[currentPos + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
				myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (float)(LSB | MSB << 8);
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
	unsigned char MSB, LSB;
	float numerator, denominator, multiplicator;
	int numberSample = (myTRCFile->dataSize / (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes));

	myTRCFile->eegAllChanels.resize(myTRCFile->header4.numberStoredChannels, std::vector<float>(numberSample));

	for (int i = 0; i < numberSample; i++)
	{
		for (int j = 0; j < (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes); j += myTRCFile->header4.numberBytes)
		{
			MSB = (unsigned char)binaryEEGData[j + 1 + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];//<< 8;
			LSB = (unsigned char)binaryEEGData[j + (myTRCFile->header4.numberStoredChannels * myTRCFile->header4.numberBytes * i)];
			myTRCFile->eegAllChanels[j / myTRCFile->header4.numberBytes][i] = (float)(LSB | MSB << 8);
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

//=== Data Write

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
			outputFileStream << (char)buffer[(myTRCFile->electrodes()[j].position * myTRCFile->header4.numberBytes) +
				(i * myTRCFile->header4.numberBytes * myTRCFile->header4.numberStoredChannels)];
			outputFileStream << (char)buffer[((myTRCFile->electrodes()[j].position * myTRCFile->header4.numberBytes) + 1) +
				(i * myTRCFile->header4.numberBytes * myTRCFile->header4.numberStoredChannels)];
		}
	}

	delete[] buffer;
}