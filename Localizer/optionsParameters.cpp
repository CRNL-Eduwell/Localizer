#include "optionsParameters.h"

//void InsermLibrary::uiUserElement::analysis(vector<locaAnalysisOption> &analysisToRun, int nbLoca)
//{
//	if (analysisToRun.size() > 0)
//		analysisToRun.clear();
//
//	vector<analysisOption> dd;
//	for (int i = 0; i < eeg2envCheckBox.size(); i++)
//	{
//		analysisOption analysisOneFrequency;
//		if (eeg2envCheckBox[i]->isChecked())
//			analysisOneFrequency.eeg2env = true;
//		else
//			analysisOneFrequency.eeg2env = false;
//
//		if (env2plotCheckBox[i]->isChecked())
//			analysisOneFrequency.env2plot = true;
//		else
//			analysisOneFrequency.env2plot = false;
//
//		if (trialmapCheckBox[i]->isChecked())
//			analysisOneFrequency.trialmat = true;
//		else
//			analysisOneFrequency.trialmat = false;
//
//		dd.push_back(analysisOneFrequency);
//	}
//
//	for (int i = 0; i < nbLoca; i++)
//	{
//		locaAnalysisOption newLoca;
//		newLoca.anaOpt = vector<analysisOption>(dd);
//		analysisToRun.push_back(newLoca);
//	}
//}
