#include "localizer.h"

Localizer::Localizer(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    ReSetupGUI();
    ConnectSignals();

    inputArguments = QCoreApplication::arguments();
    if (inputArguments.count() > 1)
    {
        if (inputArguments[1].compare("MicromedExternalCall") == 0)
        {
            QString fileName = inputArguments[2];
            if (fileName != "")
            {
                m_isPatFolder = true;
                QString rootFolderPath = QDir(fileName).absolutePath();
                LoadTreeViewFolder(rootFolderPath);
            }
        }
    }
}

Localizer::~Localizer()
{

}

void Localizer::ReSetupGUI()
{
    m_GeneralOptionsFile = new InsermLibrary::GeneralOptionsFile();
	m_GeneralOptionsFile->Load();
    m_frequencyFile = new InsermLibrary::FrequencyFile();
    m_frequencyFile->Load();
	//==
    LoadFrequencyBandsUI(m_frequencyFile->FrequencyBands());
	//==
    optPerf = new optionsPerf();
    optStat = new optionsStats();
    picOpt = new picOptions();
    //optLoca = new ProtocolWindow();

    ui.progressBar->reset();
}

void Localizer::LoadFrequencyBandsUI(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands)
{
    ui.FrequencyListWidget->clear();
    for (size_t i = 0; i < FrequencyBands.size(); i++)
    {
        QString Label = QString::fromStdString(FrequencyBands[i].Label());

        QListWidgetItem *currentBand = new QListWidgetItem(ui.FrequencyListWidget);
        currentBand->setText(Label);
        currentBand->setFlags(currentBand->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        currentBand->setCheckState(Qt::Unchecked); // AND initialize check state
    }
}

void Localizer::ResetUiCheckboxes()
{
    ui.Env2plotCheckBox->setEnabled(true);
    ui.TrialmatCheckBox->setEnabled(true);
    ui.CorrelationMapsCheckBox->setEnabled(true);
    ui.StatFileExportCheckBox->setEnabled(true);
}

void Localizer::DeactivateUIForSingleFiles()
{
    ui.Env2plotCheckBox->setEnabled(false);
    ui.Env2plotCheckBox->setChecked(false);
    ui.TrialmatCheckBox->setEnabled(false);
    ui.TrialmatCheckBox->setChecked(false);
    ui.CorrelationMapsCheckBox->setEnabled(false);
    ui.CorrelationMapsCheckBox->setChecked(false);
    ui.StatFileExportCheckBox->setEnabled(false);
    ui.StatFileExportCheckBox->setChecked(false);
}

void Localizer::ConnectSignals()
{
    ConnectMenuBar();

    ui.FileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.FileTreeView, &QTreeView::customContextMenuRequested, this, &Localizer::ShowFileTreeContextMenu);
    connect(ui.FileTreeView, &DeselectableTreeView::ResetNbFolder, this, [&]() { SetLabelCount(0); });
    
    connect(ui.BrowsePtsButton, &QPushButton::clicked, this, &Localizer::SelectPtsForCorrelation);
    connect(ui.ClearPtsButton, &QPushButton::clicked, this, &Localizer::ClearPtsForCorrelation);

    connect(ui.AllBandsCheckBox, &QCheckBox::clicked, this, &Localizer::ToggleAllBands);
    connect(ui.cancelButton, &QPushButton::clicked, this, &Localizer::CancelAnalysis);
}

void Localizer::ConnectMenuBar()
{
    //===Fichier
    QAction* openPatFolder = ui.menuFiles->actions().at(0);
    connect(openPatFolder, &QAction::triggered, this, [&] { LoadPatientFolder(); });
    QAction* openSpecificFolder = ui.menuFiles->actions().at(1);
    connect(openSpecificFolder, &QAction::triggered, this, [&] { LoadSpecificFolder(); });
    QAction* openDatabaseFolder = ui.menuFiles->actions().at(2);
    connect(openDatabaseFolder, &QAction::triggered, this, [&] { LoadDatabaseFolder(); });
    //===Configuration
    QAction* openPerfMenu = ui.menuConfiguration->actions().at(0);
    connect(openPerfMenu, &QAction::triggered, this, [&] { optPerf->exec(); });
    QAction* openStatMenu = ui.menuConfiguration->actions().at(1);
    connect(openStatMenu, &QAction::triggered, this, [&] { optStat->exec(); });
    QAction* openPicMenu = ui.menuConfiguration->actions().at(2);
    connect(openPicMenu, &QAction::triggered, this, [&] { picOpt->exec(); });
    QAction* openLocaMenu = ui.menuConfiguration->actions().at(3);
    connect(openLocaMenu, &QAction::triggered, this, [&] { optLoca->exec(); });
	QAction* openFilePriorityMenu = ui.menuConfiguration->actions().at(4);
	connect(openFilePriorityMenu, &QAction::triggered, this, [&] 
	{ 	
		generalOptionsWindow = new GeneralOptionsWindow(m_GeneralOptionsFile);
		generalOptionsWindow->setAttribute(Qt::WA_DeleteOnClose);
		generalOptionsWindow->show(); 
	});
    //===Aide
    QAction* openAbout = ui.menuHelp->actions().at(0);
    connect(openAbout, &QAction::triggered, this, [&]
    {
        AboutDycog about;
        about.exec();
    });
}

