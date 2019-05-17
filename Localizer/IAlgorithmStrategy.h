#ifndef IALGORITHMSTRATEGY_H
#define IALGORITHMSTRATEGY_H

#include <iostream>
#include "eegContainer.h"

enum Algorithm { Hilbert };
class IAlgorithmStrategy
{
public:
	virtual void Process(eegContainer* EegContainer, int IndexFrequencyData, vector<int>& FrequencyBand) { };
};

#endif