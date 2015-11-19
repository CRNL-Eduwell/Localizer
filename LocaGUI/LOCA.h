#ifndef _LOCA_H
#define _LOCA_H

/***********************************************************************************************************************************************************************************************************************************************************/
/********************************************************************************************************               Library                    *********************************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/
#include <iostream>																					/*|					Standard Library						|*******************************************************************************************/
#include <fstream>																					/*|					File Manipulating Library				|*******************************************************************************************/
#include <iomanip>																					/*|					std::setw								|*******************************************************************************************/
#include <vector>																					/*|															|*******************************************************************************************/
#include "windows.h"																				/*|															|*******************************************************************************************/
#include <algorithm>																				/*|						Min max element						|*******************************************************************************************/
#include <QtWidgets/QMainWindow>																	/*|															|*******************************************************************************************/
#include <QPainter>																					/*|															|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\MATH\MATLABFUNC.h"				/*|					 MATLAB Library (Debug)					|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\TRC\TRC.h"						/*|					   TRC Library (Debug)					|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\FFTW 3.3.4 x86\fftw3.h"					/*|						  FFTW Library						|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\ELAN\ELAN.h"						/*|					ELAN Library (Debug)					|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\PROV\PROV.h"						/*|					ELAN Library (Debug)					|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\wilcox.h"						/*|					ELAN Library (Debug)					|*******************************************************************************************/
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\C++\Debug\STATS\wilcox.hpp"						/*|					ELAN Library (Debug)					|*******************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/
#include <QColormap>
/**********************************************************************************************************************************************************************************************************************************************/
/**************************************************************************************************			 	  LOCA Class		         **************************************************************************************************/
/**********************************************************************************************************************************************************************************************************************************************/
namespace InsermLibrary																																																			  	      /*||*/
{
	struct PVALUECOORD
	{
		int elec = -69;
		int condit = -69;
		int window = -69;
		int vectorpos = -69;
		double pValue = -69;
	};

	struct MATRIXCOORD
	{
		int x = -69;
		int y = -69;
		int width = -69;
		int heigth = -69;
	};

	class TRIGG
	{
	public:
		TRIGG(int p_valueTrigger, int p_sampleTrigger, int p_rtMs, int p_rtCode, int p_origPos);
		~TRIGG();

		int valueTrigger;
		int sampleTrigger;
		int rt_ms;
		int rt_code;
		int origPos;
	};

	class TRIGGINFO
	{
	public : 
		TRIGGINFO(unsigned long *p_valueTrigg, unsigned long *p_sampleTrigg, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int *p_rtMs, int *p_rtCode, int *p_origPos, int p_numberTrigg, int p_downFactor);
		TRIGGINFO(int *p_valueTrigg, int *p_sampleTrigg, int p_numberTrigg, int p_downFactor);
		~TRIGGINFO();

		TRIGG **trigg;
		std::vector<int> mainGroupSub;
		int numberTrigg;
		int downFactor;
	};

	class LOCAANALYSISOPTION
	{
	public :
		LOCAANALYSISOPTION(std::vector<std::vector<double>> p_frequencys, std::vector<std::vector<bool>> p_analysisDetails, std::string p_trcPath, std::string p_provPath, std::string p_patientFolder, std::string p_task, std::string p_expTask);
		~LOCAANALYSISOPTION();

		std::vector<std::vector<double>> frequencys;
		std::vector<std::vector<bool>> analysisDetails;
		std::string trcPath = "";
		std::string provPath = "";
		std::string patientFolder = "";
		std::string task = "";
		std::string expTask = "";
	};

	class LOCA : public QObject
	{
		Q_OBJECT