void Localizer::LoadPatientFolder()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setOption(QFileDialog::ShowDirsOnly, true);
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose Patient Folder"));
    if (fileName != "")
    {
        m_isPatFolder = true;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewFolder(rootFolderPath);
        ClearPtsForCorrelation();
    }
}

void Localizer::LoadSpecificFolder()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setFileMode(QFileDialog::FileMode::AnyFile);
    fileDial->setNameFilters(QStringList()<<"*.trc"<<" *.eeg"<<" *.edf");
    QString fileName = fileDial->getExistingDirectory(this,  tr("Choose folder with generic eeg files : "), tr("C:\\"));
    if (fileName != "")
    {
        m_isPatFolder = false;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewFiles(rootFolderPath);
    }
}

void Localizer::LoadDatabaseFolder()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setOption(QFileDialog::ShowDirsOnly, true);
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose a folder containing multiple Patients"));
    if (fileName != "")
    {
        m_isPatFolder = true;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewDatabase(rootFolderPath);
        ClearPtsForCorrelation();
    }
}

void Localizer::LoadTreeViewFolder(QString rootFolder)
{
    ResetUiCheckboxes();
    disconnect(ui.processButton, nullptr, nullptr, nullptr);
    if (ui.FileTreeView->selectionModel() != nullptr)
        disconnect(ui.FileTreeView->selectionModel(), nullptr, nullptr, nullptr);

    LoadTreeViewUI(rootFolder);

    //==[Event connected to model of treeview]
    connect(ui.FileTreeView, &QTreeView::clicked, this, &Localizer::ModelClicked);
    //==[Event for rest of UI]
    connect(ui.processButton, &QPushButton::clicked, this, &Localizer::ProcessFolderAnalysis);
    SetLabelCount(0);
}

void Localizer::LoadTreeViewFiles(QString rootFolder)
{
    DeactivateUIForSingleFiles();
    disconnect(ui.processButton, nullptr, nullptr, nullptr);

    QDir currentDir = QDir(rootFolder);
    QStringList entries = currentDir.entryList(QDir::NoDotAndDotDot | QDir::Files);
    if (entries.size() > 0)
    {
        LoadTreeViewUI(currentDir.absolutePath());

        //==[Event connected to model of treeview]
        connect(ui.FileTreeView, &QTreeView::clicked, this, &Localizer::ModelClicked);
        //==[Event for rest of UI]
        connect(ui.processButton, &QPushButton::clicked, this, &Localizer::ProcessSingleAnalysis);
        SetLabelCount(0);
    }
    else
    {
        QMessageBox::information(this, "Are you sure ? ", "There is no file in this folder.");
    }
}

void Localizer::LoadTreeViewDatabase(QString rootFolder)
{
    ResetUiCheckboxes();
    disconnect(ui.processButton, nullptr, nullptr, nullptr);
    if (ui.FileTreeView->selectionModel() != nullptr)
        disconnect(ui.FileTreeView->selectionModel(), nullptr, nullptr, nullptr);

    LoadTreeViewUI(rootFolder);

    //==[Event connected to model of treeview]
    connect(ui.FileTreeView, &QTreeView::clicked, this, &Localizer::ModelClicked);
    //==[Event for rest of UI]
    connect(ui.processButton, &QPushButton::clicked, this, &Localizer::ProcessMultiFolderAnalysis);
    SetLabelCount(0);
}

