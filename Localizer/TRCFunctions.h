#ifndef _TRCFUNCTIONS_H
#define _TRCFUNCTIONS_H

#include "TRCFile.h"

namespace MicromedLibrary
{
	class TRCFunctions
	{
	public:
		static string signalMeasurementUnitText(int value);

		//===[ Data Modif ]===
		static void deleteOneOrSeveralElectrodesAndData(TRCFile *myTRCFile, vector<int> indexToDelete);
		static void convertAnalogDataToDigital(TRCFile *myTRCFile);

		//===[ Data Read ]===
		static void readTRCDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, int posInVector);
		static void readTRCDataMultipleChanel(TRCFile *myTRCFile);
		static void readTRCDataAllChanels(TRCFile *myTRCFile);
		//vector<double> readTRCBlockDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, int sampStart, int nbSampleToRead):
		//vector<vector<double>> readTRCBlockDataAllChanels(TRCFile *myTRCFile, int sampStart, int nbSampleToRead):

		//===[ Data Write ]===
		static void writeTRCFileFromOriginal(ofstream &outputFileStream, TRCFile *myTRCFile);
		static void writeTRCFileFromData(std::string outputPath, TRCFile *myTRCFile);
		static void concatenateTRCFile(TRCFile *myTRCFile, TRCFile *myOtherTRCFile, string outputFilePath);
		static void stapleTRCFile(TRCFile *myTRCFile, TRCFile *myOtherTRCFile, string outputFilePath);

		//===[ Debug / Tools ]===
		static void exportTRCDataCSV(string csvFilePath, TRCFile *myTRCFile);
		static void exportTRCDataCSVOneChanel(string csvFilePath, TRCFile *myTRCFile, int index);

	private:
		//===[ Data Read ]===
		static void readBinaryDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, char *binaryEEGData);
		static void binaryToDigitalDataOneChanel(TRCFile *myTRCFile, int posInVector, char *binaryEEGData);
		static void binaryToDigitalDataMultipleChanels(TRCFile *myTRCFile, char *binaryEEGData);
		static void readBinaryDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData);
		static void binaryToDigitalDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData);

		//===[ Data Write ]===
		//= write from orig
		static void copyBinaryHeader(std::ofstream &writeStream, std::ifstream &readStream, int adressFirstData);
		static void modifyBinaryHeaderValue(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile);
		static void readAndWriteBinaryDataMultiChanels(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile);
		//= write from data
		static void writeHeader(ofstream &outputFileStream, headerType4 *header);
		static void writeDescriptor(ofstream &outputFileStream, descriptorArea *descriptor, int positionFile);
		static void writeOrderStorageElectrodes(ofstream &outputFileStream, std::vector<unsigned short int> & orderStorageElec, int positionFile);
		static void writeElectrodes(ofstream &outputFileStream, TRCFile *myTRCFile);
		static void writeNotes(ofstream &outputFileStream, vector<operatorNote> &notesList, int positionFile);
		static void writeFlags(ofstream &outputFileStream, vector<selectionFlag> &flagList, int positionFile);
		static void writeReductionInfo(ofstream &outputFileStream, vector<reducedFileDescription> & reductionLists, int positionFile);
		static void writeBeginImpedance(ofstream &outputFileStream, vector<impedanceFile> &beginImpedances, int positionFile);
		static void writeEndImpedance(ofstream &outputFileStream, vector<impedanceFile> &endImpedances, int positionFile);
		static void writeMontages(ofstream &outputFileStream, vector<montagesOfTrace> &montageList, int positionFile);
		static void writeCompression(ofstream &outputFileStream, compressionDescription *montageList, int positionFile);
		static void writeAverage(ofstream &outputFileStream, offlineAverageProcess *averageParameters, int positionFile);
		static void writeDVideo(ofstream &outputFileStream, vector<dVideoFiles> &dvidTRC, int positionFile);
		static void writeEventA(ofstream &outputFileStream, eventsMarker &eventA, int positionFile);
		static void writeEventB(ofstream &outputFileStream, eventsMarker &eventB, int positionFile);
		static void writeHistoryInfo(ofstream &outputFileStream, vector<unsigned long int> & historySam, vector<montagesOfTrace> &montagesHistoryList, int positionFile);
		static void writeDigitalTriggers(ofstream &outputFileStream, std::vector<digitalTriggers> & digiTriggers, int positionFile);
		static void writeDataChanels(ofstream &outputFileStream, TRCFile *myTRCFile);
		//== concatenate
		static void concatNotes(TRCFile *myOtherTRCFile, TRCFile *concatFile);
		static void concatFlags(TRCFile *myOtherTRCFile, TRCFile *concatFile);
		static void concatEventA(TRCFile *myOtherTRCFile, TRCFile *concatFile);
		static void concatEventB(TRCFile *myOtherTRCFile, TRCFile *concatFile);
		static void concatTriggers(TRCFile *myOtherTRCFile, TRCFile *concatFile);
		static void concatDataChanels(TRCFile *myOtherTRCFile, TRCFile *concatFile);
	};
}
#endif