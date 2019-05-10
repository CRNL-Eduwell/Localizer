#ifndef FREQUENCYBANDANALYSISOPT_H
#define FREQUENCYBANDANALYSISOPT_H

#include <iostream>
#include <vector>

#include "FrequencyBand.h"

namespace InsermLibrary
{
	struct FrequencyBandAnalysisOpt
	{
		bool eeg2env2;
		bool env2plot;
		bool trialmat;
		FrequencyBand Band;
	};
}

#endif