void Localizer::LoadTreeViewUI(QString initialFolder)
{
    //Define file system model at the root folder chosen by the user
    m_localFileSystemModel = new QFileSystemModel();
    m_localFileSystemModel->setReadOnly(true);
    m_localFileSystemModel->setRootPath(initialFolder);

    //set model in treeview
    ui.FileTreeView->setModel(m_localFileSystemModel);
    //Show only what is under this path
    ui.FileTreeView->setRootIndex(m_localFileSystemModel->index(initialFolder));
    //Show everything put starts at the given model index
    //ui.FileTreeView->setCurrentIndex(m_localFileSystemModel.index(initialFolder));

    //==[Ui Layout]
    ui.FileTreeView->setAnimated(false);
    ui.FileTreeView->setIndentation(20);
    //Sorting enabled puts elements in reverse (last is first, first is last)
    //ui.FileTreeView->setSortingEnabled(true);
    //Hide name, file size, file type , etc
    ui.FileTreeView->hideColumn(1);
    ui.FileTreeView->hideColumn(2);
    ui.FileTreeView->hideColumn(3);
    ui.FileTreeView->header()->hide();
}

int Localizer::PreparePatientFolder()
{
    QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
    if(GetSelectedFolderCount(selectedRows) == 0) return -1;

    //Create data structure used by the processing part
    if (currentFiles.size() > 0)
        currentFiles.clear();
    deleteAndNullify1D(currentPat);
    currentPat = new patientFolder(m_localFileSystemModel->rootPath().toStdString());

    //check which elements to keep and delete since the ui can show single files with folders
    std::vector<bool> deleteMe = std::vector<bool>(currentPat->localizerFolder().size(), true);
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(isRoot && info.isDir())
        {
            int idToKeep = -1;
            for(int j = 0; j < currentPat->localizerFolder().size(); j++)
            {
                QString internalPath = QString::fromStdString(currentPat->localizerFolder()[j].rootLocaFolder());
                QString uiPath = QString(info.absoluteFilePath() + "/");
                if (internalPath.compare(uiPath, Qt::CaseSensitive) == 0)
                {
                    idToKeep = j;
                    break;
                }
            }

            if(idToKeep != -1)
            {
                deleteMe[idToKeep] = false;
            }
        }
    }

    int ExamCount = static_cast<int>(deleteMe.size());
    for (int i = ExamCount - 1; i >= 0; i--)
    {
        if (deleteMe[i])
        {
            currentPat->localizerFolder().erase(currentPat->localizerFolder().begin() + i);
        }
    }

    return 0;
}

void Localizer::PrepareSingleFiles()
{
    QStringList extention;
    extention << "trc" << "eeg" << "vhdr" << "edf";

    //Create data structure
    deleteAndNullify1D(currentPat);
    if (currentFiles.size() > 0)
        currentFiles.clear();

    int nbFrequencyBands = 0;
    for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
    {
        if (ui.FrequencyListWidget->item(i)->checkState() == Qt::CheckState::Checked)
            nbFrequencyBands++;
    }

    QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(isRoot && info.isFile())
        {
            if(extention.contains(info.suffix().toLower()))
            {
                //if is eeg file , get path and create singlefile and put in currentFiles vector
                singleFile file(info.absoluteFilePath().toStdString(), nbFrequencyBands);
                currentFiles.push_back(file);
            }
        }
    }
}

std::vector<patientFolder> Localizer::PrepareDBFolders()
{
    std::vector<std::string> locaToSearchFor;
    ChooseLocaWindow* elecWin = new ChooseLocaWindow(nullptr);
    int res = elecWin->exec();
    locaToSearchFor = (res == 1) ? std::vector<std::string>(elecWin->ElementsToLookFor()) : std::vector<std::string>();
    delete elecWin;

    QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
    if(GetSelectedFolderCount(selectedRows) == 0) return std::vector<patientFolder>();

    //Create data structure used by the processing part
    if (currentFiles.size() > 0) currentFiles.clear();
    deleteAndNullify1D(currentPat);

    std::vector<patientFolder> subjects;
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(isRoot && info.isDir())
        {
            QString subjectRoot = info.absoluteFilePath();
            try
            {
                patientFolder pat = patientFolder(subjectRoot.toStdString());

                //check which elements to keep and delete
                std::vector<bool> deleteMe = std::vector<bool>(pat.localizerFolder().size(), true);
                int idToKeep = -1;
                for(int j = 0; j < pat.localizerFolder().size(); j++)
                {
                    //if(pat.localizerFolder()[j].localizerName() == "ARFA" || pat.localizerFolder()[j].localizerName() == "AUDI" ||
                    //   pat.localizerFolder()[j].localizerName() == "LEC1" || pat.localizerFolder()[j].localizerName() == "LEC2" ||
                    //   pat.localizerFolder()[j].localizerName() == "MCSE" || pat.localizerFolder()[j].localizerName() == "MOTO" ||
                    //   pat.localizerFolder()[j].localizerName() == "MVEB" || pat.localizerFolder()[j].localizerName() == "MVIS" ||
                    //   pat.localizerFolder()[j].localizerName() == "REST" || pat.localizerFolder()[j].localizerName() == "VISU")
                    if(std::find(locaToSearchFor.begin(), locaToSearchFor.end(), pat.localizerFolder()[j].localizerName()) != locaToSearchFor.end())
                    {
                        deleteMe[j] = false;
                    }
                }

                int ExamCount = static_cast<int>(deleteMe.size());
                for (int j = ExamCount - 1; j >= 0; j--)
                {
                    if (deleteMe[j])
                    {
                        pat.localizerFolder().erase(pat.localizerFolder().begin() + j);
                    }
                }

                subjects.push_back(pat);
            }
            catch(const std::exception&)
            {
                QString message = subjectRoot + " does not seem to be a valid subject folder, skipping it";
                DisplayLog(message);
            }
        }
    }

    return subjects;
}

