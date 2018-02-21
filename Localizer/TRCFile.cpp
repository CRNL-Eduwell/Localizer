#include "TRCFile.h"

//===
MicromedLibrary::TRCFile::TRCFile()
{

}

MicromedLibrary::TRCFile::TRCFile(string pathTRCFile)
{
	myFilePath = pathTRCFile;

	try
	{
		ifstream sr(myFilePath, ios::binary);
		getHeaderInformations(sr);
		getDescriptorInfo(sr, &header4.descriptorCode, 176);
		getDescriptorInfo(sr, &header4.descriptorElectrode, 192);
		getDescriptorInfo(sr, &header4.descriptorNote, 208);
		getDescriptorInfo(sr, &header4.descriptorFlag, 224);
		getDescriptorInfo(sr, &header4.descriptorReduction, 240);
		getDescriptorInfo(sr, &header4.descriptorBeginImpedance, 256);
		getDescriptorInfo(sr, &header4.descriptorEndImpedance, 272);
		getDescriptorInfo(sr, &header4.descriptorMontages, 288);
		getDescriptorInfo(sr, &header4.descriptorCompression, 304);
		getDescriptorInfo(sr, &header4.descriptorAverage, 320);
		getDescriptorInfo(sr, &header4.descriptorHistory, 336);
		getDescriptorInfo(sr, &header4.descriptorDVideo, 352);
		getDescriptorInfo(sr, &header4.descriptorEventA, 368);
		getDescriptorInfo(sr, &header4.descriptorEventB, 384);
		getDescriptorInfo(sr, &header4.descriptorTrigger, 400);

		getOrderStorageElec(sr, &header4.descriptorCode, orderStorageElec);
		getUsedElectrodes(sr, &header4.descriptorElectrode, electrodesList);
		sortElectrodes(electrodesList);
		getNotes(sr, &header4.descriptorNote, notesList);
		getFlags(sr, &header4.descriptorFlag, flagsList);
		getReductionInfo(sr, &header4.descriptorReduction, reductionLists);
		truncatedFileBeginTime(reductionLists);
		truncatedFileEndTime(reductionLists);
		getBeginImpedance(sr, &header4.descriptorBeginImpedance, startingImpedance);
		getEndImpedance(sr, &header4.descriptorEndImpedance, endingImpedance);
		getMontages(sr, &header4.descriptorMontages, montagesList);
		getCompressionInfo(sr, &header4.descriptorCompression, compression);
		getHistoryInfo(sr, &header4.descriptorHistory, historySample, montagesHistoryList);
		getAverageInfo(sr, &header4.descriptorAverage, averageParameters);
		getDVideoInfo(sr, &header4.descriptorDVideo, digitalVideos);
		getEventA(sr, &header4.descriptorEventA, eventA);
		getEventB(sr, &header4.descriptorEventB, eventB);
		getDigitalTriggers(sr, &header4.descriptorTrigger, triggersList);

		sr.close();
	}
	catch (system_error& e)
	{
		cerr << "Error opening .TRC File" << endl;
		cerr << e.code().message().c_str() << endl;
	}
}

MicromedLibrary::TRCFile::~TRCFile()
{

}

//===

string MicromedLibrary::TRCFile::filePath()
{
	return myFilePath;
}

MicromedLibrary::headerType4 & MicromedLibrary::TRCFile::header()
{
	return header4;
}

unsigned long MicromedLibrary::TRCFile::DataSize()
{
	return dataSize;
}

void MicromedLibrary::TRCFile::setDataSize(unsigned long size)
{
	dataSize = size;
}

vector<MicromedLibrary::electrode> & MicromedLibrary::TRCFile::electrodes()
{
	return electrodesList;
}

vector<MicromedLibrary::operatorNote> & MicromedLibrary::TRCFile::notes()
{
	return notesList;
}

vector<MicromedLibrary::digitalTriggers> & MicromedLibrary::TRCFile::triggers()
{
	return triggersList;
}

