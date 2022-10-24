#ifndef EXPERIMENTFOLDER_H
#define EXPERIMENTFOLDER_H

#include <iostream>

#include "MicromedFileInfo.h"
#include "ElanFileInfo.h"
#include "BrainvisionFileInfo.h"
#include "EdfFileInfo.h"
#include "FrequencyFolder.h"

class ExperimentFolder
{
public:
    ExperimentFolder(std::string path);
    ~ExperimentFolder();
    inline bool IsValid() { return m_MicromedFileInfo.CheckForErrors() == 0 || m_ElanFileInfo.CheckForErrors() == 0 || m_BvFileInfo.CheckForErrors() == 0 || m_EdfFileInfo.CheckForErrors() == 0; }
    inline const std::string Path() { return m_Path; }
    inline const std::string ExperimentLabel() { return m_ExperimentLabel; }
    InsermLibrary::IEegFileInfo* GetEegFileInfo(InsermLibrary::FileType fileType);
    inline std::vector<FrequencyFolder>& FrequencyFolders() { return m_FrequencyFolders; }

private:
    void GetExperimentNameFromPath(std::string path);
    void GetEegFiles(std::string path);
    void GetFrequencyFolders(std::string path);

private:
    std::string m_Path = "";
    std::string m_FolderName = "";
    std::string m_ExperimentLabel = "";

    InsermLibrary::MicromedFileInfo m_MicromedFileInfo;
    InsermLibrary::ElanFileInfo m_ElanFileInfo;
    InsermLibrary::BrainVisionFileInfo m_BvFileInfo;
    InsermLibrary::EdfFileInfo m_EdfFileInfo;
    std::string m_DsPosFile = "";
    std::vector<FrequencyFolder> m_FrequencyFolders;
};

#endif // EXPERIMENTFOLDER_H
