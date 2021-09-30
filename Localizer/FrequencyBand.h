#pragma once

#include <iostream>
#include <vector>

namespace InsermLibrary
{
	class FrequencyBand
	{
	public:
		FrequencyBand() {};
		FrequencyBand(const std::string& bandName, int fMin, int fMax, int step);	
		inline const std::string Label() const
		{
			return m_name;
		}
        inline int FMax() const
		{
			return m_bins[m_bins.size() - 1];
		}
        inline int Step() const
		{
			return m_bins[1] - m_bins[0];
		}
        inline int FMin() const
		{
			return m_bins[0];
		}
		inline const std::vector<int>& FrequencyBins() const
		{
			return m_bins;
		}
		void CheckShannonCompliance(int samplingFrequency);

	private:
		std::string m_name;
		std::vector<int> m_bins;
	};
}
