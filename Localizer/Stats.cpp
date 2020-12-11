#include "Stats.h"

using namespace std;
using namespace InsermLibrary;

void InsermLibrary::Stats::pValuesWilcoxon(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer, int samplingFreq, PROV *myprovFile)
{
	std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();
	int ConditionCount = SubGroupStimTrials.size();
	for (int i = 0; i < bigdata.size(); i++)
	{
		vec2<float> p_valueBig;
		vec2<int> p_signeBig;
		for (int j = 0; j < ConditionCount - 1; j++)
		{
			int lowTrial = SubGroupStimTrials[j];
			int highTrial = SubGroupStimTrials[j + 1];
			int numberSubTrial = highTrial - lowTrial;

			vec1<float> baseLine = getBaselineBlocWilcoxon(i, lowTrial, numberSubTrial, samplingFreq, myprovFile->visuBlocs[j].dispBloc, bigdata);
			vec2<float> eegDataBig = getEegDataBlocWilcoxon(i, lowTrial, numberSubTrial, samplingFreq, j, myprovFile, bigdata);

			vec1<float> p_value;
			for (int l = 0; l < eegDataBig.size(); l++)
			{
				p_value.push_back(Framework::Calculations::Stats::wilcoxon(baseLine, eegDataBig[l]));
			}
			p_valueBig.push_back(p_value);

			p_signeBig.push_back(getEegSignBlocWilcoxon(baseLine, eegDataBig));
		}
		pSign3D.push_back(p_signeBig);
		pValue3D.push_back(p_valueBig);
	}
}

void InsermLibrary::Stats::pValuesKruskall(vec3<float> &pValue3D, vec3<int> &pSign3D, vec3<float> &bigdata, TriggerContainer* triggerContainer, int samplingFreq, PROV *myprovFile)
{
	int *windowSam = myprovFile->getBiggestWindowSam(samplingFreq);
	for (int i = 0; i < bigdata[0].size(); i++)
	{
		vec1<float> baseLineData = getBaselineKruskall(bigdata, triggerContainer, i, windowSam);
		vec2<float> eegDataBig = getEEGDataKruskall(bigdata, triggerContainer, i, windowSam);
		pValue3D.push_back(getPValuesKruskall(baseLineData, eegDataBig));
		pSign3D.push_back(getEegSignKruskall(baseLineData, eegDataBig));
	}
	delete windowSam;
}

