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
	struct picOptionLOCA
	{
		int width = 576;
		int height = 432;
		int interpolationFactorX = 3;
		int interpolationFactorY = 3;
	};

	struct statsOptionLOCA
	{
		bool useWilcoxon = true;
		bool useFDRWil = true;
		float pWilcoxon = 0.01;
		bool useKruskall = true;
		bool useFDRKrus = true;
		float pKruskall = 0.01;
	};

	struct OptionLOCA
	{
		picOptionLOCA picOption;
		statsOptionLOCA statsOption;
	};

	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA(OptionLOCA *p_options);
		~LOCA();
		void LocaVISU(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaLEC1(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMCSE(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMVIS(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMVEB(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMASS(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaLEC2(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMOTO(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaAUDI(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaARFA(ELAN *p_elan, PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void loc_create_pos(string posFile_path, string posXFile_path, TRC *p_trc, int p_beginningCode, PROV *p_prov);
		void renameTrigger(TRIGGINFO *triggers, TRIGGINFO* downsampledTriggers, PROV *p_prov);
		void loc2_write_conf(string confFile_path, TRC *p_trc, ELAN *p_elan);
		void loc_eeg2erp(ELAN *p_elan, string p_path, string p_exp_task, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int* v_window_ms, int nb_site);
		void loc_env2plot(ELAN *p_elan, int p_numberFrequencyBand, string p_path, string p_exp_task, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int* v_window_ms, int nb_site);
		//void loc_bar2plot(ELAN *p_elan, PROV *p_prov, int p_numberFrequencyBand, string p_path, string p_exp_task, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int* v_window_ms, int nb_site, mainEventBLOC **p_mainEvents, vector<int> p_correspondingEvents);
		void drawCards(ELAN *p_elan, string p_path, string p_exp_task, int cards2Draw, double *** bigdata, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int v_win_sam[2], int nb_site, vector<int> indexEventUsed, vector<int> EventUsed);
		void drawBars(ELAN *p_elan, PVALUECOORD **p_significantValue, int p_sizeSignificant, string p_path, string p_exp_task, int cards2Draw, double *** bigdata, int* v_code, int v_codeLength, string* a_code, int a_codeLength, int v_win_sam[2], int nb_site, vector<int> indexEventUsed, vector<int> EventUsed);
		vector<int> processEvents(PROV *p_prov);
		void loca_trialmat(ELAN *p_elan, int p_numberFrequencyBand, PROV *p_prov, string p_outputMapLabel, string p_outputFolder, vector<int> p_correspondingEvents);
		vector<vector<vector<double>>> calculatePValueKRUS(elan_struct_t *p_elan_struct, PROV *p_prov, vector<int> correspEvent, double ***eegData, int windowMS[2]);
		PVALUECOORD **calculateFDR(vector<vector<vector<double>>> pArray3D, int &p_copyIndex);
		void cat2ellaRTTrigg(PROV *p_prov);
		vector<int> sortTrials(PROV *p_prov);
		vector<int> sortByMainCode(PROV *p_prov);
		void sortBySecondaryCode(vector<int> totreat);
		void sortByLatency(vector<int> totreat);
		void cat2ellaExtractData(elan_struct_t *p_elan_struct, double ***p_eegData, int v_win_sam[2]);
		double stdMean(double **p_eegDataChanel, int p_window_sam[2]);
		vector<int> findNum(int *tab, int sizetab, int value2find);

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