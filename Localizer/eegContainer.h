#ifndef _EEGCONTAINER_H
#define _EEGCONTAINER_H

#include "IFile.h"
#include "ElanFile.h"
#include "eegContainerParameters.h"
#include "FirBandPass.h"	
#include "Convolution.h"
#include "Trigger.h"

#include <iostream>
#include <vector>
#include <thread>
#include <mutex> 

#include <direct.h>

using namespace std;
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

		vector<vector<Framework::Filtering::Linear::FirBandPass*>> Filters;
		vector<vector<float>>downData;
		vector<vector<float>>meanData;
		vector<vector<vector<float>>> convoData;

		samplingInformation sampInfo;
		int arrayLength;
		int arrayDownLength;
		int frequencyLength;
	};

	class eegContainer
	{
	public:
		eegContainer(EEGFormat::IFile* file, int downsampFrequency, int nbFreqBand);
		~eegContainer();		

		//===[ Getter / Setter ]===
		inline std::string RootFileFolder()
		{
			return EEGFormat::Utility::GetDirectoryPath(m_file->DefaultFilePath());
		}
		inline std::string RootFileName(bool withExtension = false)
		{
			return EEGFormat::Utility::GetFileName(m_file->DefaultFilePath(), withExtension);
		}
		inline int TriggerCount()
		{
			return m_file->TriggerCount();
		}
		inline std::vector<EEGFormat::ITrigger> Triggers()
		{
			int TriggerCount = m_file->TriggerCount();
			std::vector<EEGFormat::ITrigger> triggers(TriggerCount);
			for (int i = 0; i < TriggerCount; i++)
			{
				triggers.push_back(EEGFormat::ITrigger(*m_file->Trigger(i)));
			}
			return triggers;
		}
		inline int BipoleCount()
		{
			return m_bipoles.size();
		}
		//First is positiv Electrode ID
		//Second is negativ Electrode ID
		inline std::pair<int,int> Bipole(const int& i)
		{
			if (i >= 0 && i < m_bipoles.size())
			{
				return m_bipoles[i];
			}
			return std::pair<int, int>(-1, -1);
		}
		inline std::string BipolePositivLabel(const int& i)
		{
			if (i >= 0 && i < m_bipoles.size())
			{
				return flatElectrodes[m_bipoles[i].first];
			}
			return "";
		}
		inline std::string BipoleNegativLabel(const int& i)
		{
			if (i >= 0 && i < m_bipoles.size())
			{
				return flatElectrodes[m_bipoles[i].second];
			}
			return "";
		}
		inline std::vector<std::vector<float>>& Data() { return m_file->Data(EEGFormat::DataConverterType::Digital); }
		inline const std::vector<std::vector<float>>& Data() const { return m_file->Data(EEGFormat::DataConverterType::Digital); }

		//===[ Data Modification ]===
		void DeleteElectrodes(vector<int> elecToDelete);
		void GetElectrodes();
		void BipolarizeElectrodes();
		void ToHilbert(int IdFrequency, vector<int> frequencyBand);
		void LoadFrequencyData(std::vector<std::string>& filesPath, int frequencyId, int smoothingId);

		//===[ Read / Get Data ]===
		void GetFrequencyBlocData(vec3<float>& outputEegData, int frequencyId, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2]);
		void GetFrequencyBlocDataEvents(vec3<float>& outputEegData, int frequencyId, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2]);

	private:
		void GetElectrodes(EEGFormat::IFile* edf);
		int idSplitDigiAndNum(string myString);
		void calculateSmoothing();
		void initElanFreqStruct();
		void hilbertDownSampSumData(dataContainer *dataCont, int threadId, int freqId);
		void meanConvolveData(dataContainer *dataCont, int threadId);

	public :
		//[IdNbFrequency][sm0-sm5000][channels][sample]
		std::vector<std::vector<EEGFormat::ElanFile*>> elanFrequencyBand;
		//TRIGGINFO *triggEeg = nullptr;
		//TRIGGINFO *triggEegDownsampled = nullptr;
		vector<elecContainer> electrodes;
		vector<string> flatElectrodes;
		vector<int> idElecToDelete;
		samplingInformation sampInfo;
	private:
		std::vector<std::pair<int, int>> m_bipoles;
		float smoothingSample[6];
		float smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
		EEGFormat::IFile* m_file = nullptr;
		std::mutex mtx;
	};
}
#endif