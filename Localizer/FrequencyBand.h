#pragma once

#include <iostream>
#include <vector>

namespace InsermLibrary
{
	class FrequencyBand
	{
	public:
		FrequencyBand(const std::string& bandName, int fMin, int fMax, int step);	
		inline const std::string Label() const
		{
			return m_name;
		}
		inline const int FMax() const
		{
			return m_fMax;
		}
		inline const int Step() const
		{
			return m_step;
		}
		inline const int FMin() const
		{
			return m_fMin;
		}
		inline const std::vector<int>& FrequencyBins() const
		{
			return m_bins;
		}
	private:
		bool IsShannonCompliant(int samplingFrequency);
	private:
		int m_fMin = 0;
		int m_fMax = 0;
		int m_step = 0;
		std::string m_name;
		std::vector<int> m_bins;
	};
}