#ifndef IALGORITHMSTRATEGY_H
#define IALGORITHMSTRATEGY_H

#include <iostream>
#include "eegContainer.h"

class IAlgorithmStrategy
{
public:
	virtual void Process(eegContainer* EegContainer, int IndexFrequencyData, vector<int> FrequencyBand) = 0;
};

#endif