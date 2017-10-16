/***********************************************************************************************************************************************************************************************************************************************************/
/********************************************************************************************************               Library                    *********************************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/
#include <iostream>																					/*|						Standard Library					|*******************************************************************************************/
#include <complex>																					/*|					Complex Number Library					|*******************************************************************************************/
#include <fstream>																					/*|					File Manipulating Library				|*******************************************************************************************/
#include <windows.h>																				/*|															|*******************************************************************************************/
#include <stdio.h>																					/*|															|*******************************************************************************************/
																									/*|															|*******************************************************************************************/
#include "MATLABFUNC.h"																				/*|					Matlab Function Library					|*******************************************************************************************/
/***********************************************************************************************************************************************************************************************************************************************************/

InsermLibrary::FFTINFO::FFTINFO(int fftFlag, int lengthFFT)
{
	lengthArray = lengthFFT;	
	inputArray = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(lengthArray));
	outputArray = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(lengthArray));
	plan = fftwf_plan_dft_1d(lengthArray, inputArray, outputArray, fftFlag, FFTW_ESTIMATE);
}

InsermLibrary::FFTINFO::~FFTINFO()
{
	fftwf_free(inputArray);
	fftwf_free(outputArray);	
	fftwf_destroy_plan(plan);
}

InsermLibrary::FIRINFO::FIRINFO(int p_fMin, int p_fMax, float p_fSampling, int p_lengthFir)
{
	lengthFir = p_lengthFir;	
	fMin = p_fMin;					
	fMax = p_fMax;			
	fSampling = p_fSampling;	

	frequency[1] = (fMin - 0.5) / (fSampling / 2);	
	frequency[2] = fMin / (fSampling / 2);
	frequency[3] = fMax / (fSampling / 2);
	frequency[4] = (fMax + 0.5) / (fSampling / 2);
			
	firCoeffTime = new float[lengthFir]();
	fir2Temporal(firCoeffTime, lengthFir - 1, frequency, 6, magnitude, 6);			
	time2FreqFir2(firCoeffTime, lengthFir);

	firCoeffFreq = new float[lengthFir]();
	fir2Frequential(firCoeffFreq, lengthFir - 1, frequency, 6, magnitude, 6);			

	hilbertCoeff = new float[lengthFir]();
	HilbertCoeff(hilbertCoeff, lengthFir);
}

InsermLibrary::FIRINFO::~FIRINFO()
{
	delete[] frequency;	
	delete[] magnitude;	
	delete[] firCoeffTime;	
	delete[] firCoeffFreq;
	delete[] hilbertCoeff;
}

void InsermLibrary::FIRINFO::HilbertCoeff(float *HilCoeff, int signalLength)
{
	if ((signalLength > 0) && (2 * trunc(signalLength / 2) == signalLength))
	{
		//Even and Non-Empty																																																				  
		HilCoeff[0] = 1;
		HilCoeff[signalLength / 2] = 1;
		for (int i = 1; i < signalLength / 2; i++)
		{
			HilCoeff[i] = 2;
		}
	}
	else if (signalLength > 0)
	{
		//Odd and Non-Empty																																																					  
		HilCoeff[0] = 1;
		for (int i = 1; i < signalLength / 2; i++)
		{
			HilCoeff[i] = 2;
		}
	}
}

