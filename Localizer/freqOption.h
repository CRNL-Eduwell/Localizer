#ifndef FREQOPTION_H
#define FREQOPTION_H

#include <iostream>
#include <vector>
#include <fstream>	

#include "Utility.h"

namespace InsermLibrary
{
	struct frequency
	{
		std::string freqName;
		std::string freqFolderName;
		std::vector<int> freqBandValue;
	};

	struct freqOption
	{
		freqOption(std::string pathFreqFile = "./Resources/Config/frequencyBand.txt");
		~freqOption();
		std::vector<frequency> frequencyBands;
	};
}

#endif