#pragma once
#include <iostream>
#include <QPixmap>
#include <QPainter>
#include <QFileInfo>
#include <QColormap>
#include <QTextOption>
#include "Utility.h"
#include "PROV.h"

using namespace std;

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
			void trialmatTemplate(vector<int> trialsPerRow, int windowMS[2], InsermLibrary::PROV *p_prov);
			void graduateColorBar(QPainter *painter, int maxValue);
			void drawVerticalZeroLine(QPainter *painter, int windowMs[2]);
			void displayStatsOnMap(QPainter *painter, vector<vector<int>> indexCurrentMap, vector<PVALUECOORD> significantValue, int windowMS[2], int nbRow);
			void drawMapTitle(QPainter *painter, string outputLabel, string elecName);

			//=== TrialMat Data2Color Functions
			vector<vector<double>> horizontalInterpolation(double ** chanelToInterpol, int interpolationFactor, TRIGGINFO *triggCatEla, int nbSubTrials, int nbSampleWindow, int indexBegTrigg);
			vector<vector<double>> verticalInterpolation(vector<vector<double>> chanelToInterpol, int interpolationFactor);
			void eegData2ColorMap(vector<int> colorX[512], vector<int> colorY[512], vector<vector<double>> interpolatedData, int nbSubTrials, int nbSampleWindow, int horizInterpFactor, int vertInterpFactor, double maxValue);
			void eegData2ColorMap(vector<int> colorX[512], vector<int> colorY[512], double **eegData, TRIGGINFO *triggCatEla, int nbSubTrials, int nbSampleWindow, int indexBegTrigg, double maxValue);

			//=== TrialMat Stats2Map
			vector<int> checkIfNeedDisplayStat(vector<PVALUECOORD> significantValue, int indexCurrentElec);
			vector<vector<int>> checkIfConditionStat(vector<PVALUECOORD> significantValue, vector<int> allIndexMap,  int numberRow);

		private:
			void jetColorMap512(QColor *colorMap);
			void createColorBar(QPainter *painter);
			void defineLineSeparation(QPainter *painter, vector<int> nbTrialPerRow, int nbCol);
			void createTimeLegend(QPainter *painter, int windowMS[2]);
			void createTrialsLegend(QPainter *painter, PROV *p_prov);

		public :
			QPixmap pixmapTemplate;
			vector<QRect> subMatrixes;
			vector<QRect> separationLines;
			QRect MatrixRect;
			QRect colorBarRect;
			QRect fullMap;
			QColor *ColorMapJet;
			int Width, Heigth;
		};
	}
}