#ifndef _STATS_H
#define _STATS_H

#include <iostream>
#include <fstream>		
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\ELAN\ELAN.h"	
//#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\PROV\PROV.h"
#include"PROV.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\wilcox.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\kruskall.h"
#include "Utility.h"

using namespace std;
using namespace InsermLibrary;

namespace InsermLibrary
{
	class Stats
	{
	public :
		static vector<vector<vector<double>>> pValuesWilcoxon(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***bigdata);
		static vector<vector<vector<int>>> signWilcoxon(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***bigdata);

		static vector<vector<vector<double>>> pValuesKruskall(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***eegData);
		static vector<vector<vector<int>>> signKruskall(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***bigdata);

		static vector<PVALUECOORD> FDR(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D, int &copyIndex, float pLimit);
		static vector<PVALUECOORD> loadPValues(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D);
		static vector<PVALUECOORD> loadPValues(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D, float pLimit);
		static void exportStatsData(ELAN *p_elan, PROV *p_prov, vector<PVALUECOORD> pValues, string outputFolder, bool isBar);
		static void exportPChanels(string outputFolder, vector<vector<vector<double>>> pValues3D);
		static void exportFDRChanels(string outputFolder, vector<PVALUECOORD> pValuesFDR);
	};
}

#endif