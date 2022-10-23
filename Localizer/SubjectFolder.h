#ifndef SUBJECTFOLDER_H
#define SUBJECTFOLDER_H

#include <iostream>
#include "ExperimentFolder.h"

class SubjectFolder
{
public:
    SubjectFolder(std::string path);
    ~SubjectFolder();
    inline const std::string Path() { return m_Path; }
    inline const std::string SubjectLabel() { return m_FolderName; }
    inline bool IsValid()
    {
        bool hasExperimentFolder = m_ExperimentFolders.size() > 0;
        if(!hasExperimentFolder) return false;
        bool hasValidExperiments = false;
        for(int i = 0; i < m_ExperimentFolders.size(); i++)
        {
            hasValidExperiments |= m_ExperimentFolders[i].IsValid();
        }
        return hasValidExperiments;
    }

private :
    void GetExperimentFolder(std::string path);

private:
    std::string m_Path = "";
    std::string m_FolderName = "";
    std::vector<ExperimentFolder> m_ExperimentFolders;
};



#endif // SUBJECTFOLDER_H
