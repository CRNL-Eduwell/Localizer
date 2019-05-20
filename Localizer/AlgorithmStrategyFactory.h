#ifndef ALGORITHM_STRATEGY_FACTORY_H
#define ALGORITHM_STRATEGY_FACTORY_H

#include <iostream>
#include "IAlgorithmStrategy.h"
#include "HilbertEnveloppe.h"

namespace Algorithm
{
	enum AlgorithmType { Hilbert };

	class AlgorithmStrategyFactory
	{
	public:
		AlgorithmStrategyFactory();
		~AlgorithmStrategyFactory();
		IAlgorithmStrategy* GetAlgorithmStrategy(AlgorithmType algo);

	private:
		HilbertEnveloppe* m_hilbert = nullptr;
	};
}
#endif
