#ifndef _BIQUADFILTER_H
#define _BIQUADFILTER_H
#define PI 3.14159265

#include <iostream>
#include <vector>
#include <complex>
#include "MathOperations.h"

namespace Filters
{
	enum FilterType { LowPass = 0, HighPass = 1, BandPass = 2, Peak = 3, Notch = 4, LowShelf = 5, HighShelf = 6 };

	class BiquadFilter
	{
	public:
		BiquadFilter(float centerFrequency, float samplingRate, FilterType type, float Q, float gainDB = 0);
		~BiquadFilter();
		inline std::vector<float> A() const { return m_A_Coefficients; };
		inline std::vector<float> B() const { return m_B_Coefficients; };

		void CombineFilters(BiquadFilter filterToAdd);
		float* Filter(float* signal, int nbPoints);
		std::vector<float> Filter(float* signal, int nbPoints, int d);
		float* FilterAmplitudeResponse();
		float* FilterDecibelResponse();

	private:
		void CalculateCoefficients(float centerFrequency);
		float AmplitudeResponse(float f);
		float DecibelResponse(float f);
		std::complex<float> CoeffsEvaluation(std::vector<float> & coefficients, float omega);

	private:
		std::vector<float> m_A_Coefficients, m_B_Coefficients;
		float m_centerFrequency;
		float m_absoluteGain;
		float m_samplingRate;
		float m_Q;
		float m_gainDB;
		FilterType m_type;
	};
}

#endif