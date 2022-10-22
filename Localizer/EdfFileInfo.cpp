#include "EdfFileInfo.h"

InsermLibrary::EdfFileInfo::EdfFileInfo()
{

}

InsermLibrary::EdfFileInfo::EdfFileInfo(std::string edf)
{
    m_edf = edf;
}

InsermLibrary::EdfFileInfo::~EdfFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::EdfFileInfo::GetFileType()
{
    return InsermLibrary::FileType::EuropeanDataFormat;
}

std::vector<std::string> InsermLibrary::EdfFileInfo::GetFiles()
{
    std::vector<std::string> files { FilePath() };
    return files;
}

void InsermLibrary::EdfFileInfo::CheckForErrors()
{

}
