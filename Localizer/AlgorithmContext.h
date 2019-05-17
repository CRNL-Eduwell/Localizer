#ifndef ALGORITHMCONTEXT_H
#define ALGORITHMCONTEXT_H

#include <iostream>
#include <vector>
#include "eegContainer.h"
#include "IAlgorithmStrategy.h"
#include "HilbertEnveloppe.h"

class AlgorithmContext
{
public:
	AlgorithmContext();
	~AlgorithmContext();
	void SetAlgorithm(Algorithm algo);
	void ExecuteAlgorithm(eegContainer* EegContainer, int IndexFrequencyData, std::vector<int>& FrequencyBand);

private:
	HilbertEnveloppe* m_hilbert = nullptr;
	Algorithm m_algorithm;
};

#endif