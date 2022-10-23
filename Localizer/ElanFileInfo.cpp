#include "ElanFileInfo.h"
#include <filesystem>

InsermLibrary::ElanFileInfo::ElanFileInfo()
{

}

InsermLibrary::ElanFileInfo::ElanFileInfo(std::string eeg, std::string pos, std::string notes)
{
    m_Eeg = eeg;
    m_Pos = pos;
    m_Notes = notes;
}

InsermLibrary::ElanFileInfo::~ElanFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::ElanFileInfo::GetFileType()
{
    return InsermLibrary::FileType::Elan;
}

std::vector<std::string> InsermLibrary::ElanFileInfo::GetFiles()
{
    std::vector<std::string> files {Ent(), Eeg(), Notes(), Pos() };
    return files;
}

int InsermLibrary::ElanFileInfo::CheckForErrors()
{
    std::filesystem::path fseegpath(Eeg());
    std::filesystem::path fsentpath(Ent());

    if(fseegpath.empty() || fseegpath.empty())
    {
        return -3;
    }
    else if((fseegpath.extension() != ".eeg") && (fseegpath.extension() != ".eeg.ent"))
    {
        return -2;
    }
    else if(!std::filesystem::exists(Eeg()) || !std::filesystem::exists(Ent()))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
