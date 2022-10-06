#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include "json.hpp"

#include "Bloc.h"

namespace InsermLibrary
{
    class ProvFile
    {
    public:
        ProvFile(const std::string& filePath);
        ~ProvFile();
        inline const std::string FilePath() { return m_filePath; }
        inline std::vector<Bloc>& Blocs() { return m_blocs; }

    private :
        void Load(const std::string& filePath);
        void FillProtocolInformations(nlohmann::json jsonObject);
        void Save();

    private:
        std::string m_filePath = "";
        std::vector<Bloc> m_blocs;
    };
}
