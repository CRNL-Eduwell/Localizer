#include "FrequencyBand.h"

InsermLibrary::FrequencyBand::FrequencyBand(const std::string& bandName, int fMin, int fMax, int step)
{
	m_name = bandName;

	for (int i = 0; i <= ((fMax - fMin) / step); i++)
	{
		m_bins.push_back(fMin + (i * step));
	}
}

void InsermLibrary::FrequencyBand::CheckShannonCompliance(int samplingFrequency)
{
	if (!(samplingFrequency > (2 * FMax())))
	{
		int fMin = FMin();
		int step = Step();
		int fMax = ((samplingFrequency / 2) / step) * step;

		m_bins.clear();
		for (int i = 0; i <= ((fMax - fMin) / step); i++)
			m_bins.push_back(fMin + (i * step));
	}
}

