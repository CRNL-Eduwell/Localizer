#include "AlgorithmContext.h"

AlgorithmContext::AlgorithmContext()
{
	m_hilbert = new HilbertEnveloppe();
}

AlgorithmContext::~AlgorithmContext()
{
	EEGFormat::Utility::DeleteAndNullify(m_hilbert);
}


void AlgorithmContext::SetAlgorithm(Algorithm algo)
{
	m_algorithm = algo;
}

void AlgorithmContext::ExecuteAlgorithm(eegContainer* EegContainer, int IndexFrequencyData, vector<int>& FrequencyBand)
{
	switch (m_algorithm)
	{
	case Algorithm::Hilbert:
		m_hilbert->Process(EegContainer, IndexFrequencyData, FrequencyBand);
		break;
	default:
		break;
	}
}