int MicromedLibrary::TRCFile::nbSample()
{
	return dataSize / (header4.numberStoredChannels *  header4.numberBytes);
}

vector<float> & MicromedLibrary::TRCFile::eegDataOneChanel()
{
	return eegOneChanel;
}

vector<vector<float>> & MicromedLibrary::TRCFile::eegDataAllChanels()
{
	return eegAllChanels;
}

//===
unsigned char MicromedLibrary::TRCFile::binaryCharExtraction(int positionInFile, ifstream &sr)
{
	unsigned char buf;
	sr.clear();
	sr.seekg(positionInFile, std::ios::beg);
	buf = (unsigned char)(sr.get());
	return buf;
}

unsigned long MicromedLibrary::TRCFile::binaryBytesExtraction(int positionInFile, int numberOfBytes, ifstream &sr)
{
	unsigned long valueOfBytes = 0;
	unsigned char ucharValue;
	int count = 0;

	/*According to count value, we shift ucharValue from 8,16,24,32 ...*/
	while (count < numberOfBytes)
	{
		ucharValue = binaryCharExtraction(positionInFile, sr);
		valueOfBytes = valueOfBytes + unsigned long(ucharValue << (8 * count));

		count++;
		positionInFile++;
	}
	return valueOfBytes;
}

string MicromedLibrary::TRCFile::binaryStringExtraction(int positionInFile, int numberOfChar, ifstream &sr)
{
	char charValue = ' ';
	int count = 0;
	vector<char> myExtractedString;

	sr.clear();
	sr.seekg(positionInFile, ios::beg);
	for (int i = 0; i < numberOfChar; i++)
	{
		sr.get(charValue);
		if (charValue >= 0)
		{
			myExtractedString.push_back(charValue);
		}
	}
	return string(myExtractedString.begin(), myExtractedString.end());
}

//===
void MicromedLibrary::TRCFile::getHeaderInformations(ifstream &sr)
{
	header4.title = binaryStringExtraction(0, 32, sr);
	header4.laboratory = binaryStringExtraction(32, 32, sr);
	header4.patient.surname = binaryStringExtraction(64, 22, sr);
	header4.patient.name = binaryStringExtraction(86, 20, sr);
	header4.patient.month = binaryCharExtraction(106, sr);
	header4.patient.day = binaryCharExtraction(107, sr);
	header4.patient.year = binaryCharExtraction(108, sr);
	header4.recordDate.day = binaryCharExtraction(128, sr);
	header4.recordDate.month = binaryCharExtraction(129, sr);
	header4.recordDate.year = binaryCharExtraction(130, sr);
	header4.beginTime.hour = binaryCharExtraction(131, sr);
	header4.beginTime.min = binaryCharExtraction(132, sr);
	header4.beginTime.sec = binaryCharExtraction(133, sr);
	header4.acquisitionUnit = (unsigned short int) binaryBytesExtraction(134, 2, sr);
	header4.fileType = (unsigned short int) binaryBytesExtraction(136, 2, sr);
	header4.adressFirstData = (unsigned long)binaryBytesExtraction(138, 4, sr);
	header4.numberStoredChannels = (unsigned short int) binaryBytesExtraction(142, 2, sr);
	header4.multiplexer = (unsigned short int)binaryBytesExtraction(144, 2, sr);
	header4.samplingRate = (unsigned short int)binaryBytesExtraction(146, 2, sr);
	header4.numberBytes = (unsigned short int)binaryBytesExtraction(148, 2, sr);
	header4.compression = (unsigned short int)binaryBytesExtraction(150, 2, sr);
	header4.numberMontagesStored = (unsigned short int)binaryBytesExtraction(152, 2, sr);
	header4.digitalVideoStartSample = (unsigned long)binaryBytesExtraction(154, 4, sr);
	header4.headerType = binaryCharExtraction(175, sr);
	fileLength = getFileSize(sr);
	dataSize = fileLength - header4.adressFirstData;
}

int MicromedLibrary::TRCFile::getFileSize(ifstream &sr)
{
	sr.clear();
	sr.seekg(0, std::ios::end);
	return (int)sr.tellg();
}

