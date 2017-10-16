#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <vector>
#include <fstream>	
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

#include <QGroupBox>
#include <QCheckBox>
#include <QFrame>

using namespace std;

namespace InsermLibrary
{
	enum FileExt { NO_EXT = -1, TRC, EEG_ELAN, ENT_ELAN, POS_ELAN, POS_DS_ELAN, SM0_ELAN, SM250_ELAN, SM500_ELAN, SM1000_ELAN, SM2500_ELAN, SM5000_ELAN, EDF };

	/********************************************************************/
	/*	vector<string> v = split<string>("Hello, there; World", ";,");	*/
	/********************************************************************/
	template<typename T>
	vector<T> split(const T & str, const T & delimiters)
	{
		vector<T> v;
		typename T::size_type start = 0;
		auto pos = str.find_first_of(delimiters, start);
		while (pos != T::npos) {
			if (pos != start) // ignore empty tokens
				v.emplace_back(str, start, pos - start);
			start = pos + 1;
			pos = str.find_first_of(delimiters, start);
		}
		if (start < str.length()) // ignore trailing delimiter
			v.emplace_back(str, start, str.length() - start); // add what's left of the string
		return v;
	}

	template<typename T>
	void deleteAndNullify1D(T& pointer)
	{
		if (pointer != nullptr)
		{
			delete pointer;
			pointer = nullptr;
		}
	}

	template<typename T>
	T stdDeviation(T* myArray, int sizeFirstDim, int mean)
	{
		T stdTempValue = 0;

		for (int i = 0; i < sizeFirstDim; i++)
		{
			stdTempValue += (myArray[i] - mean) * (myArray[i] - mean);
		}

		return sqrt(stdTempValue / (sizeFirstDim - 1));
	}

	template<typename T>
	T mean1DArray(T* myArray, int sizeFirstDim)
	{
		T sumValues = 0;
		for (int i = 0; i < sizeFirstDim; i++)
		{
			sumValues += myArray[i];
		}
		return (sumValues / sizeFirstDim);
	}

	template<typename T>
	T mean2DArray(T** myArray, int sizeFirstDim, int posSecondDim)
	{
		T sumValues = 0;
		for (int i = 0; i < sizeFirstDim; i++)
		{
			sumValues += myArray[i][posSecondDim];
		}
		return (sumValues / sizeFirstDim);
	}

	template<class T>
	using vec1 = vector<T>; /**< templated std vector alias */

	template<class T>
	using vec2 = vector<vec1<T>>; /**< templated std vector of std vector alias */

	template<class T>
	using vec3 = vector<vec2<T>>; /**< templated std vector of std vector of std vector alias */

	vector<string> readTxtFile(string path);

	void saveTxtFile(vector<QString> data, string pathFile);

	void deblankString(std::string &myString);
	
	string GetCurrentWorkingDir();

	//=== Option structs passed in worker and localizer
	
	struct frequency
	{
		string freqName;
		string freqFolderName;
		vector<int> freqBandValue;
	};

	struct freqOption
	{
		freqOption(string pathFreqFile = "./Resources/Config/frequencyBand.txt");
		~freqOption();
		vector<frequency> frequencyBands;
	};

	struct analysisOption
	{
		bool eeg2env;
		bool env2plot;
		bool trialmat;
	};

	struct locaAnalysisOption
	{
		bool localizer;
		vector<analysisOption> anaOpt;
	};

	struct statOption
	{
		bool wilcoxon = true;
		bool FDRwilcoxon = true;
		float pWilcoxon = 0.01;
		vector<QString> locaWilcoxon;
		bool kruskall = true;
		bool FDRkruskall = true;
		float pKruskall = 0.01;
		vector<QString> locaKruskall;
	};

	struct picOption
	{
		QSize sizeTrialmap;
		QSize interpolationtrialmap;
		QSize sizePlotmap;
	};

	struct userOption
	{
		vector<QString> locaPerf;
		freqOption freqOption;
		vector<locaAnalysisOption> anaOption;
		statOption statOption;
		picOption picOption;
	};

	//== UI stuff in localizer.cpp

	struct uiUserElement
	{
		void analysis(vector<locaAnalysisOption> &analysisToRun, int nbLoca);

		vector<QCheckBox *> eeg2envCheckBox;
		vector<QFrame *>  sm0Frame;
		vector<QFrame *>  sm250Frame;
		vector<QFrame *>  sm500Frame;
		vector<QFrame *>  sm1000Frame;
		vector<QFrame *>  sm2500Frame;
		vector<QFrame *>  sm5000Frame;
		vector<QFrame *>  dsPOSFrame;
		//===
		vector<QGroupBox *> trialmapGroupBox;
		vector<QCheckBox *> trialmapCheckBox;
		vector<QFrame *>  trialmapFrame;
		//===
		vector<QGroupBox *> env2plotGroupBox;
		vector<QCheckBox *> env2plotCheckBox;
		vector<QFrame *>  env2plotFrame;
	};

	//==

	struct PVALUECOORD
	{
		int elec = -69;
		int condit = -69;
		int window = -69;
		int vectorpos = -69;
		float pValue = -69;
		int weight = 0;
	};
}

#endif