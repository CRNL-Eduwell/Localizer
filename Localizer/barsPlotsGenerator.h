#ifndef _BARSPLOTSGENERATOR_H
#define _BARSPLOTSGENERATOR_H

#include <iostream>
#include <algorithm>
#include "Utility.h"
#include "PROV.h"
#include "eegContainer.h"
#include"eegContainerParameters.h"
#include <QPixmap>
#include <QPainter>

using namespace std;
using namespace InsermLibrary;

namespace InsermLibrary
{
	namespace DrawbarsPlots
	{
		class baseCanvas
		{
		public:
			baseCanvas(PROV *myprovFile, int width, int heigth);
			~baseCanvas();
		private:
			void drawTemplate(PROV *myprovFile);
		public:
			int nbElecPerFigure = 3;
			int nbSite = 20;
			int nbColLegend = 0;
			int Width = 0, Heigth = 0;
			QPixmap pixmapTemplate;
		};

		class drawBars : public baseCanvas
		{
		public:
			drawBars(PROV *myprovFile, string outputFolder, QSize size);
			~drawBars();
			void drawDataOnTemplate(vec3<float> &bigData, TRIGGINFO *triggCatEla, vec1<PVALUECOORD> significantValue, eegContainer* myeegContainer);
			QString createPicPath(string picFolder, eegContainer* myeegContainer, int idElec);
		public:
			PROV *myprovFile = nullptr;
			string picOutputFolder = "";
		};

		class drawPlots : public baseCanvas
		{
		public:
			drawPlots(PROV *myprovFile, string outputFolder, QSize size);
			~drawPlots();
			void drawDataOnTemplate(vec3<float> &bigData, TRIGGINFO *triggCatEla, eegContainer* myeegContainer, int card2Draw);
			QString createPicPath(string picFolder, eegContainer* myeegContainer, int cards2Draw, int nbFigureDrawn);
		public:
			PROV *myprovFile = nullptr;
			string picOutputFolder = "";
		};
	}
}


#endif