void Localizer::InitProgressBar()
{
    ui.progressBar->reset();
    nbDoneTask = 0;
    nbTaskToDo = 0;

    int nbFolderSelected = GetSelectedFolderCount(ui.FileTreeView->selectionModel()->selectedRows());
    int nbFrequencyBands = 0;
    for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
    {
        if (ui.FrequencyListWidget->item(i)->checkState() == Qt::CheckState::Checked)
            nbFrequencyBands++;
    }

    ui.Eeg2envCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo++; //eeg2env, wheter we need to compute or load
    ui.Env2plotCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui.TrialmatCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui.CorrelationMapsCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;
    ui.StatFileExportCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;

    nbTaskToDo *= nbFolderSelected * nbFrequencyBands;
}

void Localizer::InitMultiSubjectProgresBar(std::vector<patientFolder> subjects)
{
    ui.progressBar->reset();
    nbDoneTask = 0;
    nbTaskToDo = 0;

    int nbFolderSelected = GetSelectedFolderCount(ui.FileTreeView->selectionModel()->selectedRows());
    int nbFrequencyBands = 0;
    for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
    {
        if (ui.FrequencyListWidget->item(i)->checkState() == Qt::CheckState::Checked)
            nbFrequencyBands++;
    }

    int subjectCount = static_cast<int>(subjects.size());
    for(int i = 0; i < subjectCount; i++)
    {
        int nbTaskPerExam = 0;
        int examCount = static_cast<int>(subjects[i].localizerFolder().size());

        ui.Eeg2envCheckBox->isChecked() ? nbTaskPerExam++ : nbTaskPerExam++; //eeg2env, wheter we need to compute or load
        ui.Env2plotCheckBox->isChecked() ? nbTaskPerExam++ : nbTaskPerExam;
        ui.TrialmatCheckBox->isChecked() ? nbTaskPerExam++ : nbTaskPerExam;
        ui.CorrelationMapsCheckBox->isChecked() ? nbTaskPerExam++ : nbTaskPerExam;
        ui.StatFileExportCheckBox->isChecked() ? nbTaskPerExam++ : nbTaskPerExam;

        nbTaskToDo += (nbTaskPerExam * examCount * nbFrequencyBands);
    }
}

