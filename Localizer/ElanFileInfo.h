#ifndef ELANFILEINFO_H
#define ELANFILEINFO_H

#include "IEegFileInfo.h"
#include <iostream>
#include <vector>
#include <filesystem>

namespace InsermLibrary
{
    class ElanFileInfo : public InsermLibrary::IEegFileInfo
    {
    public:
        ElanFileInfo();
        ElanFileInfo(std::string eeg, std::string pos = "", std::string notes = "");
        ~ElanFileInfo();

        inline std::string const Ent() { return m_Eeg + ".ent"; }
        inline std::string const Eeg() { return m_Eeg; }
        inline std::string const Pos() { return m_Pos; }
        inline std::string const Notes() { return m_Notes; }

        FileType GetFileType();
        //Header-Data-Events-Notes
        std::vector<std::string> GetFiles();
        int CheckForErrors();

    private:
        std::string m_Eeg = "";
        std::string m_Pos = "";
        std::string m_Notes = "";
    };
}

#endif // ELANFILEINFO_H
