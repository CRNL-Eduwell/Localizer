#ifndef _EEGCONTAINER_H
#define _EEGCONTAINER_H

#include "TRCFunctions.h"
#include "ELANFunctions.h"
#include "EDFFunctions.h"
#include "eegContainerParameters.h"
#include "MATLABFUNC.h"	

#include <iostream>
#include <vector>
#include <thread>
#include <mutex> 

#include <direct.h>

using namespace std;
using namespace MicromedLibrary;
using namespace InsermLibrary;

namespace InsermLibrary
{
	struct samplingInformation
	{
		int samplingFrequency;
		int downsampledFrequency;
		int downsampFactor;
		int nbSample;//Original size of one channel (no downsamp)
	};

	struct dataContainer
	{
		dataContainer(vector<int> frequencyBand, samplingInformation samplingInfo);
		~dataContainer();

		vector<vector<float>> bipData;
		vector<vector<float>> hilData;

		vector<FFTINFO*> fftFront;
		vector<FFTINFO*> fftBack;
		vector<vector<float>>downData;
		vector<vector<float>>meanData;
		vector<vector<vector<float>>> convoData;
		vector<FIRINFO*> firData;

		samplingInformation sampInfo;
		int arrayLength;
		int arrayDownLength;
		int frequencyLength;
	};

	class eegContainer
	{
	public:
		eegContainer(ELANFile* elan, int downsampFrequency, int nbFreqBand);
		eegContainer(TRCFile* trc, int downsampFrequency, int nbFreqBand);
		eegContainer(EDFFile* edf, int downsampFrequency, int nbFreqBand);
		~eegContainer();
		void deleteElectrodes(vector<int> elecToDelete);
		void bipolarizeData();
		void getElectrodes();
		void ToHilbert(elan_struct_t* elanStruct, vector<int> frequencyBand);
	private:
		void getElectrodeFromElanFile(ELANFile* elan);
		void getElectrodeFromTRCFile(TRCFile* trc);
		void getElectrodeFromEDFFile(EDFFile* edf);
		int idSplitDigiAndNum(string myString);
		void calculateSmoothing();
		vector<int> findIndexes(vector<digitalTriggers> tab, int value2find);
		vector<int> findIndexes(vector<eventElanFile> trigg, int value2find);
		vector<int> findIndexes(vector<Edf_event> trigg, int value2find);
		void initElanFreqStruct(elan_struct_t *structToInit);
		void hilbertDownSampSumData(dataContainer *dataCont, int threadId, int freqId);
		void meanConvolveData(dataContainer *dataCont, int threadId);

	public :
		vector<elan_struct_t*> elanFrequencyBand;
		TRIGGINFO *triggEeg = nullptr;
		TRIGGINFO *triggEegDownsampled = nullptr;
		vector<elecContainer> electrodes;
		vector<string> flatElectrodes;
		vector<bipole> bipoles;
		vector<vector<float>> eegData;
		vector<int> idElecToDelete;
		string originalFilePath = "";
		samplingInformation sampInfo;
	private:
		float smoothingSample[6];
		float smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
		ELANFile* elanFile = nullptr;
		TRCFile* trcFile = nullptr;
		EDFFile* edfFile = nullptr;
		std::mutex mtx;
	};
}
#endif