std::vector<InsermLibrary::FrequencyBandAnalysisOpt> Localizer::GetUIAnalysisOption()
{
    std::vector<InsermLibrary::FrequencyBand> frequencyBands = m_frequencyFile->FrequencyBands();
    std::vector<int> indexes;
    for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
    {
        if (ui.FrequencyListWidget->item(i)->checkState() == Qt::CheckState::Checked)
            indexes.push_back(i);
    }
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOpt = std::vector<InsermLibrary::FrequencyBandAnalysisOpt>(indexes.size());

    for (size_t i = 0; i < indexes.size(); i++)
    {
        //	- env2plot and / or trial mat
        analysisOpt[i].analysisParameters.eeg2env2 = ui.Eeg2envCheckBox->isChecked();
        analysisOpt[i].analysisParameters.outputType = EEGFormat::GetFileTypeFromString(ui.FileOutputComboBox->currentText().toStdString());
        analysisOpt[i].analysisParameters.calculationType = Algorithm::Strategy::GetFileTypeFromString(ui.AnalysisCcomboBox->currentText().toStdString());
        analysisOpt[i].env2plot = ui.Env2plotCheckBox->isChecked();
        analysisOpt[i].trialmat = ui.TrialmatCheckBox->isChecked();
        analysisOpt[i].correMaps = ui.CorrelationMapsCheckBox->isChecked();
        analysisOpt[i].statFiles = ui.StatFileExportCheckBox->isChecked();

        //	- what frequency bands data is
        QString label = ui.FrequencyListWidget->item(indexes[i])->text();
        std::vector<InsermLibrary::FrequencyBand>::iterator it = std::find_if(frequencyBands.begin(), frequencyBands.end(), [&](const InsermLibrary::FrequencyBand &c)
        {
            return (c.Label() == label.toStdString());
        });
        if (it != frequencyBands.end())
        {
            analysisOpt[i].Band = InsermLibrary::FrequencyBand(*it);
        }
    }

    return analysisOpt;
}

int Localizer::GetSelectedFolderCount(QModelIndexList selectedIndexes)
{
    int nbElementSelected = 0;
    for (int i = 0; i < selectedIndexes.size(); i++)
    {
        bool isRoot = selectedIndexes[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedIndexes[i]);
        bool wantedElement = m_isPatFolder ? info.isDir() : info.isFile();
        if (wantedElement && isRoot)
        {
            if (ui.FileTreeView->selectionModel()->isSelected(selectedIndexes[i]))
                nbElementSelected++;
            else
                nbElementSelected--;
        }
        //else
        //{
        //	ui.FileTreeView->selectionModel()->setCurrentIndex(selectedIndexes[i], QItemSelectionModel::Deselect);
        //}
    }
    return nbElementSelected;
}

//=== Slots	
void Localizer::SetLabelCount(int count)
{
    QString label = m_isPatFolder ? (count > 1 ? " patient folders" : " patient folder") : (count > 1 ? "single files" : "single file");
    ui.FolderCountLabel->setText(QString::number(count) + label + " selected for Analysis");
}

void Localizer::ModelClicked(const QModelIndex &current)
{
    QModelIndexList selectedIndexes = ui.FileTreeView->selectionModel()->selectedRows();
    int nbFolderSelected = GetSelectedFolderCount(selectedIndexes);
    SetLabelCount(nbFolderSelected);
}

