#ifndef CRPERF_H
#define CRPERF_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <QDir>
#include <QString>
#include "Utility.h"

using namespace std;
using namespace InsermLibrary;

namespace InsermLibrary		
{
	class CRData 
	{
		public : 
			CRData(int nbConditions);
			~CRData();

			double *conditionPerCent = nullptr;
			int numberOfTrigger = -1;
			double totalGoodAnswerPercent = -1;
			int numberOfConditions = -1;
	};

	class CRperf
	{
		public :
			static void LEC1CR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers);
			static void MCSECR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers);
			static void MVISMVEBCR(LOCAANALYSISOPTION *p_anaopt, TRIGGINFO *triggers);

			static vector<string> getAllFolderNames(string rootPath);
			static CRData* getCRInfo(string filePath, string rootName, int numberConditions);
			static int whichOneAmI(string compareMe);
			static int whereAmI(string compareMe);
			static void createCSVPerformanceReport(string rootFolder, CRData** files);
	};
}

#endif //CRPERF_H