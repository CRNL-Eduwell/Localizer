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

std::string InsermLibrary::MicromedFileInfo::GetFilesString()
{
    return m_TRC;
}

int InsermLibrary::MicromedFileInfo::CheckForErrors()
{
    std::filesystem::path fspath(m_TRC);

    if(fspath.empty())
    {
        return -3;
    }
    else if((fspath.extension() != ".trc") && (fspath.extension() != ".TRC"))
    {
        return -2;
    }
    else if(!std::filesystem::exists(m_TRC))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
