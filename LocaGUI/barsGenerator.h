#pragma once
#include <iostream>
#include <QPixmap>
#include <QPainter>
#include <algorithm>
#include "windows.h"
#include "Utility.h"
#include "PROV.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\ELAN\ELAN.h"		

using namespace std;

namespace InsermLibrary
{
	namespace DrawPlotsVisu
	{
		class baseCanvas
		{
		public :
			baseCanvas(PROV *p_prov);
			~baseCanvas();
		private:
			void drawTemplate(PROV *p_prov);
		public :
			int nbElecPerFigure = 3;
			int nbSite = 20;
			int nbColLegend;
			int screenWidth, screenHeigth;
			QPixmap pixmapTemplate;
		};

		class drawBars : public baseCanvas
		{
		public :
			drawBars(PROV *p_prov, string outputFolder);
			~drawBars();
			void drawDataOnTemplate(ELAN *p_elan, LOCAANALYSISOPTION *p_anaopt, int currentFreqBand, double *** bigdata, vector<int> p_eventsUsed, vector<PVALUECOORD> p_significantValue);
		public :
			PROV *currentProv;
			string picOutputFolder;
		};

		class drawPlots : public baseCanvas
		{
		public:
			drawPlots(PROV *p_prov, string outputFolder);
			~drawPlots();
			void drawDataOnTemplate(ELAN *p_elan, LOCAANALYSISOPTION *p_anaopt, double *** bigdata, vector<int> p_eventsUsed, int currentFreqBand, int cards2Draw);
		public:
			bool modif = false;
			PROV *currentProv;
			string picOutputFolder;
		};
	}
}