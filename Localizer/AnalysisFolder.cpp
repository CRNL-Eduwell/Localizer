#include "AnalysisFolder.h"
#include "Utility.h"

AnalysisFolder::AnalysisFolder(std::string path)
{
    m_Path = path;
    GetStatsInfoFromPath(path);
}

AnalysisFolder::~AnalysisFolder()
{

}

void AnalysisFolder::GetStatsInfoFromPath(std::string path)
{
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(path, "/");
    std::vector<std::string> splitFolder = InsermLibrary::split<std::string>(splitPath[splitPath.size() - 1], "_");

    if (splitFolder[5] == "trials")
    {
        m_MapType = StringToMapType(splitFolder[5]);

        if (splitFolder.size() - 1 == 6)
        {
            m_MapType = StringToMapType(splitFolder[5] + "_" + splitFolder[6]);
        }

        if (splitFolder.size() - 1 == 7)
        {
            m_MapType = StringToMapType(splitFolder[5] + "_" + splitFolder[6]);
            if (splitFolder[7] != "")
                m_StatAnalysis = splitFolder[7];
        }
    }
    else if (splitFolder[5] == "bar")
    {
        m_MapType = StringToMapType(splitFolder[5]);

        if (splitFolder.size() - 1 == 6)
        {
            if (splitFolder[6] != "")
            {
                m_StatAnalysis = splitFolder[6];
            }
        }
    }
    else if (splitFolder[5] == "plots")
    {
        m_MapType = StringToMapType(splitFolder[5]);
    }
}
