#include "FileHealthCheckerWindow.h"

FileHealthCheckerWindow::FileHealthCheckerWindow(SubjectFolder& filesystem, QWidget *parent) : QDialog(parent)
{
    m_fileSystem = &filesystem;

    ui.setupUi(this);
    std::vector<std::pair<std::string, std::string>> errors = ParseFileSystemData();
    PrintErrorMessages(errors);

    connect(ui.IgnoreIssuesPushButton, &QPushButton::clicked, this, &FileHealthCheckerWindow::RemoveIssuesAndValidate);
    connect(ui.CancelPushButton, &QPushButton::clicked, this, [&]{ close(); });
}

FileHealthCheckerWindow::FileHealthCheckerWindow(std::vector<SubjectFolder*>& filesystems, QWidget *parent) : QDialog(parent)
{
    m_fileSystems = &filesystems;

    ui.setupUi(this);
    std::vector<std::pair<std::string, std::string>> errors = ParseFileSystemsData();
    PrintErrorMessages(errors);

    connect(ui.IgnoreIssuesPushButton, &QPushButton::clicked, this, &FileHealthCheckerWindow::RemoveIssuesAndValidate);
    connect(ui.CancelPushButton, &QPushButton::clicked, this, [&]{ close(); });
}

FileHealthCheckerWindow::~FileHealthCheckerWindow()
{

}

std::vector<std::pair<std::string, std::string>> FileHealthCheckerWindow::ParseFileSystemData()
{
    std::vector<std::pair<std::string, std::string>> errorsToDisplay;
    if(!m_fileSystem->IsValid())
    {
        std::string ref = m_fileSystem->SubjectLabel();
        std::string error = "There does not seems to be any valid raw data";
        errorsToDisplay.push_back(std::make_pair(ref, error));
    }
    else
    {
        bool firstError = true;
        for(int i = 0; i < m_fileSystem->ExperimentFolders().size(); i++)
        {
            std::vector<std::string> errorMessages = m_fileSystem->ExperimentFolders()[i].GetErrorMessages();
            if(errorMessages.size() > 0)
            {
                if(firstError)
                {
                    std::string ref = m_fileSystem->SubjectLabel();
                    std::string error = "There does not seems to be any valid raw data";
                    errorsToDisplay.push_back(std::make_pair(ref, error));
                    firstError = false;
                }

                std::string ref = "     " + m_fileSystem->ExperimentFolders()[i].ExperimentLabel();
                for(int j = 0; j < errorMessages.size(); j++)
                {
                    errorsToDisplay.push_back(std::make_pair(ref, errorMessages[j]));
                }
                errorsToDisplay.push_back(std::make_pair("", ""));
            }
        }
    }

    return errorsToDisplay;
}

std::vector<std::pair<std::string, std::string>> FileHealthCheckerWindow::ParseFileSystemsData()
{
    std::vector<std::pair<std::string, std::string>> errorsToDisplay;
    for(int k = 0; k < m_fileSystems->size(); k++)
    {
        SubjectFolder* f = (*m_fileSystems)[k];

        bool atLeastOneError = false, firstError = true;
        for(int i = 0; i < f->ExperimentFolders().size(); i++)
        {
            std::vector<std::string> errorMessages = f->ExperimentFolders()[i].GetErrorMessages();
            if(errorMessages.size() > 0)
            {
                atLeastOneError = true;

                if(firstError)
                {
                    std::string ref = f->SubjectLabel();
                    std::string error = "There does not seems to be any valid raw data";
                    errorsToDisplay.push_back(std::make_pair(ref, error));
                    firstError = false;
                }

                std::string ref = "     " + f->ExperimentFolders()[i].ExperimentLabel();
                for(int j = 0; j < errorMessages.size(); j++)
                {
                    errorsToDisplay.push_back(std::make_pair(ref, errorMessages[j]));
                }
                errorsToDisplay.push_back(std::make_pair("", ""));
            }
        }
        if(atLeastOneError)
        {
            errorsToDisplay.push_back(std::make_pair("", "--------------------------"));
            errorsToDisplay.push_back(std::make_pair("", ""));
        }
    }

    return errorsToDisplay;
}

void FileHealthCheckerWindow::PrintErrorMessages(std::vector<std::pair<std::string, std::string>> messages)
{
    for(int i = 0; i < messages.size(); i++)
    {
        QString mess = "";
        if(messages[i].first.empty())
        {
            mess = QString::fromStdString(messages[i].first) + QString::fromStdString(messages[i].second);
        }
        else
        {
            mess = QString::fromStdString(messages[i].first) + " => " + QString::fromStdString(messages[i].second);
        }
        ui.HealthTextEdit->append(mess);
    }
}

void FileHealthCheckerWindow::RemoveIssuesAndValidate()
{
    if(m_fileSystem != nullptr)
    {
        for(int i = m_fileSystem->ExperimentFolders().size() - 1; i >= 0; i--)
        {
            if(!m_fileSystem->ExperimentFolders()[i].IsValid())
            {
                m_fileSystem->ExperimentFolders().erase(m_fileSystem->ExperimentFolders().begin() + i);
            }
        }
    }
    else
    {
        for(int i = 0; i < m_fileSystems->size(); i++)
        {
            SubjectFolder* f = (*m_fileSystems)[i];
            for(int j = f->ExperimentFolders().size() - 1; j >= 0; j--)
            {
                if(!f->ExperimentFolders()[j].IsValid())
                {
                    f->ExperimentFolders().erase(f->ExperimentFolders().begin() + j);
                }
            }
        }

        for(int i = m_fileSystems->size() - 1; i >= 0; i--)
        {
            if(!(*m_fileSystems)[i]->IsValid())
            {
                m_fileSystems->erase(m_fileSystems->begin() + i);
            }
        }
    }

    done(1);
    close();
}
