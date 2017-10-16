#ifndef _EDFFUNCTIONS_H
#define _EDFFUNCTIONS_H

#include <iostream>
#include "EDFFile.h"

namespace InsermLibrary
{
	class EDFFunctions
	{
	public:
		//=== Convert from other file format
		static void deleteOneOrSeveralElectrodesAndData(EDFFile *myEDFFile, vector<int> indexToDelete);
	};
}

#endif