	public:
		LOCA();
		~LOCA();
		void LocaVISU(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaLEC1(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMCSE(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMVIS(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMVEB(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMASS(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaLEC2(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaMOTO(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaAUDI(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);
		void LocaARFA(InsermLibrary::ELAN *p_elan, InsermLibrary::PROV *p_prov, LOCAANALYSISOPTION *p_anaopt);

		void loc_create_pos(std::string posFile_path, std::string posXFile_path, MicromedLibrary::TRC *p_trc, int p_beginningCode, InsermLibrary::PROV *p_prov);
		void renameTrigger(TRIGGINFO *triggers, TRIGGINFO* downsampledTriggers, InsermLibrary::PROV *p_prov);
		void loc2_write_conf(std::string confFile_path, MicromedLibrary::TRC *p_trc, InsermLibrary::ELAN *p_elan);
		void loc_eeg2erp(InsermLibrary::ELAN *p_elan, std::string p_path, std::string p_exp_task, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int* v_window_ms, int nb_site);
		void loc_env2plot(InsermLibrary::ELAN *p_elan, int p_numberFrequencyBand, std::string p_path, std::string p_exp_task, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int* v_window_ms, int nb_site);
		void loc_bar2plot(InsermLibrary::ELAN *p_elan, int p_numberFrequencyBand, std::string p_path, std::string p_exp_task, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int* v_window_ms, int nb_site);
		void drawCards(InsermLibrary::ELAN *p_elan, std::string p_path, std::string p_exp_task, int cards2Draw, double *** bigdata, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int v_win_sam[2], int nb_site, std::vector<int> indexEventUsed, std::vector<int> EventUsed);
		void drawBars(InsermLibrary::ELAN *p_elan, std::string p_path, std::string p_exp_task, int cards2Draw, double *** bigdata, int* v_code, int v_codeLength, std::string* a_code, int a_codeLength, int v_win_sam[2], int nb_site, std::vector<int> indexEventUsed, std::vector<int> EventUsed);
		void loca_trialmat(InsermLibrary::ELAN *p_elan, int p_numberFrequencyBand, InsermLibrary::PROV *p_prov, std::string p_outputMapLabel, std::string p_outputFolder);
		std::vector<std::vector<double>> interpolateData(double **p_eegData, int p_numberSubTrial, int p_windowSize, int p_beginTrigg, int p_interpolFactor);
		std::vector<std::vector<double>> interpolateDataVert(std::vector<std::vector<double>> p_eegData, int p_interpolFactor);
		void eegData2ColorMap(std::vector<std::vector<double>> p_eegDataInterpolated, std::vector<int> p_colorX[512], std::vector<int> p_colorY[512], int p_numberSubTrial, int p_windowSize, int p_interpolFactorX, int p_interpolFactorY, double p_minVal, double p_maxVal);
		void cat2ellaRTTrigg(mainEventBLOC **p_mainEvents, int p_numberMainEvents, secondaryEventBLOC ***p_responseEvents, int* p_sizeRespEvents);
		std::vector<int> sortTrials(InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int numCol, int numRow);
		std::vector<int> sortByMainCode(InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int numCol, int numRow);
		void sortBySecondaryCode(std::vector<int> totreat);
		void sortByLatency(std::vector<int> totreat);
		void cat2ellaExtractData(elan_struct_t *p_elan_struct, double ***p_eegData, int v_win_sam[2]);
		void createMapTemplate(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, InsermLibrary::PROV *p_prov, int v_window_ms[2], QColor *p_colorMap, mainEventBLOC **p_mainEvents, std::vector<int> p_indexes, int p_numberRow, int p_numberCol);
		void createColorBar(QPainter *p_painterTemplate, QColor *p_colorMap);
		void createTrialLegend(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, InsermLibrary::PROV *p_prov, mainEventBLOC **p_mainEvents, int p_numberMainEvents);
		void createTimeLegend(QPainter *p_painterTemplate, int v_window_ms[2]);
		void matrixLines(QPainter *p_painterTemplate, MATRIXCOORD *p_coordMat, std::vector<int> p_indexes, int p_numberRow, int p_numberCol);
		double stdMean(double **p_eegDataChanel, int p_window_sam[2]);
		void graduateColorBar(QPainter *p_painterTemplate, int p_maxiValue);
		void jetColorMap512(QColor *p_C);
		std::vector<int> findNum(int *tab, int sizetab, int value2find);
		template<typename T> std::vector<T> split(const T & str, const T & delimiters);

	signals : 
		void sendLogInfo(QString);

	public :
		TRIGGINFO *triggTRC, *triggDownTRC, *triggCatEla, *triggCatElaNoSort;
	private :
		QPixmap *pixMap;
		QPainter *painter;
	};
}
#endif