/**********************************************************************************************************************************************************************************************************************************************/
/*																											  FIR2																															  */
/*																				Returns double table containing the n+1 coefficients of an order n FIR filter.																				  */
/*																		The frequency-magnitude characteristics of this filter match those given by tables p_frequency and p_magnitude.														  */
/*																																																											  */
/*																											Values														    																  */
/*														(int)	   p_length    : Length of data																																				  */
/*														(double *) p_frequency : is a table of frequency points in the range from 0 to 1, where 1 corresponds to the Nyquist frequency.														  */
/*																				 The first point of p_frequency must be 0 and the last point 1. The frequency points must be in increasing order.											  */
/*														(double *) p_magnitude : is a table containing the desired magnitude response at the points specified in p_frequency.																  */
/*																																																											  */
/*																						/!\ p_frequency and p_magnitude must be the same length /!\																							  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::FIRINFO::time2FreqFir2(float *firCoeff, int lengthFir)																																								  //
{																																																										  	  //
	FFTINFO *fftForward = new FFTINFO(FFTW_FORWARD, lengthFir);																																												  //
																																																											  //
	//Make zero-phase filter function																																																		  //
	for (int i = 0; i < lengthFir; i++)																																																		  //
	{																																																										  //
		fftForward->inputArray[i][0] = firCoeff[i];																																															  //
		fftForward->inputArray[i][1] = 0;																																																	  //
	}																																																										  //
																																																								  //
	InsermLibrary::MATLABFUNC::FFTWForward(fftForward);																																																				  //
																																																											  //
	for (int i = 0; i < lengthFir; i++)																																																		  //
	{																																																										  //
		firCoeff[i] = sqrt((fftForward->outputArray[i][0] * fftForward->outputArray[i][0]) + (fftForward->outputArray[i][1] * fftForward->outputArray[i][1]));																				  //
	}																																																										  //
																																																											  //
	delete fftForward;																																																			  		 	  //
 }																																																											  //
																																																											  //
void InsermLibrary::FIRINFO::fir2Frequential(float *FirCoeffOut, int p_length, float *p_frequency, int p_frequency_length, float *p_magnitude, int p_magnitude_length)																  //
{																																																											  //
	float *MF2_Window = nullptr, *HH = nullptr;																																															  //
	float dt = 0;																																																							  //
	int MF2_length = 0, npt = 0;																																																			  //	
																																																											  //
	MF2_length = p_length + 1;		//Use Filter Length insted of filter order																																								  //
	if (MF2_length < 1024)																																																					  //
	{																																																										  //
		npt = 512;																																																							  //
	}																																																										  //
	else																																																									  //
	{																																																										  //
		npt = pow(2, ceil(log2(MF2_length)));																																																  //
	}																																																										  //
																																																											  //
	MF2_Window = InsermLibrary::MATLABFUNC::Hamming(MF2_length);																																											  //
																																																											  //
	HH = interpolateBreakPointsGrid(npt, p_frequency, p_frequency_length, p_magnitude);																																						  //
	dt = 0.5 * (MF2_length - 1);																																																			  //
																																																											  //
	fftwf_complex *H_Complex = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(npt));																																						  //
	complexArrayToZero(&H_Complex[0], npt);																																																	  //
	fftwf_complex *H_Complex_Conjugate = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(npt - 2));																																			  //			
	complexArrayToZero(&H_Complex_Conjugate[0], npt - 2);																																												  	  //
	fftwf_complex *H_Complex_Final = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(2 * (npt - 1)));																																		  //
	complexArrayToZero(&H_Complex_Final[0], 2 * (npt - 1));																																											  		  //
																																																											  //
	/*						Fourier transform of real series				   */																																							  //
	/*HH[i] * exp(rad[i]) =>	Real =  HH[i] * exp(rad[i][0]) * cos(rad[i][1])*/																																							  //
	/*							Imag =  HH[i] * exp(rad[i][0]) * sin(rad[i][1])*/																																							  //
	for (int i = 0; i < npt; i++)																																																			  //
	{																																																										  //
		H_Complex[i][0] = HH[i] * exp(0) * cos((-dt * PI * i) / (npt - 1));																																									  //
		H_Complex[i][1] = HH[i] * exp(0) * sin((-dt * PI * i) / (npt - 1));																																									  //
		H_Complex_Final[i][0] = H_Complex[i][0];																																															  //
		H_Complex_Final[i][1] = H_Complex[i][1];																																															  //
	}																																																										  //
																																																											  //
	for (int i = npt - 3; i > 1; i--)																																																		  //
	{																																																										  //
		H_Complex_Conjugate[i][0] = H_Complex[npt - 2 - i][0];																																												  //
		H_Complex_Conjugate[i][1] = H_Complex[npt - 2 - i][1];																																												  //
	}																																																										  //
																																																											  //
	for (int i = npt, j = 0; i < 2 * (npt - 1), j<(npt - 2); i++, j++)																																										  //
	{																																																										  //
		H_Complex_Final[i][0] = H_Complex_Conjugate[j][0];																																													  //
		H_Complex_Final[i][1] = -H_Complex_Conjugate[j][1];																																													  //
	}																																																										  //
																																																											  //
	FFTINFO *fftBackward = new FFTINFO(FFTW_BACKWARD, 2 * (npt - 1));																																										  //
	for (int i = 0; i < fftBackward->lengthArray; i++)
	{
		fftBackward->inputArray[i][0] = H_Complex_Final[i][0];
		fftBackward->inputArray[i][1] = H_Complex_Final[i][1];
	}
	InsermLibrary::MATLABFUNC::FFTWBackward(fftBackward);																																																				  //
																																																											  //
	//Refft fir coeff																																																						  //
	FFTINFO *fftForward = new FFTINFO(FFTW_FORWARD, MF2_length);																																											  //
	for (int i = 0; i < MF2_length; i++)																																																	  //
	{																																																										  //
		fftForward->inputArray[i][0] = fftBackward->outputArray[i][0] * MF2_Window[i];																																						  //
		fftForward->inputArray[i][1] = 0;																																																	  //
	}																																																										  //
																																																											  //
	InsermLibrary::MATLABFUNC::FFTWForward(fftForward);																																																				  //
																																																											  //
	//sqrt(re² + im²)																																																						  //
	for (int i = 0; i < MF2_length; i++)																																																	  //
	{																																																										  //
		FirCoeffOut[i] = sqrt((fftForward->outputArray[i][0] * fftForward->outputArray[i][0]) + (fftForward->outputArray[i][1] * fftForward->outputArray[i][1]));																			  //
	}																																																										  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	delete[] MF2_Window;																		  /*||*/																																	  //
	MF2_Window = NULL;																			  /*||*/																																	  //
																								  /*||*/																																	  //
	delete[] HH;																				  /*||*/																																	  //
	HH = NULL;																					  /*||*/																																	  //
																								  /*||*/																																	  //
	fftwf_free(H_Complex);																		  /*||*/																																	  //
	fftwf_free(H_Complex_Conjugate);															  /*||*/																																	  //
	fftwf_free(H_Complex_Final);																  /*||*/																																	  //
	delete fftForward;																			  /*||*/																																	  //
	delete fftBackward;																			  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
																																																											  //
