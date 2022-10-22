#include "MicromedFileInfo.h"

InsermLibrary::MicromedFileInfo::MicromedFileInfo()
{

}

InsermLibrary::MicromedFileInfo::MicromedFileInfo(std::string path)
{
    m_TRC = path;
}

InsermLibrary::MicromedFileInfo::~MicromedFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::MicromedFileInfo::GetFileType()
{
    return InsermLibrary::FileType::Micromed;
}

std::vector<std::string> InsermLibrary::MicromedFileInfo::GetFiles()
{
    std::vector<std::string> files { m_TRC };
    return files;
}

void InsermLibrary::MicromedFileInfo::CheckForErrors()
{

}
