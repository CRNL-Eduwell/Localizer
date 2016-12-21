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
	void deleteAndNullify1D(T* pointer)
	{
		if (pointer != nullptr)
		{
			delete pointer;
			pointer = nullptr;
		}
	}

	template<typename T>
	void deleteAndNullify2D(T** pointer, int sizeFirstDim)
	{
		if (pointer != nullptr)
		{
			for (int i = 0; i < sizeFirstDim; i++)
			{
				delete pointer;
				pointer = nullptr;
			}
		}
	}

	template<typename T>
	inline T* allocate1DArray(int sizeFirstDim)
	{
		return new T[sizeFirstDim]();
	}

	template<typename T>
	inline void deAllocate1DArray(T* myArray)
	{
		delete[] myArray;
		myArray = nullptr;
	}

	template<typename T>
	T** allocate2DArray(int sizeFirstDim, int sizeSecondDim)
	{
		T** myArray = new T*[sizeFirstDim];
		for (int i = 0; i < sizeFirstDim; i++)
		{
			myArray[i] = new T[sizeSecondDim]();
		}
		return myArray;
	}

	template<typename T>
	void deAllocate2DArray(T** myArray, int sizeFirstDim)
	{
		for (int i = 0; i < sizeFirstDim; i++)
		{
			delete[] myArray[i];
		}
		delete[] myArray;
		myArray = nullptr;
	}

	template<typename T>
	T*** allocate3DArray(int sizeFirstDim, int sizeSecondDim, int sizeThirdDim)
	{
		T*** myArray = new T**[sizeFirstDim];
		for (int i = 0; i < sizeFirstDim; i++)
		{
			myArray[i] = new T*[sizeSecondDim];
			for (int j = 0; j < sizeSecondDim; j++)
			{
				myArray[i][j] = new T[sizeThirdDim]();
			}
		}
		return myArray;
	}

	template<typename T>
	void deAllocate3DArray(T** myArray, int sizeFirstDim, int sizeSecondDim)
	{
		for (int i = 0; i < sizeFirstDim; i++)
		{
			for (int j = 0; j < sizeSecondDim; j++)
			{
				delete[] myArray[i][j];
			}
			delete[] myArray[i];
		}
		delete[] myArray;
		myArray = nullptr;
	}

	template<typename T>
	T stdDeviation(T* myArray, int sizeFirstDim)
	{
		T stdTempValue = 0;
		
		T meanValue = mean1DArray(myArray, int sizeFirstDim);
		for (int i = 0; i < sizeFirstDim; i++)
		{
			stdTempValue += (myArray[i] - meanValue) * (myArray[i] - meanValue);
		}

		return sqrt(stdTempValue / (sizeFirstDim - 1));
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


	vector<int> findIndexes(int *tab, int sizetab, int value2find);
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