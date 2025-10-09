#include "BidsSubject.h"
#include <QDebug>
#include <algorithm>

BidsSubject::BidsSubject()
{

}

BidsSubject::BidsSubject(std::string root, std::vector<std::string> tasks, std::vector<std::unique_ptr<InsermLibrary::IEegFileInfo>> fileInfos)
{
    m_rootFolder = root;
    m_tasks = std::vector<std::string>(tasks);
    m_fileInfos = std::move(fileInfos);
}

BidsSubject::BidsSubject(const BidsSubject& other)
{
    copyFrom(other);
}

BidsSubject& BidsSubject::operator=(const BidsSubject& other)
{
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

void BidsSubject::copyFrom(const BidsSubject& other)
{
    m_rootFolder = other.m_rootFolder;
    m_tasks = other.m_tasks;
    
    // Deep copy of the file infos
    m_fileInfos.clear();
    m_fileInfos.reserve(other.m_fileInfos.size());
    
    for (const auto& fileInfo : other.m_fileInfos) {
        if (fileInfo) {
            // Clone based on the actual type
            InsermLibrary::FileType type = fileInfo->GetFileType();
            switch (type) {
                case InsermLibrary::FileType::Brainvision: {
                    auto* bvInfo = dynamic_cast<InsermLibrary::BrainVisionFileInfo*>(fileInfo.get());
                    if (bvInfo) {
                        m_fileInfos.push_back(std::make_unique<InsermLibrary::BrainVisionFileInfo>(*bvInfo));
                    }
                    break;
                }
                case InsermLibrary::FileType::EuropeanDataFormat: {
                    auto* edfInfo = dynamic_cast<InsermLibrary::EdfFileInfo*>(fileInfo.get());
                    if (edfInfo) {
                        m_fileInfos.push_back(std::make_unique<InsermLibrary::EdfFileInfo>(*edfInfo));
                    }
                    break;
                }
                default:
                    // Handle other types if needed in the future
                    break;
            }
        }
    }
}

BidsSubject::~BidsSubject()
{

}

void BidsSubject::DeleteTask(std::string label)
{
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [&](const std::string& t) { return t == label; });
    int indexPos = std::distance(m_tasks.begin(), it);
    if(indexPos >= 0 && indexPos < static_cast<int>(m_tasks.size()))
    {
        m_tasks.erase(m_tasks.begin() + indexPos);
        m_fileInfos.erase(m_fileInfos.begin() + indexPos);
    }
    else
    {
        qDebug() << "Error trying to remove " << label.c_str();
    }
}
