#ifndef _EEGCONTAINERPARAMETERS_H
#define _EEGCONTAINERPARAMETERS_H

#include <iostream>
#include <vector>
#include "Utility.h"
#include "ITrigger.h"

namespace InsermLibrary
{
	struct elecContainer
	{
		std::string label;
		std::vector<int> id;
		std::vector<int> idOrigFile;
	};
}

#endif