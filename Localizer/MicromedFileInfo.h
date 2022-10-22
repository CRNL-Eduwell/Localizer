#ifndef MICROMEDFILEINFO_H
#define MICROMEDFILEINFO_H

#include "IEegFileInfo.h"

namespace InsermLibrary
{
    class MicromedFileInfo : public InsermLibrary::IEegFileInfo
    {
    public:
        MicromedFileInfo();
        MicromedFileInfo(std::string path);
        ~MicromedFileInfo();

        inline std::string const TRC() { return m_TRC; }

        FileType GetFileType();
        std::vector<std::string> GetFiles();
        void CheckForErrors();

    private:
        std::string m_TRC = "";
    };
}

#endif // MICROMEDFILEINFO_H
