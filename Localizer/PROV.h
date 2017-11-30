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
		PROV();
		PROV(string provFilePath);
		~PROV();
		//==
		int nbCol();
		int nbRow();
		vector<int> getMainCodes();
		vector<vector<int>> getSecondaryCodes();
		int *getBiggestWindowMs();
		int *getBiggestWindowSam(int samplingFreq);
		int *getWindowMs(int idBloc);
		int *getWindowSam(int samplingFreq, int idBloc);
		//==
		inline std::string filePath() const { return m_filePath; };
		inline void filePath(std::string path) { m_filePath = path; };
		void row(int idBloc, std::string rowValue);
		void column(int idBloc, std::string columnValue);
		void name(int idBloc, std::string nameValue);
		void path(int idBloc, std::string pathValue);
		void sort(int idBloc, std::string sortValue);
		//==
		void saveFile();
		void saveFile(std::string rootFolder, std::string fileName);
	private:
		void extractProvBloc(string provFilePath);
		vector<string> asciiDataProv(string provFilePath);
		void getRightOrderBloc();
	public:
		vector<int> rightOrderBlocs;
		vector<BLOC> visuBlocs;
		invertMap invertmaps;
		string changeCodeFilePath = "";
		string invertmapsinfo = "";

	private:
		string m_filePath = "";
	};
}

#endif