//===
void MicromedLibrary::TRCFile::getDescriptorInfo(ifstream &fileStream, descriptorArea *descriptor, int positionFile)
{
	std::string nameRead = binaryStringExtraction(positionFile, 8, fileStream);
	deblankString(nameRead);
	if (nameRead == descriptor->name)
	{
		fileStream.read((char *)&descriptor->startOffset, sizeof(unsigned long int));
		fileStream.read((char *)&descriptor->length, sizeof(unsigned long int));
	}
	else
	{
		descriptor->startOffset = 0;
		descriptor->length = 0;
	}
}

//===
void MicromedLibrary::TRCFile::getOrderStorageElec(ifstream &fileStream, descriptorArea *descriptorCode, vector<unsigned short int> &orderStorageElec)
{
	unsigned short int currentOrderStorageValue = 0;
	for (int i = 0; i < int(descriptorCode->length / 2); i++)
	{
		currentOrderStorageValue = (unsigned short int)binaryBytesExtraction(descriptorCode->startOffset + (2 * i), 2, fileStream);
		if (currentOrderStorageValue != 0)
		{
			orderStorageElec.push_back(currentOrderStorageValue);
		}
	}
}

void MicromedLibrary::TRCFile::getUsedElectrodes(ifstream &fileStream, descriptorArea *descriptorElec, vector<electrode> &electrodesList)
{
	for (int i = 0; i < int(descriptorElec->length / 128); i++)
	{
		electrode currentElectrode;
		currentElectrode.status = binaryCharExtraction(descriptorElec->startOffset + (i * 128), fileStream);
		if (currentElectrode.status == 1)
		{
			currentElectrode.type = binaryCharExtraction(descriptorElec->startOffset + 1 + (i * 128), fileStream);
			currentElectrode.positiveInputLabel = binaryStringExtraction(descriptorElec->startOffset + 2 + (i * 128), 6, fileStream);
			deblankString(currentElectrode.positiveInputLabel);
			currentElectrode.negativeInputLabel = binaryStringExtraction(descriptorElec->startOffset + 8 + (i * 128), 6, fileStream);
			deblankString(currentElectrode.negativeInputLabel);
			currentElectrode.logicMinimum = binaryBytesExtraction(descriptorElec->startOffset + 14 + (i * 128), 4, fileStream);
			currentElectrode.logicMaximum = binaryBytesExtraction(descriptorElec->startOffset + 18 + (i * 128), 4, fileStream);
			currentElectrode.logicGround = binaryBytesExtraction(descriptorElec->startOffset + 22 + (i * 128), 4, fileStream);
			currentElectrode.physicMinimum = binaryBytesExtraction(descriptorElec->startOffset + 26 + (i * 128), 4, fileStream);
			currentElectrode.physicMaximum = binaryBytesExtraction(descriptorElec->startOffset + 30 + (i * 128), 4, fileStream);
			currentElectrode.measurementUnit = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 34 + (i * 128), 2, fileStream);
			currentElectrode.prefilteringHighPassLimit = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 36 + (i * 128), 2, fileStream);
			currentElectrode.prefilteringHighPassType = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 38 + (i * 128), 2, fileStream);
			currentElectrode.prefilteringLowPassLimit = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 40 + (i * 128), 2, fileStream);
			currentElectrode.prefilteringLowPassType = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 42 + (i * 128), 2, fileStream);
			currentElectrode.rateCoefficient = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 44 + (i * 128), 2, fileStream);
			currentElectrode.position = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 46 + (i * 128), 2, fileStream);
			fileStream.seekg(descriptorElec->startOffset + 48 + (i * 128));
			fileStream.read((char *)&currentElectrode.lattitude, sizeof(float));
			fileStream.seekg(descriptorElec->startOffset + 52 + (i * 128));
			fileStream.read((char *)&currentElectrode.longitude, sizeof(float));
			currentElectrode.presentInMap = binaryCharExtraction(descriptorElec->startOffset + 56 + (i * 128), fileStream);
			currentElectrode.isInAvg = binaryCharExtraction(descriptorElec->startOffset + 57 + (i * 128), fileStream);
			currentElectrode.description = binaryStringExtraction(descriptorElec->startOffset + 58 + (i * 128), 32, fileStream);
			fileStream.seekg(descriptorElec->startOffset + 90 + (i * 128));
			fileStream.read((char *)&currentElectrode.coordinate3DX, sizeof(float));
			fileStream.seekg(descriptorElec->startOffset + 94 + (i * 128));
			fileStream.read((char *)&currentElectrode.coordinate3DY, sizeof(float));
			fileStream.seekg(descriptorElec->startOffset + 98 + (i * 128));
			fileStream.read((char *)&currentElectrode.coordinate3DZ, sizeof(float));
			currentElectrode.corrdinateType = (unsigned short)binaryBytesExtraction(descriptorElec->startOffset + 102 + (i * 128), 2, fileStream);
			currentElectrode.freeFurtherUse = binaryStringExtraction(descriptorElec->startOffset + 104 + (i * 128), 24, fileStream);
			electrodesList.push_back(currentElectrode);
		}
	}
}

