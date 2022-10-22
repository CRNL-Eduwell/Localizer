#ifndef FREQUENCYFOLDER_H
#define FREQUENCYFOLDER_H

#include <iostream>
#include "ElanFileInfo.h"
#include "BrainvisionFileInfo.h"
#include "AnalysisFolder.h"

enum class SmoothingWindow { SM0, SM250, SM500, SM1000, SM2500, SM5000 };

class FrequencyFolder
{
public:
    FrequencyFolder(std::string path);
    ~FrequencyFolder();

private :
    void GetFrequencyBandFromPath(std::string path);
    void GetFrequencyEegFiles(std::string path);
    void GetDataFolders(std::string path);

private:
    std::string m_Path = "";
    std::string m_FolderName = "";
    std::string m_FrequencyBandLabel = "";
    std::vector<std::pair<int, InsermLibrary::ElanFileInfo>> m_ElanSmoothingXFiles;
    std::vector<std::pair<int, InsermLibrary::BrainVisionFileInfo>> m_BvSmoothingXFiles;
    std::vector<AnalysisFolder> m_DataFolder;
};



#endif // FREQUENCYFOLDER_H
