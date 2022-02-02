#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include "json.hpp"

namespace InsermLibrary
{
    class ProvFile
    {
    public:
        ProvFile(const std::string& filePath);
        ~ProvFile();
    //    inline const std::vector<FrequencyBand>& FrequencyBands()
    //    {
    //        return m_frequencyBands;
    //    }
        void Load(const std::string& filePath);
        void Save();

    private :
        void FillProtocolInformations(nlohmann::json jsonObject);

    //private:
        //std::vector<FrequencyBand> m_frequencyBands;
    };
}
