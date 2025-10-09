#ifndef BIDSSUBJECT_H
#define BIDSSUBJECT_H

#include <iostream>
#include <vector>
#include <memory>
#include "IEegFileInfo.h"
#include "BrainvisionFileInfo.h"
#include "EdfFileInfo.h"

class BidsSubject
{

public:
    BidsSubject();
    BidsSubject(std::string root, std::vector<std::string> tasks, std::vector<std::unique_ptr<InsermLibrary::IEegFileInfo>> fileInfos);
    BidsSubject(const BidsSubject& other); // Copy constructor
    BidsSubject& operator=(const BidsSubject& other); // Assignment operator
    ~BidsSubject();

    inline std::string RootFolder() { return m_rootFolder; }
    inline std::vector<std::string>& Tasks() { return m_tasks; }
    inline InsermLibrary::IEegFileInfo* FileInfo(int i) { return m_fileInfos[i].get(); }

    void DeleteTask(std::string label);

private:
    std::string m_rootFolder = "";
    std::vector<std::string> m_tasks;
    std::vector<std::unique_ptr<InsermLibrary::IEegFileInfo>> m_fileInfos;

    // Helper method for deep copy
    void copyFrom(const BidsSubject& other);
};

#endif // BIDSALYZER_H
