#ifndef _MAPSGENERATOR_H
#define _MAPSGENERATOR_H

#include <iostream>
#include "Utility.h"
#include "PROV.h"
#include "optionsParameters.h"

#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QPainterPath>

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
			void trialmatTemplate(std::vector<int> trialsPerRow, PROV *myprovFile);
			void trialmatTemplate(std::vector<std::tuple<int, int, int>> codesAndTrials, PROV* myprovFile);
			void graduateColorBar(QPainter *painter, int maxValue);
			void drawVerticalZeroLine(QPainter *painter, PROV* myprovFile);
			void displayStatsOnMap(QPainter *painter, vec2<int> idCurrentMap, vec1<PVALUECOORD> significantValue, PROV* myprovFile);
			void displayStatsOnMap(QPainter* painter, vec1<PVALUECOORD> significantValue, int electrodeIndex, PROV* myprovFile);
			void drawMapTitle(QPainter *painter, std::string title);

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
			void defineLineSeparation(QPainter *painter, std::vector<int> nbTrialPerRow, int nbCol);
			void defineLineSeparation(QPainter* painter, std::vector<std::tuple<int, int, int>> codesAndTrials, PROV* myprovFile);
			void createTimeLegend(QPainter *painter, PROV *myprovFile);
			void createTrialsLegend(QPainter *painter, PROV *myprovFile);
			void createTrialsLegend(QPainter* painter, std::vector<std::tuple<int, int, int>> codesAndTrials, PROV* myprovFile);

		public:
			QPixmap pixmapTemplate;
			std::vector<QRect> subMatrixes;
			std::vector<int> subMatrixesCodes;
			std::vector<QRect> separationLines;
			QRect MatrixRect;
			QRect colorBarRect;
			QRect fullMap;
			QColor *ColorMapJet = nullptr;
			int Width, Heigth;
		};
	}
}




#endif
