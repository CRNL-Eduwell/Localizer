#include "BrainvisionFileInfo.h"

InsermLibrary::BrainVisionFileInfo::BrainVisionFileInfo(std::string bvheader)
{
    m_BvHeader = bvheader;
}

InsermLibrary::BrainVisionFileInfo::~BrainVisionFileInfo()
{

}

InsermLibrary::FileType InsermLibrary::BrainVisionFileInfo::GetFileType()
{
    return InsermLibrary::FileType::Brainvision;
}

std::vector<std::string> InsermLibrary::BrainVisionFileInfo::GetFiles()
{
    std::vector<std::string> files { Header() };
    return files;
}

void InsermLibrary::BrainVisionFileInfo::CheckForErrors()
{

}
