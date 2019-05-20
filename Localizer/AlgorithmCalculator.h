#ifndef ALGORITHM_CALCULATOR_H
#define ALGORITHM_CALCULATOR_H

#include <iostream>
#include <vector>
#include "AlgorithmStrategyFactory.h"
#include "eegContainer.h"

namespace Algorithm
{
	class AlgorithmCalculator
	{
	public:
		static void ExecuteAlgorithm(Algorithm::AlgorithmType algo, eegContainer* EegContainer, int IndexFrequencyData, const std::vector<int>& FrequencyBand);

	private:
		static AlgorithmStrategyFactory m_factory;
	};
}
#endif