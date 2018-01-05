#ifndef _TRCFILE_H
#define _TRCFILE_H

#include <fstream>
#include <ostream>
#include <vector>
#include <algorithm>
#include "Utility.h"
#include "TRCParameters.h"

using namespace std;
using namespace InsermLibrary;

namespace MicromedLibrary
{
	class TRCFile
	{
		friend class TRCFunctions;

	public:
		//Constructor
		TRCFile();
		TRCFile(string pathTRCFile);
		~TRCFile();

		// getters
		string filePath();
		headerType4 & header();									//return ref
		unsigned long DataSize();
		void setDataSize(unsigned long size);
		vector<electrode> & electrodes();						//return ref
		vector<operatorNote> & notes();							//return ref
		vector<digitalTriggers> & triggers();					//return ref
		int nbSample();
		vector<float> & eegDataOneChanel();						//return ref
		vector<vector<float>>& eegDataAllChanels();				//return ref

	private:
		//Read Binary Data TRC
		unsigned char binaryCharExtraction(int positionInFile, ifstream &sr);
		unsigned long binaryBytesExtraction(int positionInFile, int numberOfBytes, ifstream &sr);
		string binaryStringExtraction(int positionInFile, int numberOfChar, ifstream &sr);

		//Header Reading Functions
		void getHeaderInformations(ifstream &sr);
		int getFileSize(ifstream &sr);

		//Descriptors Position in Header
		void getDescriptorInfo(ifstream &fileStream, descriptorArea *descriptor, int positionFile);

		//Descriptor Functions
		void getOrderStorageElec(ifstream &fileStream, descriptorArea *descriptorCode, vector<unsigned short int> &orderStorageElec);
		void getUsedElectrodes(ifstream &fileStream, descriptorArea *descriptorElec, vector<electrode> &electrodesList);
		void sortElectrodes(vector<electrode> &electrodesList);
		void getNotes(ifstream &fileStream, descriptorArea *descriptorNote, vector<operatorNote> &notesList);
		void getFlags(ifstream &fileStream, descriptorArea *descriptorFlags, vector<selectionFlag> &flagsList);
		void getReductionInfo(ifstream &fileStream, descriptorArea *descriptorReduction, vector<reducedFileDescription> &reductionList);
		void truncatedFileBeginTime(vector<reducedFileDescription> reductionList);
		void truncatedFileEndTime(vector<reducedFileDescription> reductionList);
		void getBeginImpedance(ifstream &fileStream, descriptorArea *descriptorBeginImp, vector<impedanceFile> &startingImpedance);
		void getEndImpedance(ifstream &fileStream, descriptorArea *descriptorEndImp, vector<impedanceFile> &endingImpedance);
		void getMontages(ifstream &fileStream, descriptorArea *descriptorMontages, vector<montagesOfTrace> &montageList);
		void getCompressionInfo(ifstream &fileStream, descriptorArea *descriptorCompression, compressionDescription &compression);
		void getAverageInfo(ifstream &fileStream, descriptorArea *descriptorAverage, offlineAverageProcess &averageParameters);
		void getHistoryInfo(ifstream &fileStream, descriptorArea *descriptorHistory, vector<unsigned long int> & historySam, vector<montagesOfTrace> &montagesHistoryList);
		void getDVideoInfo(ifstream &fileStream, descriptorArea *descriptorDvideo, vector<dVideoFiles> &dvidTRC);
		void getEventA(ifstream &fileStream, descriptorArea *descriptorEventA, eventsMarker &eventA);
		void getEventB(ifstream &fileStream, descriptorArea *descriptorEventB, eventsMarker &eventB);
		void getDigitalTriggers(ifstream &fileStream, descriptorArea *descriptorDigiTrig, vector<digitalTriggers> &digiTriggers);

	private:
		//=== Header Info
		headerType4 header4;
		vector<unsigned short int> orderStorageElec;
		vector<electrode> electrodesList;
		vector<operatorNote> notesList;
		vector<selectionFlag> flagsList;
		vector<reducedFileDescription> reductionLists;
		vector<impedanceFile> startingImpedance;
		vector<impedanceFile> endingImpedance;
		vector<montagesOfTrace> montagesList;
		compressionDescription compression;
		offlineAverageProcess averageParameters;
		std::vector<unsigned long int> historySample; // [MAX_SAMPLE];
		vector<montagesOfTrace> montagesHistoryList;
		vector<dVideoFiles> digitalVideos;
		eventsMarker eventA;
		eventsMarker eventB;
		vector<digitalTriggers> triggersList;
		//=== EEG Data
		vector<float> eegOneChanel;
		vector<vector<float>> eegAllChanels;
		//=== Useful Info not in TRC File
		int fileLength;
		unsigned long int dataSize;
		string myFilePath;
	};
}
#endif