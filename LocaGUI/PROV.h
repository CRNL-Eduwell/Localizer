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
		vector<int> getMainCodes();
		vector<int> getSecondaryCodes();
		int *getBiggestWindowMs();
		int *getBiggestWindowSam(int samplingFreq);
		int *getWindowMs(int idBloc);
		int *getWindowSam(int samplingFreq, int idBloc);
	private:
		void extractProvBloc(string provFilePath);
		vector<string> asciiDataProv(string provFilePath);
		void getRightOrderBloc();
	public:
		vector<int> rightOrderBlocs;
		vector<BLOC> visuBlocs;
		invertMap invertmaps;
		string filePath = "";
		string changeCodeFilePath = "";
		string invertmapsinfo = "";
	};
}

#endif