vec1<PVALUECOORD> InsermLibrary::Stats::FDR(vec3<float> &pValues3D, vec3<int> &pSign3D, int &copyIndex, float pLimit)
{
	int V = pValues3D.size() * pValues3D[0].size() * pValues3D[0][0].size();
	float CV = log(V) + 0.5772;
	float slope = pLimit / (V * CV);

	vec1<PVALUECOORD> preFDRValues = loadPValues(pValues3D, pSign3D);

	std::sort(preFDRValues.begin(), preFDRValues.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	for (int i = 0; i < V; i++)
	{
		if (preFDRValues[i].pValue >((double)slope * (i + 1)))
		{
			copyIndex = i;
			break;
		}
	}

	vector<PVALUECOORD> significantValue;
	for (int i = 0; i < copyIndex; i++)
	{
		significantValue.push_back(preFDRValues[i]);
	}

	//vérifier si celui la est nécessaire ??
	std::sort(significantValue.begin(), significantValue.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	return significantValue;
}

vec1<PVALUECOORD> InsermLibrary::Stats::loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D)
{
	int compteur = 0;
	PVALUECOORD tempPValue;
	vector<PVALUECOORD> pValues;

	for (int i = 0; i < pValues3D.size(); i++)
	{
		for (int j = 0; j < pValues3D[i].size(); j++)
		{
			for (int k = 0; k < pValues3D[i][j].size(); k++)
			{
				tempPValue.elec = i;
				tempPValue.condit = j;
				tempPValue.window = k;
				tempPValue.vectorpos = compteur;
				tempPValue.pValue = pValues3D[i][j][k];
				tempPValue.weight = pSign3D[i][j][k];
				pValues.push_back(tempPValue);
				compteur++;
			}
		}
	}

	return pValues;
}

vec1<PVALUECOORD> InsermLibrary::Stats::loadPValues(vec3<float> &pValues3D, vec3<int> &pSign3D, float pLimit)
{
	int compteur = 0;
	PVALUECOORD tempPValue;
	vector<PVALUECOORD> pValues;

	for (int i = 0; i < pValues3D.size(); i++)
	{
		for (int j = 0; j < pValues3D[i].size(); j++)
		{
			for (int k = 0; k < pValues3D[i][j].size(); k++)
			{
				if (pValues3D[i][j][k] < pLimit)
				{
					tempPValue.elec = i;
					tempPValue.condit = j;
					tempPValue.window = k;
					tempPValue.vectorpos = compteur;
					tempPValue.pValue = pValues3D[i][j][k];
					tempPValue.weight = pSign3D[i][j][k];

					pValues.push_back(tempPValue);
				}
				compteur++;
			}
		}
	}

	return pValues;
}

void InsermLibrary::Stats::exportStatsData(eegContainer *myEegContainer, PROV *myprovFile, vec1<PVALUECOORD> pValues, 
										   string outputFolder, bool isBar)
{
	ofstream fichierSt(outputFolder + "/statLoca.csv", ios::out);
	fichierSt << " " << ";";
	for (int i = 0; i < myEegContainer->BipoleCount(); i++)
	{
		fichierSt << myEegContainer->flatElectrodes[myEegContainer->Bipole(i).first] << ";";
	}
	fichierSt << endl;

	for (int j = 0; j < myprovFile->visuBlocs.size(); j++)
	{
		fichierSt << myprovFile->visuBlocs[j].mainEventBloc.eventLabel << ";";
		for (int i = 0; i < myEegContainer->BipoleCount(); i++)
		{
			vector<int> indexes;
			for (int z = 0; z < pValues.size(); z++)
			{
				if (isBar == false)
				{
					if (pValues[z].elec == i && pValues[z].condit == j)
						indexes.push_back(z);
				}
				else
				{
					if (pValues[z].elec == i && pValues[z].window == j)
						indexes.push_back(z);
				}
			}

			if (indexes.size() > 0)
			{
				int val = 0;
				for (int z = 0; z < indexes.size(); z++)
				{
					val += pValues[indexes[z]].weight;
				}
				fichierSt << val << ";";
			}
			else
			{
				fichierSt << "FALSE" << ";";
			}
		}
		fichierSt << endl;	
	}
	fichierSt.close();
}

vec1<float> InsermLibrary::Stats::getBaselineBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, 
														  int samplingFreq, displayBLOC dispBloc, vec3<float> &bigdata)
{
	vec1<float> baseLine;
	for (int k = 0; k < numberSubTrial; k++)
	{
		float sum = 0.0;
		int begBaseline = round((samplingFreq * (dispBloc.baseLineMin() -
								 dispBloc.windowMin())) / 1000);
		int endBaseline = round((samplingFreq * (dispBloc.baseLineMax() -
								 dispBloc.windowMin())) / 1000);

		for (int l = 0; l < (endBaseline - begBaseline); l++)
		{
			sum += bigdata[currentChanel][lowTrial + k][begBaseline + l];
		}
		baseLine.push_back(sum / (endBaseline - begBaseline));
	}
	return baseLine;
}

vec2<float> InsermLibrary::Stats::getEegDataBlocWilcoxon(int currentChanel, int lowTrial, int numberSubTrial, 
														 int samplingFreq, int idBloc, PROV *myprovFile, 
														 vec3<float> &bigdata)
{
	vec2<float> eegDataBig;

	int numberWindows = 0;
	if (myprovFile->invertmapsinfo != "")
	{	//100 because 200ms window with overlap 50%
		numberWindows = (myprovFile->visuBlocs[idBloc].dispBloc.windowMax() -
			myprovFile->visuBlocs[idBloc].dispBloc.windowMin()) / 100;
	}
	else
	{
		numberWindows = myprovFile->visuBlocs[idBloc].dispBloc.windowMax() / 100;
	}

	int *windowMs = myprovFile->getBiggestWindowMs();
	for (int k = 0; k < numberWindows - 1; k++)
	{
		vec1<float> eegData;
		for (int l = 0; l < numberSubTrial; l++)
		{
			float sum = 0.0;
			int begWindow = round((samplingFreq * (0 + (100 * k) + (myprovFile->visuBlocs[idBloc].dispBloc.windowMin() - windowMs[0]))) / 1000);
			int endWindow = round((samplingFreq * (200 + (100 * k) + (myprovFile->visuBlocs[idBloc].dispBloc.windowMin() - windowMs[0]))) / 1000);

			for (int m = 0; m < (endWindow - begWindow); m++)
			{
				sum += bigdata[currentChanel][lowTrial + l][begWindow + m];
			}
			eegData.push_back(sum / (endWindow - begWindow));
		}
		eegDataBig.push_back(eegData);
	}
	delete windowMs;
	return eegDataBig;
}

