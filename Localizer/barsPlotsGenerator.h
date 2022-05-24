#ifndef _BARSPLOTSGENERATOR_H
#define _BARSPLOTSGENERATOR_H

#include <iostream>
#include <algorithm>
#include "Utility.h"
#include "PROV.h"
#include "eegContainer.h"
#include "TriggerContainer.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include "optionsParameters.h"

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
			drawBars(PROV *myprovFile, std::string outputFolder, QSize size);
			~drawBars();
			void drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, vec1<PVALUECOORD> significantValue, eegContainer* myeegContainer);
			QString createPicPath(std::string picFolder, eegContainer* myeegContainer, int idElec);
		public:
			PROV *myprovFile = nullptr;
			std::string picOutputFolder = "";
		};

		class drawPlots : public baseCanvas
		{
		public:
			drawPlots(PROV *myprovFile, std::string outputFolder, QSize size);
			~drawPlots();
			void drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, int card2Draw);
			QString createPicPath(std::string picFolder, eegContainer* myeegContainer, int cards2Draw, int nbFigureDrawn);
		public:
			PROV *myprovFile = nullptr;
			std::string picOutputFolder = "";
		};
	}
}


#endif