void Localizer::ShowFileTreeContextMenu(QPoint point)
{
    QStringList extention;
    extention << "trc" << "eeg" << "vhdr" << "edf";

    QMenu* contextMenu = new QMenu();

    QModelIndex index = ui.FileTreeView->indexAt(point);
    QFileInfo selectedElementInfo = QFileInfo(m_localFileSystemModel->filePath(index));
    QString suffix = selectedElementInfo.suffix().toLower();

    bool isRootFile = m_isPatFolder ? (index.parent().parent() == ui.FileTreeView->rootIndex()) : (index.parent() == ui.FileTreeView->rootIndex());
    if (isRootFile && extention.contains(suffix)) //if it is a recognized eeg file at the root level
    {
        QAction* processErpAction = contextMenu->addAction("Process ERP", [=]
        {
            QList<QString> files;
            QModelIndexList indexes = ui.FileTreeView->selectionModel()->selectedRows();
            for (int i = 0; i < indexes.count(); i++)
            {
                files.push_back(m_localFileSystemModel->filePath(indexes[i]));
            }

            if (!isAlreadyRunning)
            {
                ErpProcessor erpWindow(files, this);
                connect(&erpWindow, &ErpProcessor::SendExamCorrespondance, this, &Localizer::ProcessERPAnalysis);
                erpWindow.exec();
                disconnect(&erpWindow, nullptr, nullptr, nullptr);

            }
            else
            {
                QMessageBox::information(this, "Error", "Process already running");
            }
        });
        QAction* convertFileAction = contextMenu->addAction("Convert File", [=]
        {
            QList<QString> files;
            QModelIndexList indexes = ui.FileTreeView->selectionModel()->selectedRows();
            for (int i = 0; i < indexes.count(); i++)
            {
                files.push_back(m_localFileSystemModel->filePath(indexes[i]));
            }

            if (!isAlreadyRunning)
            {
                FileConverterProcessor fileWindow(files, this);
                connect(&fileWindow, &FileConverterProcessor::SendExamCorrespondance, this, &Localizer::ProcessFileConvertion);
                fileWindow.exec();
                disconnect(&fileWindow,nullptr, nullptr, nullptr);
            }
            else
            {
                QMessageBox::information(this, "Error", "Process already running");
            }
        });
        QAction* processConcatenationAction = contextMenu->addAction("Concatenate TRC Files", [=]
        {
            QList<QString> files;
            QModelIndexList indexes = ui.FileTreeView->selectionModel()->selectedRows();
            for (int i = 0; i < indexes.count(); i++)
            {
                QFileInfo selectedElementInfo = QFileInfo(m_localFileSystemModel->filePath(index));
                QString suffix = selectedElementInfo.suffix().toLower();
                if(suffix.contains("trc"))
                    files.push_back(m_localFileSystemModel->filePath(indexes[i]));
            }

            if (!isAlreadyRunning)
            {
                //TODO : QinputDialog is exploding, need to see why since it has been a long time since we reactivated concatenation
                //QString fileName = QInputDialog::getText(this, "New Name", "Choose New File Name", QLineEdit::Normal, "New Micromed File", nullptr);

                QString fileName = "CONCATENATED_FILE";
                std::string suffixx = EEGFormat::Utility::GetFileExtension(fileName.toStdString());
                QString suffix = QString::fromStdString(suffixx).toLower();

                if (!fileName.isEmpty()) //TODO : put that back when issue with qinputdialog is solved => && suffix.contains("trc"))
                {
                    QString dir = QFileInfo(m_localFileSystemModel->filePath(indexes[0])).dir().absolutePath();
                    ProcessMicromedFileConcatenation(files, dir, fileName);
                }
                else
                {
                    QMessageBox::information(this, "Error", "File name must contain the .TRC extention and be non empty");
                }
            }
            else
            {
                QMessageBox::information(this, "Error", "Process already running");
            }
        });
    
        if (sender() == ui.FileTreeView)
        {
            contextMenu->exec(ui.FileTreeView->viewport()->mapToGlobal(point));
        }

        //Note : intead of using this => connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
        //we delete the menu at the end so whether there was an action or not it is cleaned correctly . This is
        //due to the need of having gui for user interactions
        delete contextMenu;
    }
}

void Localizer::SelectPtsForCorrelation()
{
    QFileDialog* fileDial = new QFileDialog(this);
    fileDial->setFileMode(QFileDialog::FileMode::AnyFile);
    QString fileName = fileDial->getOpenFileName(this, tr("Choose Patient Folder"), tr("C:\\"), tr("*.pts"));
    if (fileName != "")
    {
        PtsFilePath = fileName.toStdString();
        ui.BrowsePtsButton->setEnabled(false);
    }
}

void Localizer::ClearPtsForCorrelation()
{
    ui.BrowsePtsButton->setEnabled(true);
    PtsFilePath = "";
}

void Localizer::ToggleAllBands()
{
    Qt::CheckState status = ui.AllBandsCheckBox->checkState();
    for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
    {
        ui.FrequencyListWidget->item(i)->setCheckState(status);
    }
}

void Localizer::ProcessFolderAnalysis()
{
    if (!isAlreadyRunning)
    {
        //Data Struct info
        InsermLibrary::picOption optpic = picOpt->getPicOption();
        InsermLibrary::statOption optstat = optStat->getStatOption();

        //UI
        InitProgressBar();
        std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();
		std::vector<InsermLibrary::FileExt> filePriority = std::vector<InsermLibrary::FileExt>(m_GeneralOptionsFile->FileExtensionsFavorite());

        //Should probably senbd back the struct here and not keep a global variable
        int result = PreparePatientFolder();
        if(result != -1)
        {
            thread = new QThread;
            worker = new PatientFolderWorker(*currentPat, analysisOptions, optstat, optpic, filePriority, PtsFilePath);

            //=== Event update displayer
            connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);
            connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Localizer::DisplayLog);
            connect(worker->GetLoca(), &InsermLibrary::LOCA::incrementAdavnce, this, &Localizer::UpdateProgressBar);

            //New ping pong order
            connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
            connect(worker, &IWorker::sendElectrodeList, this, &Localizer::ReceiveElectrodeList);
            connect(this, &Localizer::MontageDone, worker, &IWorker::Process);

			//=== Event From worker and thread
            connect(worker, &IWorker::finished, thread, &QThread::quit);
            connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

            //=== Launch Thread and lock possible second launch
            worker->moveToThread(thread);
            thread->start();
            isAlreadyRunning = true;
        }
        else
        {
            QMessageBox::critical(this, "No Folder Selected", "You need to select at least one folder in the user interface");
        }
    }
    else
    {
        QMessageBox::critical(this, "Analysis already running", "Please wait until the current analysis if finished");
    }
}