void MicromedLibrary::TRCFile::sortElectrodes(vector<electrode> &electrodesList)
{
	std::sort(electrodesList.begin(), electrodesList.end(),
		[](electrode firstElectrode, electrode secondElectrode) {
		return (firstElectrode.position < secondElectrode.position);
	});

	for (int i = 0; i < electrodesList.size(); i++)
	{
		electrodesList[i].idExtractFile = i;
	}
}

void MicromedLibrary::TRCFile::getNotes(ifstream &fileStream, descriptorArea *descriptorNote, vector<operatorNote> &notesList)
{
	operatorNote currentNote;

	for (int i = 0; i < int(descriptorNote->length / 44); i++)
	{
		currentNote.sample = binaryBytesExtraction(descriptorNote->startOffset + (i * 44), 4, fileStream);
		if (currentNote.sample != 0)
		{
			currentNote.comment = binaryStringExtraction(descriptorNote->startOffset + 4 + (i * 44), 40, fileStream);
			notesList.push_back(currentNote);
		}
	}
}

void MicromedLibrary::TRCFile::getFlags(ifstream &fileStream, descriptorArea *descriptorFlags, vector<selectionFlag> &flagsList)
{
	selectionFlag currentFlag;

	for (int i = 0; i < int(descriptorFlags->length / 8); i++)
	{
		currentFlag.begin = binaryBytesExtraction(descriptorFlags->startOffset + (i * 8), 4, fileStream);
		if (currentFlag.begin != 0)
		{
			currentFlag.end = (long)binaryBytesExtraction(descriptorFlags->startOffset + 4 + (i * 8), 4, fileStream);
			flagsList.push_back(currentFlag);
		}
	}
}

void MicromedLibrary::TRCFile::getReductionInfo(ifstream &fileStream, descriptorArea *descriptorReduction, vector<reducedFileDescription> &reductionList)
{
	reducedFileDescription red;

	for (int i = 0; i < int(descriptorReduction->length); i++)
	{
		red.time = binaryBytesExtraction(descriptorReduction->startOffset + (i * 4), 4, fileStream);
		if (red.time == 0)
		{
			break;
		}
		red.sample = binaryBytesExtraction(descriptorReduction->startOffset + 4 + (i * 4), 4, fileStream);
		reductionList.push_back(red);
	}
}