void InsermLibrary::FIRINFO::fir2Temporal(float *FirCoeffOut, int p_length, float *p_frequency, int p_frequency_length, float *p_magnitude, int p_magnitude_length)																	  //
{																																																											  //
	float *MF2_Window = nullptr, *HH = nullptr;																																															  //
	float dt = 0;																																																							  //
	int MF2_length = 0, npt = 0;																																																			  //		
																																																											  //
	MF2_length = p_length + 1;	//Use Filter Length insted of filter order																																									  //
	if (MF2_length < 1024)																																																					  //
	{																																																										  //
		npt = 512;																																																							  //
	}																																																										  //
	else																																																									  //
	{																																																										  //
		npt = pow(2, ceil(log2(MF2_length)));																																																  //
	}																																																										  //
																																																											  //
	MF2_Window = InsermLibrary::MATLABFUNC::Hamming(MF2_length);																																											  //		
																																																											  //
	HH = interpolateBreakPointsGrid(npt, p_frequency, p_frequency_length, p_magnitude);																																						  //
	dt = 0.5 * (MF2_length - 1);																																																			  //
																																																											  //
	fftwf_complex *H_Complex = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(npt));																																						  //
	complexArrayToZero(&H_Complex[0], npt);																																																	  //
	fftwf_complex *H_Complex_Conjugate = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(npt - 2));																																			  //			
	complexArrayToZero(&H_Complex_Conjugate[0], npt - 2);																																												  	  //
	fftwf_complex *H_Complex_Final = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*(2 * (npt - 1)));																																		  //
	complexArrayToZero(&H_Complex_Final[0], 2 * (npt - 1));																																											  		  //
																																																											  //
	/*						Fourier transform of real series				   */																																							  //
	/*HH[i] * exp(rad[i]) =>	Real =  HH[i] * exp(rad[i][0]) * cos(rad[i][1])*/																																							  //
	/*							Imag =  HH[i] * exp(rad[i][0]) * sin(rad[i][1])*/																																							  //
	for (int i = 0; i < npt; i++)																																																			  //
	{																																																										  //
		H_Complex[i][0] = HH[i] * exp(0) * cos((-dt * PI * i) / (npt - 1));																																									  //
		H_Complex[i][1] = HH[i] * exp(0) * sin((-dt * PI * i) / (npt - 1));																																									  //
		H_Complex_Final[i][0] = H_Complex[i][0];																																															  //
		H_Complex_Final[i][1] = H_Complex[i][1];																																															  //
	}																																																										  //
																																																											  //
	for (int i = npt - 3; i > 1; i--)																																																		  //
	{																																																										  //
		H_Complex_Conjugate[i][0] = H_Complex[npt - 2 - i][0];																																												  //
		H_Complex_Conjugate[i][1] = H_Complex[npt - 2 - i][1];																																												  //
	}																																																										  //
																																																											  //
	for (int i = npt, j = 0; i < 2 * (npt - 1), j<(npt - 2); i++, j++)																																										  //
	{																																																										  //
		H_Complex_Final[i][0] = H_Complex_Conjugate[j][0];																																													  //
		H_Complex_Final[i][1] = -H_Complex_Conjugate[j][1];																																													  //
	}																																																										  //
																																																											  //
	FFTINFO *fftBackward = new FFTINFO(FFTW_BACKWARD, 2 * (npt - 1));																																										  //
	for (int i = 0; i < fftBackward->lengthArray; i++)
	{
		fftBackward->inputArray[i][0] = H_Complex_Final[i][0];
		fftBackward->inputArray[i][1] = H_Complex_Final[i][1];
	}
	InsermLibrary::MATLABFUNC::FFTWBackward(fftBackward);																																																				  //
																																																											  //
	for (int i = 0; i < MF2_length; i++)																																																	  //
	{																																																										  //
		FirCoeffOut[i] = fftBackward->outputArray[i][0] * MF2_Window[i];																																									  //
	}																																																										  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	delete[] MF2_Window;																		  /*||*/																																	  //
	MF2_Window = NULL;																			  /*||*/																																	  //
																								  /*||*/																																	  //
	delete[] HH;																				  /*||*/																																	  //
	HH = NULL;																					  /*||*/																																	  //
																								  /*||*/																																	  //
	fftwf_free(H_Complex);																		  /*||*/																																	  //
	fftwf_free(H_Complex_Conjugate);															  /*||*/																																	  //
	fftwf_free(H_Complex_Final);																  /*||*/																																	  //
	delete fftBackward;																			  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
																																																										      //