void Localizer::ProcessSingleAnalysis()
{
    if (!isAlreadyRunning)
    {
        InitProgressBar();
        std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();

        //Should probably senbd back the vector here and not keep a global variable
        PrepareSingleFiles();

        thread = new QThread;
        worker = new SingleFilesWorker(currentFiles, analysisOptions);

        //=== Event update displayer
        connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);
        connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Localizer::DisplayLog);
        connect(worker->GetLoca(), &InsermLibrary::LOCA::incrementAdavnce, this, &Localizer::UpdateProgressBar);

        //New ping pong order
        connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
        connect(worker, &IWorker::sendElectrodeList, this, &Localizer::ReceiveElectrodeList);
        connect(this, &Localizer::MontageDone, worker, &IWorker::Process);

        //=== Event From worker and thread
        connect(worker, &IWorker::finished, thread, &QThread::quit);
        connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

        //=== Launch Thread and lock possible second launch
        worker->moveToThread(thread);
        thread->start();
        isAlreadyRunning = true;
    }
    else
    {
        QMessageBox::information(this, "Error", "Analysis already running");
    }
}

void Localizer::ProcessMultiFolderAnalysis()
{
    if (!isAlreadyRunning)
    {
        //Data Struct info
        InsermLibrary::picOption optpic = picOpt->getPicOption();
        InsermLibrary::statOption optstat = optStat->getStatOption();
        std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();
        std::vector<InsermLibrary::FileExt> filePriority = std::vector<InsermLibrary::FileExt>(m_GeneralOptionsFile->FileExtensionsFavorite());

        //Should probably senbd back the struct here and not keep a global variable
        std::vector<patientFolder> subjects = PrepareDBFolders();
        InitMultiSubjectProgresBar(subjects);

        if(subjects.size() > 0)
        {
            thread = new QThread;
            worker = new MultiSubjectWorker(subjects, analysisOptions, optstat, optpic, filePriority, PtsFilePath);

            //=== Event update displayer
            connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);
            connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Localizer::DisplayLog);
            connect(worker->GetLoca(), &InsermLibrary::LOCA::incrementAdavnce, this, &Localizer::UpdateProgressBar);

            //New ping pong order
            connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
            connect(worker, &IWorker::sendElectrodeList, this, &Localizer::ReceiveElectrodeList);
            connect(this, &Localizer::MontageDone, worker, &IWorker::Process);

            //=== Event From worker and thread
            connect(worker, &IWorker::finished, thread, &QThread::quit);
            connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

            //=== Launch Thread and lock possible second launch
            worker->moveToThread(thread);
            thread->start();
            isAlreadyRunning = true;
        }
        else
        {
            QMessageBox::critical(this, "No subject folder selected", "You need to select at least one folder in the user interface");
        }
    }
    else
    {
        QMessageBox::critical(this, "Analysis already running", "Please wait until the current analysis if finished");
    }
}

void Localizer::ProcessERPAnalysis(QList<QString> exams)
{
    QModelIndexList indexes = ui.FileTreeView->selectionModel()->selectedRows();
    if (indexes.size() != exams.size())
    {
        DisplayColoredLog("Not the same number of eeg files and exam files, aborting Erp Processing", Qt::GlobalColor::red);
        return;
    }

    std::vector<std::string> files;
    std::vector<std::string> provFiles;
    for (int i = 0; i < indexes.count(); i++)
    {
        files.push_back(m_localFileSystemModel->filePath(indexes[i]).toStdString());
        provFiles.push_back(exams[i].toStdString());
    }

    nbTaskToDo = exams.size();
    nbDoneTask = 0;
    ui.progressBar->reset();

    InsermLibrary::picOption opt = picOpt->getPicOption();
    thread = new QThread;
    worker = new ErpWorker(files, provFiles, opt);

    //=== Event update displayer
    connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);
    connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Localizer::DisplayLog);
    connect(worker, &IWorker::incrementAdavnce, this, &Localizer::UpdateProgressBar);

    //New ping pong order
    connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
    connect(worker, &IWorker::sendElectrodeList, this, &Localizer::ReceiveElectrodeList);
    connect(this, &Localizer::MontageDone, worker, &IWorker::Process);

    //=== Event From worker and thread
    connect(worker, &IWorker::finished, thread, &QThread::quit);
    connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

    //=== Launch Thread and lock possible second launch
    worker->moveToThread(thread);
    thread->start();
    isAlreadyRunning = true;
}

