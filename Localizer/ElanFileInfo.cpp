#include "ElanFileInfo.h"

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

void InsermLibrary::ElanFileInfo::CheckForErrors()
{

}
