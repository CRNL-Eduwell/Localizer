#include "BiquadFilter.h"

Filters::BiquadFilter::BiquadFilter(float centerFrequency, float samplingRate, FilterType type, float Q, float gainDB)
{
	m_B_Coefficients = std::vector<float>{ 0, 0, 0 };
	m_A_Coefficients = std::vector<float>{ 0, 0, 0 };

	Q = (Q == 0) ? 1e-9 : Q;
	m_type = type;
	m_samplingRate = samplingRate;
	m_Q = Q;
	m_gainDB = gainDB;
	CalculateCoefficients(centerFrequency);
}
Filters::BiquadFilter::~BiquadFilter()
{

}

/// <summary>
/// Sequential connection of two biquad filters
/// </summary>
/// <param name="filterToAdd"></param>
void Filters::BiquadFilter::CombineFilters(Filters::BiquadFilter filterToAdd)
{
	m_B_Coefficients = MathOperations::VectorConvolution(m_B_Coefficients, filterToAdd.B());
	m_A_Coefficients = MathOperations::VectorConvolution(m_A_Coefficients, filterToAdd.A());

	//for (int i = 1; i < m_A_Coefficients.size(); i++)
	//	m_A_Coefficients[i] /= m_A_Coefficients[0];
	//for (int i = 0; i < m_B_Coefficients.size(); i++)
	//	m_B_Coefficients[i] /= m_A_Coefficients[0];
	//m_A_Coefficients[0] = 1;
}
/// <summary>
/// Calculate the filtered value of an input signal according to the parameters
/// of the filter .
/// y[n] = B[0] * x[n] + B[1] * x[n-1] + B[nb] * x[n-nb] - A[1] * y[n-1] - A[na] * y[n-na];
/// nb = number of B Coefficients
/// na = number of A Coefficients
/// </summary>
/// <param name="signal">Signal to filter</param>
/// <returns>Filtered Signal</returns>
float* Filters::BiquadFilter::Filter(float* signal, int nbPoints)
{
	float* result = new float[nbPoints];
	for (int i = 0; i < nbPoints; ++i)
	{
		float tmp = 0.0;
		for (int j = 0; j < m_B_Coefficients.size(); ++j)
		{
			if (i - j < 0) continue;
			tmp += m_B_Coefficients[j] * signal[i - j];
		}
		for (int j = 1; j < m_A_Coefficients.size(); ++j)
		{
			if (i - j < 0) continue;
			tmp -= m_A_Coefficients[j] * result[i - j];
		}

		result[i] = tmp;
	}
	return result;
}
std::vector<float> Filters::BiquadFilter::Filter(float* signal, int nbPoints, int d)
{
	std::vector<float>  result = std::vector<float>(nbPoints);
	for (int i = 0; i < nbPoints; ++i)
	{
		float tmp = 0.0;
		for (int j = 0; j < m_B_Coefficients.size(); ++j)
		{
			if (i - j < 0) continue;
			tmp += m_B_Coefficients[j] * signal[i - j];
		}
		for (int j = 1; j < m_A_Coefficients.size(); ++j)
		{
			if (i - j < 0) continue;
			tmp -= m_A_Coefficients[j] * result[i - j];
		}

		result[i] = tmp;
	}
	return result;
}
/// <summary>
/// Provide the amplitude response of a filter
/// </summary>
/// <returns></returns>
float* Filters::BiquadFilter::FilterAmplitudeResponse()
{
	float* filterAmplitudeResponse = new float[m_samplingRate];
	for (int i = 0; i < m_samplingRate; i++)
	{
		filterAmplitudeResponse[i] = AmplitudeResponse(i);
	}
	return filterAmplitudeResponse;
}
/// <summary>
/// Provide the decibel response of a filter
/// </summary>
/// <returns></returns>
float* Filters::BiquadFilter::FilterDecibelResponse()
{
	float* filterDecibelResponse = new float[m_samplingRate];
	for (int i = 0; i < m_samplingRate; i++)
	{
		filterDecibelResponse[i] = DecibelResponse(i);
	}
	return filterDecibelResponse;
}