float* InsermLibrary::FIRINFO::interpolateBreakPointsGrid(int npt, float *p_frequency, int p_frequency_length, float *p_magnitude)																									  //
{																																																											  //
	int *j_tab = nullptr;																																																					  //
	float df[5]{};																																																							  //
	float *inc = nullptr, *HH = nullptr;																																																	  //
	int lap = 0, nbrk = 0, nint = 0;																																																		  //
	double nb = 0, ne = 0;																																																					  //
																																																											  //
	HH = new float[npt + 1]();																																																				  //
	lap = trunc(npt / 25);																																																					  //
	nbrk = p_frequency_length;																																																				  //
	nint = nbrk - 1;																																																						  //
																																																											  //
	for (int i = p_frequency_length - 1; i > 0; i--)																																														  //
	{																																																										  //
		df[i - 1] = p_frequency[i] - p_frequency[i - 1];																																													  //
		if (df[i - 1] < 0)																																																					  //
		{																																																									  //
			std::cout << "Invalid Frequencies, Frequencies must be non-decreasing" << std::endl;																																			  //
		}																																																									  //
	}																																																										  //
																																																											  //
	npt = npt + 1;																																																							  //
	nb = 1;																																																									  //
	HH[0] = p_magnitude[0];																																																					  //
																																																											  //
	inc = new float[npt]();																																																				  //
	j_tab = new int[npt]();																																																					  //
																																																											  //
	for (int i = 0; i < nint; i++)																																																			  //
	{																																																										  //
		if (df[i] == 0)																																																						  //
		{																																																									  //
			nb = nb - lap / 2;																																																				  //
			ne = nb + lap;																																																					  //
		}																																																									  //
		else																																																								  //
		{																																																									  //
			ne = trunc(p_frequency[i + 1] * npt);																																															  //
		}																																																									  //
																																																											  //
		if ((nb < 0) || (ne > npt))																																																			  //
		{																																																									  //
			std::cout << "Signal Error, Too abrupt an aplitude change near end of frequency interval" << std::endl;																															  //
		}																																																								   	  //
																																																											  //
		for (int j = 0, val = (int)nb - 1; (val < ne); j++, val++)																																											  //
		{																																																									  //
			j_tab[j] = val;																																																					  //
		}																																																									  //
																																																											  //
		for (int j = 0; j < ((int)(ne - nb) + 1); j++)																																														  //
		{																																																									  //
			if (nb == ne)																																																					  //
			{																																																								  //
				inc[j] = 0;																																																					  //
			}																																																								  //
			else																																																							  //
			{																																																								  //
				inc[j] = ((j_tab[j] + 1) - nb) / (ne - nb);																																													  //
			}																																																								  //
			HH[(j_tab[j])] = (inc[j] * p_magnitude[i + 1]) + ((1 - inc[j]) * p_magnitude[i]);																																				  //
		}																																																									  //
																																																											  //	
		nb = ne + 1;																																																						  //
	}																																																										  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	delete[] inc;																				  /*||*/																																	  //
	inc = NULL;																					  /*||*/																																	  //
	delete[] j_tab;																				  /*||*/																																	  //
	j_tab = NULL;																				  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
																																																											  //
	return HH;																																																								  //
}																																																											  //

