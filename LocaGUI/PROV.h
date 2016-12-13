#ifndef _PROV_H
#define _PROV_H

#include <iostream>	
#include <vector>
#include <sstream>
#include <fstream>
#include "Utility.h"
#include "PROVParameters.h"

using namespace std;

namespace InsermLibrary
{
	class PROV
	{
	public:
		PROV(string provFilePath);
		~PROV();
		int nbCol();
		int nbRow();
	private:
		void extractProvBloc(string provFilePath);
		vector<string> asciiDataProv(string provFilePath);
	public:
		vector<BLOC> visuBlocs;
		invertMap invertmaps;
		string filePath = "";
		string changeCodeFilePath = "";
		string invertmapsinfo = "";
	};
}

#endif