void MicromedLibrary::TRCFile::truncatedFileBeginTime(vector<reducedFileDescription> reductionList)
{
	unsigned long actualValueBegin = 0, valueAddBegin = 0, currentTimeValue = 0;
	/*We check if file has been truncated and if so we fix the beginning time*/

	if (reductionList.size() != 0)
	{
		actualValueBegin = (header4.beginTime.hour * 3600) + (header4.beginTime.min * 60) + (header4.beginTime.sec);
		valueAddBegin = reductionList[reductionList.size() - 1].time / header4.samplingRate;

		if (actualValueBegin < valueAddBegin)
		{
			currentTimeValue = actualValueBegin + valueAddBegin - 86400;
		}
		else
		{
			if (reductionList.size() - 1 == 0)
			{
				currentTimeValue = actualValueBegin + valueAddBegin;
			}
		}

		header4.beginTime.hour = unsigned char(currentTimeValue / 3600);
		header4.beginTime.min = unsigned char((currentTimeValue % 3600) / 60);
		header4.beginTime.sec = (currentTimeValue % 3600) % 60;
	}
}

void MicromedLibrary::TRCFile::truncatedFileEndTime(vector<reducedFileDescription> reductionList)
{
	unsigned long actualValueEnd = 0, valueAddEnd = 0, currentEndTimeValue = 0;

	actualValueEnd = (header4.beginTime.hour * 3600) + (header4.beginTime.min * 60) + (header4.beginTime.sec);
	valueAddEnd = dataSize / (header4.numberBytes * header4.numberStoredChannels * header4.samplingRate);

	switch (reductionList.size())
	{
	case 0:
		currentEndTimeValue = actualValueEnd + valueAddEnd;
		break;
	case 1:
		currentEndTimeValue = (reductionLists[reductionList.size() - 1].time - reductionLists[reductionList.size() - 1].sample) / header4.samplingRate;
		currentEndTimeValue += actualValueEnd + valueAddEnd;
		break;
	default:
		currentEndTimeValue = (reductionLists[reductionList.size() - 2].time - reductionLists[reductionList.size() - 2].sample) / header4.samplingRate;
		currentEndTimeValue += actualValueEnd + valueAddEnd;
		break;
	}

	header4.endTime.hour = unsigned char(currentEndTimeValue / 3600);
	header4.endTime.min = unsigned char((currentEndTimeValue % 3600) / 60);
	header4.endTime.sec = (currentEndTimeValue % 3600) % 60;
}

void MicromedLibrary::TRCFile::getBeginImpedance(ifstream &fileStream, descriptorArea *descriptorBeginImp, vector<impedanceFile> &startingImpedance)
{
	impedanceFile impedance;
	for (int i = 0; i < int(descriptorBeginImp->length / 2); i++)
	{
		impedance.positive = binaryCharExtraction(descriptorBeginImp->startOffset + (2 * i), fileStream);
		impedance.negative = binaryCharExtraction(descriptorBeginImp->startOffset + 1 + (2 * i), fileStream);
		if (impedance.positive != 255)
			startingImpedance.push_back(impedance);
	}
}

void MicromedLibrary::TRCFile::getEndImpedance(ifstream &fileStream, descriptorArea *descriptorEndImp, vector<impedanceFile> &endingImpedance)
{
	impedanceFile impedance;
	for (int i = 0; i < int(descriptorEndImp->length / 2); i++)
	{
		impedance.positive = binaryCharExtraction(descriptorEndImp->startOffset + (2 * i), fileStream);
		impedance.negative = binaryCharExtraction(descriptorEndImp->startOffset + 1 + (2 * i), fileStream);
		if (impedance.positive != 255)
			endingImpedance.push_back(impedance);
	}
}

