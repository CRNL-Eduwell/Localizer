#ifndef FILEHEALTHCHECKERWINDOW_H
#define FILEHEALTHCHECKERWINDOW_H

#include "ui_FileHealthCheckerWindow.h"
#include <QtWidgets/QDialog>
#include "SubjectFolder.h"
#include <iostream>
#include <vector>

class FileHealthCheckerWindow : public QDialog
{
    Q_OBJECT

public:
    FileHealthCheckerWindow(SubjectFolder& filesystem, QWidget *parent = nullptr);
    FileHealthCheckerWindow(std::vector<SubjectFolder*> &filesystems, QWidget *parent = nullptr);
    ~FileHealthCheckerWindow();

private:
    std::vector<std::pair<std::string, std::string>> ParseFileSystemData();
    std::vector<std::pair<std::string, std::string>> ParseFileSystemsData();
    void PrintErrorMessages(std::vector<std::pair<std::string, std::string>> messages);
    void RemoveIssuesAndValidate();

private:
    Ui::FileHealthCheckerWindow ui;
    SubjectFolder *m_fileSystem = nullptr;
    std::vector<SubjectFolder*> m_fileSystems;
};
#endif // FILEHEALTHCHECKERWINDOW_H
