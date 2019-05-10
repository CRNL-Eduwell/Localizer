#include "FrequencyBand.h"

InsermLibrary::FrequencyBand::FrequencyBand(const std::string& bandName, int fMin, int fMax, int step)
{
	m_name = bandName;
	m_fMin = fMin;
	m_fMax = fMax;
	m_step = step;

	for (int i = 0; i <= ((m_fMax - m_fMin) / m_step); i++)
	{
		m_bins.push_back(m_fMin + (i * m_step));
	}
}

bool InsermLibrary::FrequencyBand::IsShannonCompliant(int samplingFrequency)
{
	int BandCount = m_bins.size();
	if (m_bins[BandCount - 1] > (samplingFrequency / 2))
		return true;
	else
		return false;
}

