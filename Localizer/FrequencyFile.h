#pragma once

#include <iostream>
#include "ITxtFile.h"
#include "FrequencyBand.h"

namespace InsermLibrary
{
	class FrequencyFile : public ITxtFile
	{
	public:
		FrequencyFile(const std::string& filePath = "./Resources/Config/frequencyBand.txt");
		inline const std::vector<FrequencyBand>& FrequencyBands() 
		{
			return m_frequencyBands;
		}
		void Load();
		void Save();

	private:
		std::vector<FrequencyBand> m_frequencyBands;
	};
}