vec1<int> InsermLibrary::Stats::getEegSignBlocWilcoxon(vec1<float> &baseLine, vec2<float> &eegDataBig)
{
	vec1<int> valueSigne;
	for (int i = 0; i < eegDataBig.size(); i++)
	{
		double diff = 0;
		double meanBaseLine = 0, meanWindow = 0;
		for (int m = 0; m < eegDataBig[i].size(); m++)
		{
			meanBaseLine += baseLine[m];
			meanWindow += eegDataBig[i][m];
		}
		meanBaseLine /= eegDataBig[i].size();
		meanWindow /= eegDataBig[i].size();

		diff = meanWindow - meanBaseLine;
		if (diff < 0)
		{
			valueSigne.push_back(-1);
		}
		else if (diff == 0)
		{
			valueSigne.push_back(0);
		}
		else if (diff > 0)
		{
			valueSigne.push_back(1);
		}
	}
	return valueSigne;
}

vec1<float> InsermLibrary::Stats::getBaselineKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam)
{
	int SubGroupCount = triggerContainer->SubGroupStimTrials().size();
	std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();

	int lowTrial = SubGroupStimTrials[SubGroupCount - 2];
	int highTrial = SubGroupStimTrials[SubGroupCount - 1];
	int numberSubTrial = highTrial - lowTrial;

	vec1<float> baseLineData;
	for (int i= 0; i < numberSubTrial; i++)
	{
		int begBaseline = 0;
		int endBaseLine = windowSam[1] - windowSam[0];
		float sum = 0.0;
		for (int j = 0; j < (endBaseLine - begBaseline); j++)
		{
			sum += bigdata[lowTrial + i][currentChanel][begBaseline + j];
		}
		baseLineData.push_back(sum / (endBaseLine - begBaseline));
	}

	return baseLineData;
}

vec2<float> InsermLibrary::Stats::getEEGDataKruskall(vec3<float> &bigdata, TriggerContainer* triggerContainer, int currentChanel, int* windowSam)
{
	int SubGroupCount = triggerContainer->SubGroupStimTrials().size();
	std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();

	vec2<float> eegDataBig;
	for (int j = 0; j < SubGroupCount - 2; j++)
	{
		int lowTrial = SubGroupStimTrials[j];
		int highTrial = SubGroupStimTrials[j + 1];
		int numberSubTrial = highTrial - lowTrial;

		vec1<float> eegData;
		for (int k = 0; k < numberSubTrial; k++)
		{
			int begWindow = 0;
			int endWindow = windowSam[1] - windowSam[0];

			float sum = 0.0;
			for (int l = 0; l < (endWindow - begWindow); l++)
			{
				sum += bigdata[lowTrial + k][currentChanel][begWindow + l];
			}
			eegData.push_back(sum / (endWindow - begWindow));
		}
		eegDataBig.push_back(eegData);
	}
	return eegDataBig;
}

vec2<float> InsermLibrary::Stats::getPValuesKruskall(vec1<float> &baseLineData, vec2<float> &eegData)
{
	vec1<float> pValue;
	vec2<float> pValueBig;

	float *dataArray[2];
	int nbSamplePerGroup[2];
	double p = 0, H = 0;
	dataArray[0] = &baseLineData[0];
	for (int j = 0; j < eegData.size(); j++)
	{
		dataArray[1] = &eegData[j][0];
		nbSamplePerGroup[0] = eegData[j].size();
		nbSamplePerGroup[1] = eegData[j].size();

		Framework::Calculations::Stats::kruskal_wallis(dataArray, 2, nbSamplePerGroup, &H, &p, 1);
		pValue.push_back(p);
	}
	pValueBig.push_back(pValue);

	return pValueBig;
}

vec2<int> InsermLibrary::Stats::getEegSignKruskall(vec1<float> &baseLineData, vec2<float> &eegData)
{
	vec2<int> valueSignBig;
	vec1<int> valueSign;
	for (int j = 0; j < eegData.size(); j++)
	{
		double diff = 0;
		double meanBaseLine = 0, meanWindow = 0;
		for (int k = 0; k < eegData[j].size(); k++)
		{
			meanBaseLine += baseLineData[k];
			meanWindow += eegData[j][k];
		}
		meanBaseLine /= eegData[j].size();
		meanWindow /= eegData[j].size();

		diff = meanWindow - meanBaseLine;
		if (diff < 0)
		{
			valueSign.push_back(-1);
		}
		else if (diff == 0)
		{
			valueSign.push_back(0);
		}
		else if (diff > 0)
		{
			valueSign.push_back(1);
		}
	}
	valueSignBig.push_back(valueSign);
	return valueSignBig;
}
