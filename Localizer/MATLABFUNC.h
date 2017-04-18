#ifndef _MATLABFUNC_H
#define _MATLABFUNC_H

/***********************************************************************************************************************************************************************************************************************************************************/
/********************************************************************************************************               Library                    *********************************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/
#include <iostream>																					/*|					Standard Library						|*******************************************************************************************/
#include <complex>																					/*|					Complex Number Library					|*******************************************************************************************/
#include "time.h"																					/*|					Time Measure Library					|*******************************************************************************************/
#include "Utility.h"
#include "D:\Users\Florian\Documents\Arbeit\Software\DLL\FFTW 3.3.4 x64\fftw3.h"					/*|					FFTW Library							|*******************************************************************************************/
#define PI 3.14159265																				/*|															|*******************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/**************************************************************************************************             MATLABFUNC Class             **************************************************************************************************/
/**********************************************************************************************************************************************************************************************************************************************/
namespace InsermLibrary															    																																					  /*||*/
{															    																																										  /*||*/
	class FFTINFO																			    																																		  /*||*/
	{																																																									  /*||*/
	public:																																																								  /*||*/
		FFTINFO(int fftFlag, int lengthFFT);																									/*|		fftFlag : -1 FORWARD +1 BACKWARD													 |*/
		~FFTINFO();																																	    																				  /*||*/
																																																										  /*||*/
		fftwf_complex *inputArray = nullptr;																																																  /*||*/
		fftwf_complex *outputArray = nullptr;																																															  /*||*/
		int lengthArray;																																    																			  /*||*/
		fftwf_plan plan = nullptr;																																								    									  /*||*/
	};																																	    																							  /*||*/
																																																										  /*||*/
	class FIRINFO																																	    																				  /*||*/
	{																																																									  /*||*/
	public:																																																								  /*||*/
		FIRINFO(int p_fMin, int p_fMax, float p_fSampling, int p_lengthFir);
		~FIRINFO();																																																						  /*||*/
																																																										  /*||*/
		void HilbertCoeff(float *HilCoeff, int signalLength);																																										      /*||*/
		void time2FreqFir2(float *firCoeff, int lengthFir);																																										      /*||*/
		void fir2Frequential(float *FirCoeffOut, int p_length, float *p_frequency, int p_frequency_length, float *p_magnitude, int p_magnitude_length);																			      /*||*/
		void fir2Temporal(float *FirCoeffOut, int p_length, float *p_frequency, int p_frequency_length, float *p_magnitude, int p_magnitude_length);																					  /*||*/
		float* interpolateBreakPointsGrid(int npt, float *p_frequency, int p_frequency_length, float *p_magnitude);																													  /*||*/
		void complexArrayToZero(fftwf_complex *complexArray, int lengthArray);																																							  /*||*/
																																																										  /*||*/
		float *firCoeffTime = nullptr, *firCoeffFreq = nullptr, *hilbertCoeff = nullptr;																																				  /*||*/
		float *frequency = new float[6]{ 0, 0, 0, 0, 0, 1 };																																											  /*||*/
		float *magnitude = new float[6]{ 0, 0, 1, 1, 0, 0 };																																	    	   							      /*||*/
		int lengthFir = 0;																																	    																		  /*||*/
		int fMin = 0;																																	    																		  /*||*/
		int fMax = 0;																																	    																		  /*||*/
		float fSampling = 0.0;																																	    																	  /*||*/
	};																																																									  /*||*/
																																																										  /*||*/
	class MATLABFUNC																																																					  /*||*/
	{																																																									  /*||*/
	public:																																	    																						  /*||*/
		/*|		Class Methods																																																				 |*/
		static void Convolution2(float *p_source_tab, int p_source_tab_length, float *p_out_tab, int p_coeff);																														  /*||*/
		static void Convolution(float *p_source_tab, int p_source_tab_length, float *p_out_tab, int p_coeff);																															  /*||*/
		static void bandPassHilbertFreq(float *SignIn, float *SignOut, FIRINFO *fir, FFTINFO* fftForward, FFTINFO* fftBackward);																										 /*||*/
		static void hilbertEnvellope(float* xr, int xrLength, float *SignOut, FFTINFO* fftForward, FFTINFO* fftBackward);																												  /*||*/
		static void bandPassFilter(float *SignIn, int SignInLength, float *FirCoeff, float * SignOut, FFTINFO* fftForward, FFTINFO* fftBackward);																						/*||*/
		static void FFTWForward(FFTINFO* fftForward);																																													 /*||*/
		static void FFTWBackward(FFTINFO* fftBackward);																																													/*||*/
		static float* Hamming(int length);																																																  /*||*/
		static float* Hanning(int length);																																																  /*||*/
		static float* BlackmanHarris(int length);																																														  /*||*/
		static float stdMean(vec2<float> eegDataChanel, int windowSam[2]);

	};																																															    									  /*||*/
}																																																										  /*||*/
/**********************************************************************************************************************************************************************************************************************************************/
#endif