#ifndef BIDSSUBJECT_H
#define BIDSSUBJECT_H

#include <iostream>
#include <vector>
#include "IEegFileInfo.h"
#include "BrainvisionFileInfo.h"
//#include "EdfFileInfo.h" , later since bids might also be edf

class BidsSubject
{

public:
    BidsSubject();
    BidsSubject(std::string root, std::vector<std::string> tasks, std::vector<InsermLibrary::BrainVisionFileInfo> fileInfos);
    ~BidsSubject();

    inline std::string RootFolder() { return m_rootFolder; }
    inline std::vector<std::string>& Tasks() { return m_tasks; }
    inline InsermLibrary::IEegFileInfo* FileInfo(int i) { return &m_fileInfos[i]; }

    void DeleteTask(std::string label);

private:
    std::string m_rootFolder = "";
    std::vector<std::string> m_tasks;
    std::vector<InsermLibrary::BrainVisionFileInfo> m_fileInfos;
};

#endif // BIDSALYZER_H
