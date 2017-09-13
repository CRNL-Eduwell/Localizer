#ifndef _ELANFUNCTIONS_H
#define _ELANFUNCTIONS_H

#include "ELANFile.h"
#include "eegContainerParameters.h"
#include <fstream>
#include <numeric>      // std::accumulate
#include <iomanip>

using namespace std;
using namespace MicromedLibrary;
using namespace InsermLibrary;

namespace InsermLibrary
{
	class ELANFunctions
	{
	public:
		//=== Convert from other file format
		static ELANFile *micromedToElan(TRCFile *trc);

		//=== Data Modification
		static void deleteOneOrSeveralElectrodesAndData(ELANFile *elan, vector<int> indexToDelete);
		static void convertMicromedAnalogDataToDigital(ELANFile *elan, TRCFile *trc);
		static void convertELANAnalogDataToDigital(ELANFile *elan);
		static void convertELANAnalogDataToDigital(elan_struct_t *elan);
		static void convertELANDigitalToAnalog(ELANFile *elan);

		//=== Data Extract
		static void readPosFile(ELANFile *elan, string pathPosFile);
		static void readNotesFile(ELANFile *elan, string pathNoteFile);
		static void readFile(ELANFile *elan, string filePath);
		static void readHeader(ELANFile *elan, string filePath);
		static void readDataAllChannels(ELANFile *elan, string filePath);
		static void readBlocDataAllChannels(elan_struct_t *elan, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2]);
		static void readBlocDataEventsAllChannels(elan_struct_t *elan, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2]);

		//=== Data Write
		static void writePosFile(ELANFile *elan, string pathPosFile);
		static void writeNotesFile(ELANFile *elan, string pathNoteFile);
		static void writeFile(ELANFile *elan, string filePath);
		static void writeHeader(ELANFile *elan, string filePath);
		static void writeDataAllChannels(ELANFile *elan, string filePath);
		static void writeOldElanHeader(elan_struct_t *elan, string filePath);
		static void writeOldElanData(elan_struct_t *elan, string filePath, int idMeasure);

		//=== Debug Tools
		static void exportEEGDataCSV(string csvFilePath, ELANFile *elan);
		static void exportTEEGDataCSVOneChanel(string csvFilePath, ELANFile *elan, int index);

	private:
		static void initOrigStructElanStruct(elan_struct_t *elanStruct);
		static void createNewElanStructForCopy(ELANFile *elan, vector<int> indexToDelete);
		static void writeOldElanHeader(ELANFile *elan, string filePath);
		static void writeOldElanData(ELANFile *elan, string filePath);
	};
}
#endif