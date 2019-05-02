#ifndef _STATS_H
#define _STATS_H

#include <iostream>
#include <fstream>		

#include "PROV.h"
#include "TriggerContainer.h"
#include "eegContainer.h"

#include "Wilcox.h"
#include "Kruskall.h"
#include "Utility.h"
#include "optionsParameters.h"

using namespace std;
using namespace InsermLibrary;

namespace InsermLibrary
{
	class Stats
	{
	public:
		static void pValuesWilcoxon(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer,
									int samplingFreq, PROV *myprovFile);
		static void pValuesKruskall(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer,
									int samplingFreq, PROV *myprovFile);
		static vec1<PVALUECOORD> FDR(vec3<float> &pValues3D, vec3<int> &pSign3D, int &copyIndex, float pLimit);
		static vec1<PVALUECOORD> loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D);
		static vec1<PVALUECOORD> loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D, float pLimit);
		static void exportStatsData(eegContainer *myEegContainer, PROV *myprovFile, vec1<PVALUECOORD> pValues, 
									string outputFolder, bool isBar);
	private:
		static vec1<float> getBaselineBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int samplingFreq,
												   displayBLOC dispBloc, vec3<float> &bigdata);
		static vec2<float> getEegDataBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, int samplingFreq,
												  int idBloc, PROV *myprovFile, vec3<float> &bigdata);
		static vec1<int> getEegSignBlocWilcoxon(vec1<float> &baseLine, vec2<float> &eegDataBig);
		//==
		static vec1<float> getBaselineKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam);
		static vec2<float> getEEGDataKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam);
		static vec2<float> getPValuesKruskall(vec1<float> &baseLineData, vec2<float> &eegData);
		static vec2<int> getEegSignKruskall(vec1<float> &baseLineData, vec2<float> &eegData);
	};
}

#endif