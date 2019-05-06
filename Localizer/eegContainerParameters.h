#ifndef _EEGCONTAINERPARAMETERS_H
#define _EEGCONTAINERPARAMETERS_H

#include <iostream>
#include <vector>
#include "Utility.h"
#include "ITrigger.h"

using namespace std;

namespace InsermLibrary
{

	struct elecContainer
	{
		string label;
		vector<int> id;
		vector<int> idOrigFile;
	};
}

#endif