void Localizer::ProcessFileConvertion(QList<QString> newFileType)
{
    QModelIndexList indexes = ui.FileTreeView->selectionModel()->selectedRows();
    if (indexes.size() != newFileType.size())
    {
        DisplayColoredLog("Not the same number of eeg files and selected new types, aborting File Convertion", Qt::GlobalColor::red);
        return;
    }

    std::vector<std::string> files;
    std::vector<std::string> provFiles;
    for (int i = 0; i < indexes.count(); i++)
    {
        files.push_back(m_localFileSystemModel->filePath(indexes[i]).toStdString());
        provFiles.push_back(newFileType[i].toStdString());
    }

    nbTaskToDo = newFileType.size();
    nbDoneTask = 0;
    ui.progressBar->reset();

    InsermLibrary::picOption opt = picOpt->getPicOption();
    thread = new QThread;
    worker = new FileConverterWorker(files, provFiles);

    //=== Event update displayer
    connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);

    //New ping pong order
    connect(thread, &QThread::started, this, [&]{ worker->ExtractElectrodeList(); });
    connect(worker, &IWorker::sendElectrodeList, this, &Localizer::ReceiveElectrodeList);
    connect(this, &Localizer::MontageDone, worker, &IWorker::Process);

    //=== Event From worker and thread
    connect(worker, &IWorker::finished, thread, &QThread::quit);
    connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &IWorker::finished, this, [&] { ui.progressBar->setValue(100); });
    connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

    //=== Launch Thread and lock possible second launch
    worker->moveToThread(thread);
    thread->start();
    isAlreadyRunning = true;
}

void Localizer::ProcessMicromedFileConcatenation(QList<QString> files, QString directoryPath, QString fileName)
{
    int FileNumber = files.size();
    std::vector<std::string> trcFiles;
    for (int i = 0; i < FileNumber; i++)
    {
        trcFiles.push_back(files[i].toStdString());
    }

    thread = new QThread;
    std::string directoryPathString = directoryPath.toStdString();
    std::string fileNameString = fileName.toStdString();

    worker = new ConcatenationWorker(trcFiles, directoryPathString, fileNameString);

    //Update info
    connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);

    //=== Event From worker and thread
    connect(thread, &QThread::started, worker, &IWorker::Process);
    connect(worker, &IWorker::finished, thread, &QThread::quit);
    connect(worker, &IWorker::finished, worker, &IWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

    //=== Launch Thread and lock possible second launch
    worker->moveToThread(thread);
    thread->start();
    isAlreadyRunning = true;
}

void Localizer::DisplayLog(QString messageToDisplay)
{
    ui.messageDisplayer->append(messageToDisplay);
}

void Localizer::DisplayColoredLog(QString messageToDisplay, QColor color)
{
    ui.messageDisplayer->setTextColor(color);
    DisplayLog(messageToDisplay);
    ui.messageDisplayer->setTextColor(Qt::GlobalColor::black);
}

void Localizer::UpdateProgressBar(int divider)
{
    nbDoneTask = nbDoneTask + (1.0f / divider);
    ui.progressBar->setValue((nbDoneTask / nbTaskToDo) * 100);
}

void Localizer::CancelAnalysis()
{
    if (isAlreadyRunning)
    {
        m_lockLoop.lockForWrite();
        thread->terminate();
        while (!thread->isFinished())
        {
        }
        isAlreadyRunning = false;
        QMessageBox::information(this, "Canceled", "Analysis has been canceled by the user");
        DisplayLog("");
        DisplayLog("Analysis has been canceled by the user");
        DisplayLog("");
        m_lockLoop.unlock();
    }
}

void Localizer::ReceiveElectrodeList(std::vector<std::string> ElectrodeList, std::string ConnectCleanerFile)
{
    ConnectCleaner *elecWin = new ConnectCleaner(ElectrodeList, ConnectCleanerFile.c_str(), nullptr);
    int res = elecWin->exec();
    if(res == 1)
    {
        worker->SetExternalParameters(elecWin->IndexToDelete(), elecWin->CorrectedLabel(), elecWin->OperationToDo());
        MontageDone(res);
    }
    else
    {
        CancelAnalysis();
    }
    delete elecWin;
}