void InsermLibrary::FIRINFO::complexArrayToZero(fftwf_complex *complexArray, int lengthArray)
{
	for (int i = 0; i < lengthArray; i++)
	{
		complexArray[i][0] = 0;
		complexArray[i][1] = 0;
	}
}
/**********************************************************************************************************************************************************************************************************************************************/

/*****************************************************************************************************************************************************************************************************************************************************/
/*******************************************************************************************************          MATLABFUNC Methods              ****************************************************************************************************/
/*****************************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																										Convolution																															  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::MATLABFUNC::Convolution2(float *p_source_tab, int p_source_tab_length, float *p_out_tab, int p_coeff)																													  //
{
	int index = p_coeff / 2;																																																				  
	double temp = 0;
	int debut, fin, taille;
	for (int i = 0; i < p_source_tab_length; i++)
	{
		temp = 0;
		if (i - index <= 0)
		{
			taille = i + index;
			debut = 0;
			fin = taille;
			//std::cout << "0" << std::endl;
		}
		else if (i >= p_source_tab_length - index)
		{
			taille = p_source_tab_length - (i-index);
			debut = i - index + 1;
			fin = p_source_tab_length - 1;
			//std::cout << "1" << std::endl;
		}
		else 
		{
			taille = p_coeff;
			debut = i - (index - 1);
			fin = i + index;
			//std::cout << "2" << std::endl;
		}

		//std::cout << "taile : " << taille << std::endl;
		//std::cout << " i : " << i << std::endl;
		//std::cout << "Debut : " << debut << std::endl;
		//std::cout << "Fin : " << fin << std::endl;
		//std::cout << std::endl;
		for (int j = debut; j <= fin; j++)
		{
			temp += p_source_tab[j] *(1.0 / p_coeff);
		}
		p_out_tab[i] = temp;
	}
	//std::cout << "end" << std::endl;
}

void InsermLibrary::MATLABFUNC::Convolution(float *p_source_tab, int p_source_tab_length, float *p_out_tab, int p_coeff)																													  //
{																																																											  //
	int index = p_coeff / 2;																																																				  //
	float temp = 0;																																																						  //
																																																											  //
	for (int i = 0; i < p_source_tab_length; i++)																																															  //
	{																																																										  //
		temp = 0;																																																							  //
		if (i - index < 0)																																																					  //
		{																																																									  //
			for (int j = 0; j <= index + i; j++)																																															  //
			{																																																								  //
				temp += p_source_tab[j];																																																	  //
			}																																																								  //
		}																																																									  //
		else if (i + index >= p_source_tab_length - 1)																																														  //
		{																																																									  //
			for (int j = (i - index) + 1; j < p_source_tab_length; j++)																																										  //
			{																																																								  //
				temp += p_source_tab[j];																																																	  //
			}																																																								  //
		}																																																									  //
		else																																																								  //
		{																																																									  //
			for (int j = i - (index - 1); j < index + i + 1; j++)																																											  //
			{																																																								  //
				temp += p_source_tab[j];																																																	  //
			}																																																								  //
		}																																																									  //
		p_out_tab[i] = temp * (1.0 / p_coeff);																																																  //
		//p_out_tab[i] = p_source_tab[i] * (1.0 / p_coeff);
	}																																																										  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

void InsermLibrary::MATLABFUNC::bandPassHilbertFreq(float *SignIn, float *SignOut, InsermLibrary::FIRINFO *fir, InsermLibrary::FFTINFO* fftForward, InsermLibrary::FFTINFO* fftBackward)
{
	for (int i = 0; i < fftForward->lengthArray; i++)
	{
		fftForward->inputArray[i][0] = SignIn[i];
		fftForward->inputArray[i][1] = 0;
	}

	FFTWForward(fftForward);

	//============ Band Pass + Hilbert
	for (int i = 0; i < fftForward->lengthArray; i++)
	{
		fftBackward->inputArray[i][0] = fftForward->outputArray[i][0] * fir->firCoeffFreq[i] * fir->hilbertCoeff[i];
		fftBackward->inputArray[i][1] = fftForward->outputArray[i][1] * fir->firCoeffFreq[i] * fir->hilbertCoeff[i];
	}
	//============

	FFTWBackward(fftBackward);

	for (int i = 0; i < fftBackward->lengthArray; i++)
	{
		SignOut[i] = sqrt((fftBackward->outputArray[i][0] * fftBackward->outputArray[i][0]) + (fftBackward->outputArray[i][1] * fftBackward->outputArray[i][1]));
	}
}

void InsermLibrary::MATLABFUNC::bandPassHilbertFreq(vector<float> & SignIn, vector<float> & SignOut, FIRINFO *fir, FFTINFO* fftForward, FFTINFO* fftBackward)
{
	for (int i = 0; i < fftForward->lengthArray; i++)
	{
		fftForward->inputArray[i][0] = SignIn[i];
		fftForward->inputArray[i][1] = 0;
	}

	FFTWForward(fftForward);

	//============ Band Pass + Hilbert
	for (int i = 0; i < fftForward->lengthArray; i++)
	{
		fftBackward->inputArray[i][0] = fftForward->outputArray[i][0] * fir->firCoeffFreq[i] * fir->hilbertCoeff[i];
		fftBackward->inputArray[i][1] = fftForward->outputArray[i][1] * fir->firCoeffFreq[i] * fir->hilbertCoeff[i];
	}
	//============

	FFTWBackward(fftBackward);

	for (int i = 0; i < fftBackward->lengthArray; i++)
	{
		SignOut[i] = sqrt((fftBackward->outputArray[i][0] * fftBackward->outputArray[i][0]) + (fftBackward->outputArray[i][1] * fftBackward->outputArray[i][1]));
	}
}
/**********************************************************************************************************************************************************************************************************************************************/

