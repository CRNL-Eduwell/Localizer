#ifndef HILBERTENVELOPPE_H
#define HILBERTENVELOPPE_H

#include <iostream>
#include "IAlgorithmStrategy.h"
#include "eegContainer.h"
#include "Convolution.h"

class HilbertEnveloppe : public IAlgorithmStrategy
{
public:
	HilbertEnveloppe() { };
	~HilbertEnveloppe() { };
	void Process(eegContainer* Container, int IndexFrequencyData, vector<int> FrequencyBand);

private:
	void CalculateSmoothingCoefficients(int SamplingFrequency, int DownsamplingFactor);
	void HilbertDownSampSumData(DataContainer* DataContainer, int threadId, int freqId);
	void MeanConvolveData(DataContainer *DataContainer, int threadId);

private:
	float m_smoothingSample[6];
	float m_smoothingMilliSec[6] = { 0, 250, 500, 1000, 2500, 5000 };
	std::mutex m_mtx;
};

#endif