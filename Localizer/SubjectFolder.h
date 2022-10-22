#ifndef SUBJECTFOLDER_H
#define SUBJECTFOLDER_H

#include <iostream>
#include "ExperimentFolder.h"

class SubjectFolder
{
public:
    SubjectFolder(std::string path);
    ~SubjectFolder();

private :
//    void GetFrequencyBandFromPath(std::string path);
    void GetExperimentFolder(std::string path);

private:
    std::string m_Path = "";
    std::string m_FolderName = "";
    std::vector<ExperimentFolder> m_ExperimentFolders;
};



#endif // SUBJECTFOLDER_H
