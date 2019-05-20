#include "AlgorithmStrategyFactory.h"

Algorithm::AlgorithmStrategyFactory::AlgorithmStrategyFactory()
{
	m_hilbert = new HilbertEnveloppe();
};

Algorithm::AlgorithmStrategyFactory::~AlgorithmStrategyFactory()
{
	EEGFormat::Utility::DeleteAndNullify(m_hilbert);
};

IAlgorithmStrategy* Algorithm::AlgorithmStrategyFactory::GetAlgorithmStrategy(AlgorithmType algo)
{
	switch (algo)
	{
	case AlgorithmType::Hilbert:
		return m_hilbert;
		break;
	default:
		return nullptr;
		break;
	}
}