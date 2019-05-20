#include "AlgorithmCalculator.h"

Algorithm::AlgorithmStrategyFactory Algorithm::AlgorithmCalculator::m_factory;

void Algorithm::AlgorithmCalculator::ExecuteAlgorithm(Algorithm::AlgorithmType algo, eegContainer* EegContainer, int IndexFrequencyData, const vector<int>& FrequencyBand)
{
	IAlgorithmStrategy* AlgorithmStrategy = m_factory.GetAlgorithmStrategy(algo);
	if (AlgorithmStrategy != nullptr)
	{
		AlgorithmStrategy->Process(EegContainer, IndexFrequencyData, FrequencyBand);
	}
}

