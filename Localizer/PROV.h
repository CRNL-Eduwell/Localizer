#ifndef _PROV_H
#define _PROV_H

#include <iostream>	
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include "Utility.h"
#include "PROVParameters.h"

namespace InsermLibrary
{
	class PROV
	{
	public:
		PROV();
        PROV(std::string provFilePath);
		~PROV();
		//==
		int nbCol();
		int nbRow();
        std::vector<int> getMainCodes();
        std::vector<std::vector<int>> getSecondaryCodes();
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
        void extractProvBloc(std::string provFilePath);
        std::vector<std::string> asciiDataProv(std::string provFilePath);
		void getRightOrderBloc();
	public:
        std::vector<int> rightOrderBlocs;
        std::vector<BLOC> visuBlocs;
		invertMap invertmaps;
        std::string changeCodeFilePath = "";
        std::string invertmapsinfo = "";

	private:
        std::string m_filePath = "";
	};
}

#endif
