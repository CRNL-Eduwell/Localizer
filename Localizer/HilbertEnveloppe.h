#ifndef HILBERTENVELOPPE_H
#define HILBERTENVELOPPE_H

#include <iostream>
#include "IAlgorithmStrategy.h"
#include "eegContainer.h"
#include "Convolution.h"

namespace Algorithm
{
	namespace Strategy
	{
		class HilbertEnveloppe : public IAlgorithmStrategy
		{
		public:
			HilbertEnveloppe() { };
			~HilbertEnveloppe() { };
			virtual void Process(eegContainer* Container, vector<int> FrequencyBand);

		private:
			void InitOutputDataStructure(eegContainer* EegContainer);
			void CalculateSmoothingCoefficients(int SamplingFrequency, int DownsamplingFactor);
			void HilbertDownSampSumData(DataContainer* DataContainer, int threadId, int freqId);
			void MeanConvolveData(DataContainer *DataContainer, int threadId);

		private:
			float m_smoothingSample[6];
			float m_smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
			std::mutex m_mtx;
		};
	}
}

#endif