void MicromedLibrary::TRCFile::getMontages(ifstream &fileStream, descriptorArea *descriptorMontages, vector<montagesOfTrace> &montageList)
{
	montagesOfTrace montage;

	for (int i = 0; i < MAX_MONT; i++)
	{
		montage.lines = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + (i * 4096), 2, fileStream);
		montage.sectors = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + 2 + (i * 4096), 2, fileStream);
		montage.baseTime = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + 4 + (i * 4096), 2, fileStream);
		montage.notch = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + 6 + (i * 4096), 2, fileStream);

		fileStream.seekg(descriptorMontages->startOffset + 8 + (i * 4096));
		fileStream.read((char *)&montage.colour, sizeof(unsigned char[128]));
		fileStream.seekg(descriptorMontages->startOffset + 136 + (i * 4096));
		fileStream.read((char *)&montage.selection, sizeof(unsigned char[128]));
		fileStream.read((char *)&montage.description, sizeof(char[64]));
		for (int j = 0; j < MAX_CAN_VIEW; j++)
		{
			montage.inputs[j].nonInverting = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + 328 + (4 * j) + (i * 4096), 2, fileStream);
			montage.inputs[j].inverting = (unsigned short)binaryBytesExtraction(descriptorMontages->startOffset + 330 + (4 * j) + (i * 4096), 2, fileStream);
		}
		fileStream.seekg(descriptorMontages->startOffset + 840 + (i * 4096));
		fileStream.read((char *)&montage.highPassFilter, sizeof(unsigned long[128]));
		fileStream.seekg(descriptorMontages->startOffset + 1352 + (i * 4096));
		fileStream.read((char *)&montage.lowPassFilter, sizeof(unsigned long[128]));
		fileStream.seekg(descriptorMontages->startOffset + 1864 + (i * 4096));
		fileStream.read((char *)&montage.reference, sizeof(unsigned long[128]));

		for (int j = 0; j < 1720; j++)
			montage.free[j] = binaryCharExtraction(descriptorMontages->startOffset + 2376 + j + (i * 4096), fileStream);

		montageList.push_back(montage);
	}
}

void MicromedLibrary::TRCFile::getCompressionInfo(ifstream &fileStream, descriptorArea *descriptorCompression, compressionDescription &compression)
{
	fileStream.seekg(descriptorCompression->startOffset);
	fileStream.read((char *)&compression.compressionStuff, sizeof(char[10]));
}

void MicromedLibrary::TRCFile::getAverageInfo(ifstream &fileStream, descriptorArea *descriptorAverage, offlineAverageProcess &averageParameters)
{
	averageParameters.meanTrace = binaryBytesExtraction(descriptorAverage->startOffset, 4, fileStream);
	averageParameters.meanFile = binaryBytesExtraction(descriptorAverage->startOffset + 4, 4, fileStream);
	averageParameters.meanPrestim = binaryBytesExtraction(descriptorAverage->startOffset + 8, 4, fileStream);
	averageParameters.meanPoststim = binaryBytesExtraction(descriptorAverage->startOffset + 12, 4, fileStream);
	averageParameters.meanType = binaryBytesExtraction(descriptorAverage->startOffset + 16, 4, fileStream);
	fileStream.seekg(descriptorAverage->startOffset + 20);
	fileStream.read((char *)&averageParameters.freeFurtherUse, sizeof(unsigned char[108]));
}

