#include "SubjectFolder.h"
#include "Utility.h"
#include <QDir>

SubjectFolder::SubjectFolder(std::string path)
{
    std::vector<std::string> splitPath = InsermLibrary::split<std::string>(path, "\\/");
    if (path[0] == '/' && path[1] == '/') //In case this is a newtork ressource
    {
        m_Path = "//";
    }
    else
    {
        if (path[0] == '/') //In case this is a unix path
            m_Path = "/";
    }

    for (int i = 0; i < splitPath.size(); i++)
        m_Path += splitPath[i] + "/";

    m_FolderName = splitPath[splitPath.size() - 1];
    GetExperimentFolder(m_Path);
}

SubjectFolder::~SubjectFolder()
{

}

void SubjectFolder::GetExperimentFolder(std::string path)
{
    //Get every folder corresponding to one LOCALIZER exam
    QDir currentDir(path.c_str());
    currentDir.setFilter(QDir::Dirs);

    QStringList entries = currentDir.entryList();
    for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
    {
        QString dirname = *entry;
        if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
        {
            m_ExperimentFolders.push_back(ExperimentFolder(path + dirname.toStdString() + "/"));
        }
    }
}
