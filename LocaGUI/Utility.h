#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

namespace InsermLibrary
{
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
	void deleteAndNullify(T pointer)
	{
		if (pointer != nullptr)
		{
			delete pointer;
			pointer = nullptr;
		}
	}
	//===
	
	class LOCAANALYSISOPTION
	{
	public:
		LOCAANALYSISOPTION(vector<vector<double>> p_frequencys, vector<vector<bool>> p_analysisDetails, string p_trcPath, string p_provPath, string p_patientFolder, string p_task, string p_expTask);
		~LOCAANALYSISOPTION();

		vector<vector<double>> frequencys;
		vector<vector<bool>> analysisDetails;
		string trcPath = "";
		string provPath = "";
		string patientFolder = "";
		string task = "";
		string expTask = "";
	};

	struct PVALUECOORD
	{
		int elec = -69;
		int condit = -69;
		int window = -69;
		int vectorpos = -69;
		double pValue = -69;
	};

	class TRIGG
	{
	public:
		TRIGG(int p_valueTrigger, int p_sampleTrigger, int p_rtMs, int p_rtCode, int p_origPos);
		~TRIGG();

		int valueTrigger;
		int sampleTrigger;
		int rt_ms;
		int rt_code;
		int origPos;
	};

	class TRIGGINFO
	{
	public:
		TRIGGINFO(unsigned long *p_valueTrigg, unsigned long *p_sampleTrigg, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int *p_rtCode, int *p_origPos, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int p_numberTrigg, int p_downFactor);
		~TRIGGINFO();

		vector<TRIGG> trigg;
		vector<int> mainGroupSub;
		int numberTrigg;
		int downFactor;
	};
}

#endif