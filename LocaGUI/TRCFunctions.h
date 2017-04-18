#ifndef _TRCFUNCTIONS_H
#define _TRCFUNCTIONS_H

#include "TRCFile.h"

namespace MicromedLibrary
{
	class TRCFunctions
	{
	public:
		static string signalMeasurementUnitText(int value);
		//Data Modification
		static void deleteOneOrSeveralElectrodesAndData(TRCFile *myTRCFile, vector<int> indexToDelete);
		static void convertAnalogDataToDigital(TRCFile *myTRCFile);

		//Data Extract
		static void readTRCDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, int posInVector);
		static void readTRCDataMultipleChanel(TRCFile *myTRCFile);
		static void readTRCDataAllChanels(TRCFile *myTRCFile);
		//vector<double> readTRCBlockDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, int sampStart, int nbSampleToRead):
		//vector<vector<double>> readTRCBlockDataAllChanels(TRCFile *myTRCFile, int sampStart, int nbSampleToRead):

		//Data Write
		static void writeTRCFileFromOriginal(ofstream &outputFileStream, TRCFile *myTRCFile);
		//static void writeTRCFileFromData(ofstream &outputFileStream, TRCFile *myTRCFile);
		//void concatenateTRCFile(TRCFile *myTRCFile, TRCFile *myOtherTRCFile, string outputFilePath);

		//Debug/Tools
		static void exportTRCDataCSV(string csvFilePath, TRCFile *myTRCFile);
		static void exportTRCDataCSVOneChanel(string csvFilePath, TRCFile *myTRCFile, int index);

	private:
		//Data Extract
		static void readBinaryDataOneChanel(TRCFile *myTRCFile, int indexChanelToExtract, char *binaryEEGData);
		static void binaryToDigitalDataOneChanel(TRCFile *myTRCFile, int posInVector, char *binaryEEGData);
		static void binaryToDigitalDataMultipleChanels(TRCFile *myTRCFile, char *binaryEEGData);
		static void readBinaryDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData);
		static void binaryToDigitalDataAllChanels(TRCFile *myTRCFile, char *binaryEEGData);

		//Data Write
		static void copyBinaryHeader(std::ofstream &writeStream, std::ifstream &readStream, int adressFirstData);
		static void modifyBinaryHeaderValue(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile);
		static void readAndWriteBinaryDataMultiChanels(ofstream &outputFileStream, ifstream &readStream, TRCFile *myTRCFile);
	};
}
#endif