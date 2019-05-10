#ifndef OPTIONSPARAMETERS_H
#define OPTIONSPARAMETERS_H

#include <iostream>
#include <vector>

#include <QGroupBox>
#include <QCheckBox>
#include <QFrame>

#include "freqOption.h"

namespace InsermLibrary
{
	struct analysisOption
	{
		bool eeg2env;
		bool env2plot;
		bool trialmat;
	};

	struct locaAnalysisOption
	{
		bool localizer;
		std::vector<analysisOption> anaOpt;
	};

	struct statOption
	{
		bool wilcoxon = true;
		bool FDRwilcoxon = true;
		float pWilcoxon = 0.01;
		std::vector<QString> locaWilcoxon;
		bool kruskall = true;
		bool FDRkruskall = true;
		float pKruskall = 0.01;
		std::vector<QString> locaKruskall;
	};

	struct picOption
	{
		QSize sizeTrialmap;
		QSize interpolationtrialmap;
		QSize sizePlotmap;
	};

	struct userOption
	{
		std::vector<QString> locaPerf;
		freqOption freqOption;
		std::vector<locaAnalysisOption> anaOption;
		statOption statOption;
		picOption picOption;
	};

	struct PVALUECOORD
	{
		int elec = -69;
		int condit = -69;
		int window = -69;
		int vectorpos = -69;
		float pValue = -69;
		int weight = 0;
	};

	//== UI stuff in localizer.cpp

	//struct uiUserElement
	//{
	//	void analysis(vector<locaAnalysisOption> &analysisToRun, int nbLoca);

	//	vector<QCheckBox *> eeg2envCheckBox;
	//	vector<QFrame *>  sm0Frame;
	//	vector<QFrame *>  sm250Frame;
	//	vector<QFrame *>  sm500Frame;
	//	vector<QFrame *>  sm1000Frame;
	//	vector<QFrame *>  sm2500Frame;
	//	vector<QFrame *>  sm5000Frame;
	//	vector<QFrame *>  dsPOSFrame;
	//	//===
	//	vector<QGroupBox *> trialmapGroupBox;
	//	vector<QCheckBox *> trialmapCheckBox;
	//	vector<QFrame *>  trialmapFrame;
	//	//===
	//	vector<QGroupBox *> env2plotGroupBox;
	//	vector<QCheckBox *> env2plotCheckBox;
	//	vector<QFrame *>  env2plotFrame;
	//};

	//==
}
#endif