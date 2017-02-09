#ifndef _LOCA_H
#define _LOCA_H

#include <iostream>	
#include <fstream>	
#include <iomanip>	
#include <vector>
#include "windows.h"
#include <algorithm>	
#include <QtWidgets/QMainWindow>	
#include <QPainter>	
#include <QColormap>
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\MATH\MATLABFUNC.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\TRC\TRC.h"	
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\FFTW 3.3.4 x86\fftw3.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\ELAN\ELAN.h"		
//#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\PROV\PROV.h"	
#include "PROV.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\wilcox.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\wilcox.hpp"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\kruskall.h"
#include "Stats.h"
#include "mapsGenerator.h"
#include "CRperf.h"
#include "Utility.h"


using namespace std;
using namespace MicromedLibrary;
using namespace InsermLibrary;
using namespace DrawCard;

namespace InsermLibrary	
{
	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA(OptionLOCA *p_options);
		~LOCA();
		void LocaSauron(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void loc_create_pos(string posFile_path, string posXFile_path, TRC *p_trc, int p_beginningCode, PROV *p_prov);
		void renameTrigger(TRIGGINFO *triggers, TRIGGINFO* downsampledTriggers, PROV *p_prov);
		void loc2_write_conf(string confFile_path, TRC *p_trc, ELAN *p_elan);
		void loc_eeg2erp(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, string outputFolder);
		void loc_env2plot(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, int currentFreqBand, string outputFolder);
		void loc_bar2plot(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt, vector<int> p_correspondingEvents, int currentFreqBand, string outputFolder);
		void loca_trialmat(ELAN *p_elan, int p_numberFrequencyBand, PROV *p_prov, string p_outputMapLabel, string p_outputFolder, vector<int> p_correspondingEvents);
		vector<int> processEvents(PROV *p_prov);
		void cat2ellaRTTrigg(PROV *p_prov);
		vector<int> sortTrials(PROV *p_prov);
		vector<int> sortByMainCode(PROV *p_prov);
		void sortBySecondaryCode(vector<int> totreat);
		void sortByLatency(vector<int> totreat);
		void cat2ellaExtractData(elan_struct_t *p_elan_struct, double ***p_eegData, int v_win_sam[2]);
		double stdMean(double **p_eegDataChanel, int p_window_sam[2]);

	signals : 
		void sendLogInfo(QString);

	public :
		TRIGGINFO *triggTRC = nullptr, *triggDownTRC = nullptr, *triggCatEla = nullptr, *triggCatElaNoSort = nullptr;
	private :
		QPixmap *pixMap = nullptr;
		QPainter *painter = nullptr;
		OptionLOCA *opt = nullptr;
	};
}
#endif