void InsermLibrary::MATLABFUNC::hilbertEnvellope(float* xr, int xrLength, float *SignOut, FFTINFO* fftForward, FFTINFO* fftBackward)																										  //
{																																																											  //
	float *h;																																																								  //
	int SignalLength = 0;																																																					  //
																																																											  //
	SignalLength = xrLength;																																																				  //
																																																											  //
	//init h à 0																																																							  //
	h = new float[SignalLength]();																																																			  //
																																																											  //
	if ((SignalLength > 0) && (2 * trunc(SignalLength / 2) == SignalLength))																																								  //
	{																																																										  //
		//Even and Non-Empty																																																				  //
		h[0] = 1;																																																							  //
		h[SignalLength / 2] = 1;																																																			  //
		for (int i = 1; i < SignalLength / 2; i++)																																															  //
		{																																																									  //
			h[i] = 2;																																																						  //
		}																																																									  //
	}																																																										  //
	else if (SignalLength > 0)																																																				  //
	{																																																										  //
		//Odd and Non-Empty																																																					  //
		h[0] = 1;																																																							  //
		for (int i = 1; i < SignalLength / 2; i++)																																															  //
		{																																																									  //
			h[i] = 2;																																																						  //
		}																																																									  //
	}																																																										  //
																																																											  //
	for (int i = 0; i < SignalLength; i++)																																																	  //
	{																																																										  //
		fftForward->inputArray[i][0] = xr[i];																																																  //
		fftForward->inputArray[i][1] = 0;																																																	  //
	}																																																										  //
																																																											  //
	FFTWForward(fftForward);																																																				  //
																																																											  //
	for (int i = 0; i < SignalLength; i++)																																																	  //
	{																																																										  //
		fftBackward->inputArray[i][0] = h[i] * fftForward->outputArray[i][0];																																								  //
		fftBackward->inputArray[i][1] = h[i] * fftForward->outputArray[i][1];																																								  //
	}																																																										  //
																																																											  //
	FFTWBackward(fftBackward);																																																				  //
																																																											  //
	//abs(complex), évite de le faire dans envhilbert																																														  //
	for (int i = 0; i < SignalLength; i++)																																																	  //
	{																																																										  //
		SignOut[i] = sqrt((fftBackward->outputArray[i][0] * fftBackward->outputArray[i][0]) + (fftBackward->outputArray[i][1] * fftBackward->outputArray[i][1]));																			  //
	}																																																										  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	delete[] h;																					  /*||*/																																	  //
	h = NULL;																					  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																						The passbands (Fp1 Fp2) frequencies are defined in Hz as																							  */
/*																						                ----------                                      																					  */
/*																						  	          /|         | \                                    																					  */
/*																									 / |         |  \                                   																					  */
/*																								    /  |         |   \                                  																					  */
/*																								   /   |         |    \                                 																					  */
/*																								  /    |         |     \                                  																					  */
/*																				   ---------------	   |         |      ----------------																									  */
/*														 							             Fs1  Fp1       Fp2   Fs2                               																					  */
/*																																                                                      														  */
/*																									DEFAULTS values                                     																					  */
/*																								  Fs1 = Fp1 - 0.5 in Hz                               																						  */
/*																								  Fs2 = Fp2 + 0.5 in Hz                               																						  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::MATLABFUNC::bandPassFilter(float *SignIn, int SignInLength, float *FirCoeff, float * SignOut, FFTINFO* fftForward, FFTINFO* fftBackward)																				  //
{																																																											  //
	float *H = nullptr;																																																					  //
																																																											  //
	//Make zero-phase filter function																																																		  //
	for (int i = 0; i < SignInLength; i++)																																																	  //
	{																																																										  //
		fftForward->inputArray[i][0] = FirCoeff[i];																																															  //
		fftForward->inputArray[i][1] = 0;																																																	  //
	}																																																										  //
																																																											  //
	FFTWForward(fftForward);																																																				  //
																																																											  //
	H = new float[SignInLength]();																																																			  //
	for (int i = 0; i < SignInLength; i++)																																																	  //
	{																																																										  //
		H[i] = sqrt((fftForward->outputArray[i][0] * fftForward->outputArray[i][0]) + (fftForward->outputArray[i][1] * fftForward->outputArray[i][1]));																						  //
	}																																																										  //
																																																											  //
	//[-------------------------------------------------]																																													  //
	for (int i = 0; i < SignInLength; i++)																																																	  //
	{																																																										  //
		fftForward->inputArray[i][0] = SignIn[i];																																															  //
		fftForward->inputArray[i][1] = 0;																																																	  //
	}																																																										  //
																																																											  //
	FFTWForward(fftForward);																																																				  //
																																																											  //
	//[-------------------------------------------------]																																													  //
	for (int i = 0; i < SignInLength; i++)																																																	  //
	{																																																										  //
		fftBackward->inputArray[i][0] = fftForward->outputArray[i][0] * H[i];																																								  //
		fftBackward->inputArray[i][1] = fftForward->outputArray[i][1] * H[i];																																								  //
	}																																																										  //
																																																											  //
	//[-------------------------------------------------]																																													  //
	FFTWBackward(fftBackward);																																																				  //
																																																											  //
	for (int i = 0; i < SignInLength; i++)																																																	  //
	{																																																										  //
		SignOut[i] = fftBackward->outputArray[i][0];																																														  //
	}																																																										  //
																																																											  //
	/****************************************** Free Memory *******************************************/																																	  //
	delete[] H;																					  /*||*/																																	  //
	H = NULL;																					  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																			in case of Multithreaded FFT add this around the main call of FFT																								  */
/**********************************************************************************************************************************************************************************************************************************************/
/*Pour FFTWW Multithread*/																																																					  //
//fftw_init_threads();																																																						  //
//fftw_plan_with_nthreads(3);																																																				  //
///*Exec*/																																																									  //
//fftw_cleanup_threads();																																																					  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																																																											  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::MATLABFUNC::FFTWForward(FFTINFO* fftForward)																																											  //
{																																																									 		  //
	/****************************************** Launch FFTW *******************************************/																																	  //
	fftwf_execute(fftForward->plan);															  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																				FFT Back : FFTW Output has been Normalised by (1/Length Of FFT)																								  */
/**********************************************************************************************************************************************************************************************************************************************/
void InsermLibrary::MATLABFUNC::FFTWBackward(FFTINFO* fftBackward)																																											  //
{																																																											  //
	float fftBackCoeff = 0;																																																					  //
	/****************************************** Launch FFTW *******************************************/																																	  //
	fftwf_execute(fftBackward->plan);															  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
																																																											  //
	/****************************** Normalise FFT Output by 1/Length FFT ******************************/																																      //
	fftBackCoeff = ((float)1 / fftBackward->lengthArray);										  /*||*/																																	  //
	for (int i = 0; i < fftBackward->lengthArray; i++)											  /*||*/																																	  //
	{																							  /*||*/																																	  //
		fftBackward->outputArray[i][0] = fftBackward->outputArray[i][0] * fftBackCoeff;			  /*||*/																																	  //
		fftBackward->outputArray[i][1] = fftBackward->outputArray[i][1] * fftBackCoeff;		      /*||*/																																	  //
	}		 																					  /*||*/																																	  //
	/**************************************************************************************************/																																	  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																											Hamming																															  */
/*																						Returns an L-point symmetric Hamming window																											  */
/*																																																											  */
/*																						(int) length : Length of the Window returned																										  */
/**********************************************************************************************************************************************************************************************************************************************/
float* InsermLibrary::MATLABFUNC::Hamming(int length)																																														  //
{																																																											  //
	float *HWindow = new float[length];																																																		  //
	for (int i = 0; i < length; i++)																																																		  //
	{																																																										  //
		HWindow[i] = 0.54 - 0.46 * cos(2 * PI * i / (length));																																												  //
	}																																																										  //
	return HWindow;																																																							  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************************************************************************/
/*																											Hanning																															  */
/*																						Returns an L-point symmetric Hanning window																											  */
/*																																																											  */
/*																						(int) length : Length of the Window returned																										  */
/**********************************************************************************************************************************************************************************************************************************************/
float* InsermLibrary::MATLABFUNC::Hanning(int length)																																														  //
{																																																											  //
	float *HWindow = new float[length];																																																		  //
	for (int i = 0; i < length; i++)																																																		  //
	{																																																										  //
		HWindow[i] = 0.5 * (1 - cos(2 * PI * i / (length - 1)));																																											  //
	}																																																										  //
	return HWindow;																																																							  //
}																																																											  //
/**********************************************************************************************************************************************************************************************************************************************/


/**********************************************************************************************************************************************************************************************************************************************/
/*																											BlackmanHarris																													  */
/*																						Returns an N-point symmetric 4-term Blackman-Harris window																							  */
/*																																																											  */
/*																						(int) length : Length of the Window returned																										  */
/**********************************************************************************************************************************************************************************************************************************************/
float* InsermLibrary::MATLABFUNC::BlackmanHarris(int length)																																												  //
{																																																											  //
	float *BHWindow = new float[length];																																																	  //
																																																											  //
	for (int i = 0; i < length; i++)																																																		  //
	{																																																										  //
		BHWindow[i] = 0.35875 - 0.48829 * cos(2 * PI * i / (length - 1)) + 0.14128 * cos(4 * PI * i / (length - 1)) - 0.01168 * cos(6 * PI * i / (length - 1));																				  //
	}																																																										  //
																																																											  //
	return BHWindow;																																																						  //
}																																																											  //	
/**********************************************************************************************************************************************************************************************************************************************/

/****************************************************************/
/*	Standard Derivation => mean then sqrt(sum((data-mean)²))	*/
/****************************************************************/

/****************************************************************/
/*	Standard Derivation => mean then sqrt(sum((data-mean)²))	*/
/****************************************************************/
float InsermLibrary::MATLABFUNC::stdMean(vec2<float> eegDataChanel, int windowSam[2])
{
	int nbTrigg = eegDataChanel.size();
	int nbSampleWin = windowSam[1] - windowSam[0];
	vec1<float> erp = vec1<float>(nbTrigg);
	vec1<float> stdDeviation = vec1<float>(nbTrigg);

	for (int i = 0; i < nbTrigg; i++)
	{
		double tempErp = 0;
		for (int m = 0; m < nbSampleWin; m++)
		{
			tempErp += eegDataChanel[i][m];
		}
		erp[i] = tempErp / nbSampleWin;
	}

	for (int i = 0; i < nbTrigg; i++)
	{
		double tempStd = 0;
		for (int m = 0; m < nbSampleWin; m++)
		{
			tempStd += (eegDataChanel[i][m] - erp[i]) * (eegDataChanel[i][m] - erp[i]);
		}
		stdDeviation[i] = sqrt(tempStd / (nbSampleWin - 1));
	}

	double tempstd = 0;
	for (int k = 0; k < nbTrigg; k++)
	{
		tempstd += stdDeviation[k];
	}
	tempstd /= nbTrigg;

	return tempstd;
}