/// <summary>
/// Calculate the Biquad Coefficient according to the type of the wanted filter and the center frequency
/// </summary>
/// <param name="centerFrequency"></param>
void Filters::BiquadFilter::CalculateCoefficients(float centerFrequency)
{
	m_centerFrequency = centerFrequency;
	// m_absoluteGain is only used for peaking and shelving filters
	m_absoluteGain = powf(10, m_gainDB / 40);
	float omega = 2 * PI * m_centerFrequency / m_samplingRate;
	float sn = sin(omega);
	float cs = cos(omega);
	float alpha = sn / (2 * m_Q);
	float beta = sqrt(m_absoluteGain + m_absoluteGain);

	switch (m_type)
	{
	case Filters::FilterType::LowPass:
		m_B_Coefficients[0] = (1 - cs) / 2;
		m_B_Coefficients[1] = 1 - cs;
		m_B_Coefficients[2] = (1 - cs) / 2;
		m_A_Coefficients[0] = 1 + alpha;
		m_A_Coefficients[1] = -2 * cs;
		m_A_Coefficients[2] = 1 - alpha;
		break;
	case Filters::FilterType::HighPass:
		m_B_Coefficients[0] = (1 + cs) / 2;
		m_B_Coefficients[1] = -(1 + cs);
		m_B_Coefficients[2] = (1 + cs) / 2;
		m_A_Coefficients[0] = 1 + alpha;
		m_A_Coefficients[1] = -2 * cs;
		m_A_Coefficients[2] = 1 - alpha;
		break;
	case Filters::FilterType::BandPass:
		m_B_Coefficients[0] = alpha;
		m_B_Coefficients[1] = 0;
		m_B_Coefficients[2] = -alpha;
		m_A_Coefficients[0] = 1 + alpha;
		m_A_Coefficients[1] = -2 * cs;
		m_A_Coefficients[2] = 1 - alpha;
		break;
	case Filters::FilterType::Peak:
		m_B_Coefficients[0] = 1 + (alpha * m_absoluteGain);
		m_B_Coefficients[1] = -2 * cs;
		m_B_Coefficients[2] = 1 - (alpha * m_absoluteGain);
		m_A_Coefficients[0] = 1 + (alpha / m_absoluteGain);
		m_A_Coefficients[1] = -2 * cs;
		m_A_Coefficients[2] = 1 - (alpha / m_absoluteGain);
		break;
	case Filters::FilterType::Notch:
		m_B_Coefficients[0] = 1;
		m_B_Coefficients[1] = -2 * cs;
		m_B_Coefficients[2] = 1;
		m_A_Coefficients[0] = 1 + alpha;
		m_A_Coefficients[1] = -2 * cs;
		m_A_Coefficients[2] = 1 - alpha;
		break;
	case Filters::FilterType::LowShelf:
		m_B_Coefficients[0] = m_absoluteGain * ((m_absoluteGain + 1) - (m_absoluteGain - 1) * cs + beta * sn);
		m_B_Coefficients[1] = 2 * m_absoluteGain * ((m_absoluteGain - 1) - (m_absoluteGain + 1) * cs);
		m_B_Coefficients[2] = m_absoluteGain * ((m_absoluteGain + 1) - (m_absoluteGain - 1) * cs - beta * sn);
		m_A_Coefficients[0] = (m_absoluteGain + 1) + (m_absoluteGain - 1) * cs + beta * sn;
		m_A_Coefficients[1] = -2 * ((m_absoluteGain - 1) + (m_absoluteGain + 1) * cs);
		m_A_Coefficients[2] = (m_absoluteGain + 1) + (m_absoluteGain - 1) * cs - beta * sn;
		break;
	case Filters::FilterType::HighShelf:
		m_B_Coefficients[0] = m_absoluteGain * ((m_absoluteGain + 1) + (m_absoluteGain - 1) * cs + beta * sn);
		m_B_Coefficients[1] = -2 * m_absoluteGain * ((m_absoluteGain - 1) + (m_absoluteGain + 1) * cs);
		m_B_Coefficients[2] = m_absoluteGain * ((m_absoluteGain + 1) + (m_absoluteGain - 1) * cs - beta * sn);
		m_A_Coefficients[0] = (m_absoluteGain + 1) - (m_absoluteGain - 1) * cs + beta * sn;
		m_A_Coefficients[1] = 2 * ((m_absoluteGain - 1) - (m_absoluteGain + 1) * cs);
		m_A_Coefficients[2] = (m_absoluteGain + 1) - (m_absoluteGain - 1) * cs - beta * sn;
		break;
	}

	// prescale flter constants
	m_B_Coefficients[0] /= m_A_Coefficients[0];
	m_B_Coefficients[1] /= m_A_Coefficients[0];
	m_B_Coefficients[2] /= m_A_Coefficients[0];
	m_A_Coefficients[2] /= m_A_Coefficients[0];
	m_A_Coefficients[1] /= m_A_Coefficients[0];
	m_A_Coefficients[0] = 1;
}
/// <summary>
/// Provide an amplitude result for a given frequency
/// The transfer function of the filter can be described with
/// 
///             SUM[a(n) * exp (-n * j * w)] from n = 0 to n = Total number of A coefficient
/// H(e^(jw)) = -----------------------------
///             SUM[b(m) * exp (-m * j * w)] from m = 0 to m = Total number of B coefficient
/// 
/// The magnitude and the phase can then be obtained with
/// Magnitude = |H| = abs(H) = Sqrt(H.real² + H.imag²)
/// Phase = atan2(H.imag, H.real)
/// 
/// </summary>
/// <param name="f">Frequency beetween 0 and samplingRate / 2</param>
/// <returns>Amplitude Response</returns>
float Filters::BiquadFilter::AmplitudeResponse(float f)
{
	float omega = (f * PI) / (m_samplingRate / 2);
	std::complex<float> resZeros = CoeffsEvaluation(m_B_Coefficients, omega);
	std::complex<float> resPoles = CoeffsEvaluation(m_A_Coefficients, omega);
	std::complex<float> Hw = resZeros / resPoles;
	return sqrt((Hw.real() * Hw.real()) + (Hw.imag() * Hw.imag()));
}
/// <summary>
/// Provide a decibel result for a given frequency
/// </summary>
/// <param name="f">Frequency beetween 0 and samplingRate / 2</param>
/// <returns>Decibel Response</returns>
float Filters::BiquadFilter::DecibelResponse(float f)
{
	float r;
	try
	{
		r = 20 * log10(AmplitudeResponse(f));
	}
	catch (std::exception e)
	{
		r = -100;
	}
	if (isinf(r) || isnan(r))
	{
		r = -100;
	}
	return r;
}
/// <summary>
/// Calculate the sum of a list of coefficient to evaluate the amplitude response of a given angular frequency
/// SUM[coeff(n) * exp (-n * j * w)] from n = 0 to n = Total number of coefficient in the list
/// </summary>
/// <param name="coefficients">Coefficients of the transfer function numerator or denominator</param>
/// <param name="omega">Current angular frequency </param>
/// <returns></returns>
std::complex<float> Filters::BiquadFilter::CoeffsEvaluation(std::vector<float> & coefficients, float omega)
{
	std::complex<float> res = 0;
	for (int i = 0; i < coefficients.size(); i++)
		res += coefficients[i] * exp(std::complex<float>(0, -i * omega));
	return res;
}