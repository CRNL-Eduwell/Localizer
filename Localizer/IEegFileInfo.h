#ifndef IEEGFILEINFO_H
#define IEEGFILEINFO_H

#include <iostream>
#include <vector>

namespace InsermLibrary
{
    enum class FileType { UNKNOWN, Micromed, Elan, Brainvision, EuropeanDataFormat };

    class IEegFileInfo
    {
    public:
        virtual ~IEegFileInfo() = 0;
        
        virtual FileType GetFileType() = 0;
        virtual std::vector<std::string> GetFiles() = 0;
        virtual void CheckForErrors() = 0;
    };
}

#endif // IEEGFILEINFO_H
