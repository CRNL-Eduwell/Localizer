#include "Utility.h"

vector<int> InsermLibrary::findIndexes(int *tab, int sizetab, int value2find)	
{
	vector<int> indexesFound;	
	for (int i = 0; i < sizetab; i++)
	{
		if (tab[i] == value2find)
		{	
			indexesFound.push_back(i);	
		}	
	}	
			
	return indexesFound;	
}

InsermLibrary::LOCAANALYSISOPTION::LOCAANALYSISOPTION(vector<vector<double>> p_frequencys, vector<vector<bool>> p_analysisDetails, string p_trcPath, string p_provPath, string p_patientFolder, string p_task, string p_expTask)
{
	frequencys = p_frequencys;
	analysisDetails = p_analysisDetails;
	trcPath = p_trcPath;
	provPath = p_provPath;
	patientFolder = p_patientFolder;
	task = p_task;
	expTask = p_expTask;
}

InsermLibrary::LOCAANALYSISOPTION::~LOCAANALYSISOPTION()
{

}

InsermLibrary::TRIGG::TRIGG(int p_valueTrigger, int p_sampleTrigger, int p_rtMs, int p_rtCode, int p_origPos)
{
	valueTrigger = p_valueTrigger;
	sampleTrigger = p_sampleTrigger;
	rt_ms = p_rtMs;
	rt_code = p_rtCode;
	origPos = p_origPos;
}

InsermLibrary::TRIGG::~TRIGG()
{

}

InsermLibrary::TRIGGINFO::TRIGGINFO(unsigned long *p_valueTrigg, unsigned long *p_sampleTrigg, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	for (int i = 0; i < numberTrigg; i++)
	{
		TRIGG t(p_valueTrigg[i], p_sampleTrigg[i] / downFactor, 1000000, -1, -1); //10000000 et -1 default value
		trigg.push_back(t);
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	for (int i = 0; i < numberTrigg; i++)
	{
		TRIGG t(p_valueTrigg[i], p_sampleTrigg[i], p_rtMs[i], -1, -1); // -1 default value
		trigg.push_back(t);
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int *p_rtCode, int *p_origPos, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	for (int i = 0; i < numberTrigg; i++)
	{
		TRIGG t(p_valueTrigg[i], p_sampleTrigg[i], p_rtMs[i], p_rtCode[i], p_origPos[i]); // -1 default value
		trigg.push_back(t);
	}
}

InsermLibrary::TRIGGINFO::TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int p_numberTrigg, int p_downFactor)
{
	numberTrigg = p_numberTrigg;
	downFactor = p_downFactor;

	for (int i = 0; i < numberTrigg; i++)
	{
		TRIGG t(p_valueTrigg[i], p_sampleTrigg[i] / downFactor, 10000000, -1, -1); //10000000 et -1 default value
		trigg.push_back(t);
	}
}

InsermLibrary::TRIGGINFO::~TRIGGINFO()
{

}