void MicromedLibrary::TRCFile::getHistoryInfo(ifstream &fileStream, descriptorArea *descriptorHistory, vector<unsigned long int> & historySam, vector<montagesOfTrace> &montagesHistoryList)
{
	for (int i = 0; i < MAX_SAMPLE; i++)
	{
		unsigned long int value = binaryBytesExtraction(descriptorHistory->startOffset + (4 * i), 4, fileStream);
		if (value < 4294967295)
			historySam.push_back(value);
	}

	int beginValue = descriptorHistory->startOffset + (4 * MAX_SAMPLE);

	for (int i = 0; i < MAX_HISTORY; i++)
	{
		montagesOfTrace montageHist;

		montageHist.lines = (unsigned short)binaryBytesExtraction(beginValue + (i * 4096), 2, fileStream);
		montageHist.sectors = (unsigned short)binaryBytesExtraction(beginValue + 2 + (i * 4096), 2, fileStream);
		montageHist.baseTime = (unsigned short)binaryBytesExtraction(beginValue + 4 + (i * 4096), 2, fileStream);
		montageHist.notch = (unsigned short)binaryBytesExtraction(beginValue + 6 + (i * 4096), 2, fileStream);

		fileStream.seekg(beginValue + 8 + (i * 4096));
		fileStream.read((char *)&montageHist.colour, sizeof(unsigned char[128]));
		fileStream.seekg(beginValue + 136 + (i * 4096));
		fileStream.read((char *)&montageHist.selection, sizeof(unsigned char[128]));
		fileStream.read((char *)&montageHist.description, sizeof(char[64]));
		for (int j = 0; j < MAX_CAN_VIEW; j++)
		{
			montageHist.inputs[j].nonInverting = (unsigned short)binaryBytesExtraction(beginValue + 328 + (4 * j) + (i * 4096), 2, fileStream);
			montageHist.inputs[j].inverting = (unsigned short)binaryBytesExtraction(beginValue + 330 + (4 * j) + (i * 4096), 2, fileStream);
		}
		fileStream.seekg(beginValue + 840 + (i * 4096));
		fileStream.read((char *)&montageHist.highPassFilter, sizeof(unsigned long[128]));
		fileStream.seekg(beginValue + 1352 + (i * 4096));
		fileStream.read((char *)&montageHist.lowPassFilter, sizeof(unsigned long[128]));
		fileStream.seekg(beginValue + 1864 + (i * 4096));
		fileStream.read((char *)&montageHist.reference, sizeof(unsigned long[128]));
		fileStream.seekg(beginValue + 2376 + (i * 4096));
		fileStream.read((char *)&montageHist.free, sizeof(unsigned char[1720]));

		montagesHistoryList.push_back(montageHist);
	}
}

void MicromedLibrary::TRCFile::getDVideoInfo(ifstream &fileStream, descriptorArea *descriptorDvideo, vector<dVideoFiles> &dvidTRC)
{
	dVideoFiles vid;
	for (int i = 0; i < MAX_FILE; i++)
	{
		vid.DV_Begin = binaryBytesExtraction(descriptorDvideo->startOffset + i, 4, fileStream);
		dvidTRC.push_back(vid);
	}
}

void MicromedLibrary::TRCFile::getEventA(ifstream &fileStream, descriptorArea *descriptorEventA, eventsMarker &eventA)
{
	MarkerPair markSelec;

	fileStream.seekg(descriptorEventA->startOffset);
	fileStream.read((char *)&eventA.description, sizeof(char[64]));
	for (int i = 0; i < MAX_EVENT; i++)
	{
		markSelec.begin = binaryBytesExtraction(descriptorEventA->startOffset + 64 + (i * 8), 4, fileStream);
		markSelec.end = binaryBytesExtraction(descriptorEventA->startOffset + 68 + (i * 8), 4, fileStream);
		eventA.selection.push_back(markSelec);
	}
}

void MicromedLibrary::TRCFile::getEventB(ifstream &fileStream, descriptorArea *descriptorEventB, eventsMarker &eventB)
{
	MarkerPair markSelec;

	fileStream.seekg(descriptorEventB->startOffset);
	fileStream.read((char *)&eventB.description, sizeof(char[64]));
	for (int i = 0; i < MAX_EVENT; i++)
	{
		markSelec.begin = binaryBytesExtraction(descriptorEventB->startOffset + 64 + (i * 8), 4, fileStream);
		markSelec.end = binaryBytesExtraction(descriptorEventB->startOffset + 68 + (i * 8), 4, fileStream);
		eventB.selection.push_back(markSelec);
	}
}

void MicromedLibrary::TRCFile::getDigitalTriggers(ifstream &fileStream, descriptorArea *descriptorDigiTrig, vector<digitalTriggers> &digiTriggers)
{
	digitalTriggers trigger;

	for (int i = 0; i < int(descriptorDigiTrig->length / 6); i++)
	{
		trigger.triggerSample = binaryBytesExtraction(descriptorDigiTrig->startOffset + (i * 6), 4, fileStream);
		if (trigger.triggerSample < 4294967295)
		{
			trigger.triggerValue = (unsigned short)binaryBytesExtraction(descriptorDigiTrig->startOffset + 4 + (i * 6), 2, fileStream);
			digiTriggers.push_back(trigger);
		}
	}
}