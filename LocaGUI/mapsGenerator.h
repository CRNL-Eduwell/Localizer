#ifndef _MAPSGENERATOR_H
#define _MAPSGENERATOR_H
#include <iostream>
#include "Utility.h"
#include "PROV.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>

using namespace std;
using namespace InsermLibrary;

namespace InsermLibrary
{
	namespace DrawCard
	{
		class mapsGenerator
		{
		public:
			mapsGenerator(int width, int heigth);
			~mapsGenerator();
			//=== TrialMat Drawing Functions
			void trialmatTemplate(vector<int> trialsPerRow, PROV *myprovFile);
			void graduateColorBar(QPainter *painter, int maxValue);
			void drawVerticalZeroLine(QPainter *painter, PROV* myprovFile);
			void displayStatsOnMap(QPainter *painter, vec2<int> idCurrentMap, vec1<PVALUECOORD> significantValue, PROV* myprovFile);
			void drawMapTitle(QPainter *painter, string title);

			//=== TrialMat Data2Color Functions
			vec2<float> horizontalInterpolation(vec2<float> chanelToInterpol, int interpolationFactor, int idBegTrigg, int nbSubTrials);
			vec2<float> verticalInterpolation(vec2<float> chanelToInterpol, int interpolationFactor);
			void eegData2ColorMap(vec1<int> colorX[512], vec1<int> colorY[512], vec2<float> interpolatedData, float maxValue);
		
			//=== TrialMat Stats2Map
			vec1<int> checkIfNeedDisplayStat(vec1<PVALUECOORD> significantValue, int idCurrentElec);
			vec2<int>checkIfConditionStat(vec1<PVALUECOORD> significantValue, vec1<int> significantIdMap, int nbRow);

		private:
			void jetColorMap512(QColor *colorMap);
			void createColorBar(QPainter *painter);
			void defineLineSeparation(QPainter *painter, vector<int> nbTrialPerRow, int nbCol);
			void createTimeLegend(QPainter *painter, PROV *myprovFile);
			void createTrialsLegend(QPainter *painter, PROV *myprovFile);

		public:
			QPixmap pixmapTemplate;
			vector<QRect> subMatrixes;
			vector<QRect> separationLines;
			QRect MatrixRect;
			QRect colorBarRect;
			QRect fullMap;
			QColor *ColorMapJet = nullptr;
			int Width, Heigth;
		};
	}
}




#endif