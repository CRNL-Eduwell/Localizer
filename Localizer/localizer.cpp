#include "localizer.h"
#include "FrequenciesWindow.h"

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
                m_subjectType = SubjectType::Subject;
                QString rootFolderPath = QDir(fileName).absolutePath();
                LoadTreeViewFolder(rootFolderPath);
                SetupComboxBoxElements();
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
    m_frequencyFile = InsermLibrary::FrequencyFile();
    m_frequencyFile.Load();
	//==
    LoadFrequencyBandsUI(m_frequencyFile.FrequencyBands());
	//==
    optPerf = new optionsPerf();
    optStat = new optionsStats();
    picOpt = new picOptions();

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

void Localizer::SetupComboxBoxElements()
{
    QStringList list;

    ui.FileOutputComboBox->clear();
    if(m_subjectType == SubjectType::BidsSubject)
    {
        list << "BrainVision";
    }
    else
    {
        list << "Elan" << "BrainVision";
    }
    ui.FileOutputComboBox->addItems(list);
}

void Localizer::ConnectSignals()
{
    ConnectMenuBar();

    ui.FileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.FileTreeView, &QTreeView::customContextMenuRequested, this, &Localizer::ShowFileTreeContextMenu);
    connect(ui.FileTreeView, &DeselectableTreeView::ResetNbFolder, this, [&]() { SetLabelCount(0); });
    
    connect(ui.BrowsePtsButton, &QPushButton::clicked, this, &Localizer::SelectPtsForCorrelation);
    connect(ui.ClearPtsButton, &QPushButton::clicked, this, &Localizer::ClearPtsForCorrelation);
    connect(ui.BypassCCFPushButton, &QPushButton::clicked, this, &Localizer::DealWithCCFToggle);

    connect(ui.AllBandsCheckBox, &QCheckBox::clicked, this, &Localizer::ToggleAllBands);
    connect(ui.cancelButton, &QPushButton::clicked, this, &Localizer::CancelAnalysis);
}

void Localizer::ConnectMenuBar()
{
    //===Fichier
    QAction* openPatFolder = ui.menuFiles->actions().at(0);
    connect(openPatFolder, &QAction::triggered, this, [&]
    {
        if(!isAlreadyRunning)
        {
            LoadPatientFolder();
        }
        else
        {
            QMessageBox::information(this, "Error", "Wait until end of analysis or cancel it");
        }
    });
    QAction* openDatabaseFolder = ui.menuFiles->actions().at(1);
    connect(openDatabaseFolder, &QAction::triggered, this, [&]
    {
        if(!isAlreadyRunning)
        {
            LoadDatabaseFolder();
        }
        else
        {
            QMessageBox::information(this, "Error", "Wait until end of analysis or cancel it");
        }
    });
    QAction* openSpecificFolder = ui.menuFiles->actions().at(2);
    connect(openSpecificFolder, &QAction::triggered, this, [&]
    {
        if(!isAlreadyRunning)
        {
            LoadSpecificFolder();
        }
        else
        {
            QMessageBox::information(this, "Error", "Wait until end of analysis or cancel it");
        }
    });
    QAction* openLoadBidsSubjectFolder = ui.menuFiles->actions().at(3);
    connect(openLoadBidsSubjectFolder, &QAction::triggered, this, [&]
    {
        if(!isAlreadyRunning)
        {
            LoadBidsSubject();
        }
        else
        {
            QMessageBox::information(this, "Error", "Wait until end of analysis or cancel it");
        }
    });
    //===Configuration
    QAction* openLocaMenu = ui.menuConfiguration->actions().at(0);
    connect(openLocaMenu, &QAction::triggered, this, [&] 
    { 
        ProtocolsWindow* protocolsWindow = new ProtocolsWindow(this);
        protocolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        protocolsWindow->show();
    });
    QAction* openFilePriorityMenu = ui.menuConfiguration->actions().at(1);
	connect(openFilePriorityMenu, &QAction::triggered, this, [&] 
	{ 	        
        GeneralOptionsWindow* generalOptionsWindow = new GeneralOptionsWindow(m_GeneralOptionsFile);
		generalOptionsWindow->setAttribute(Qt::WA_DeleteOnClose);
		generalOptionsWindow->show(); 
	});
    QAction* openStatMenu = ui.menuConfiguration->actions().at(2);
    connect(openStatMenu, &QAction::triggered, this, [&] { optStat->exec(); });
    QAction* openPicMenu = ui.menuConfiguration->actions().at(3);
    connect(openPicMenu, &QAction::triggered, this, [&] { picOpt->exec(); });
    QAction* openPerfMenu = ui.menuConfiguration->actions().at(4);
    connect(openPerfMenu, &QAction::triggered, this, [&] { optPerf->exec(); });
    QAction* openFreqBandMenu = ui.menuConfiguration->actions().at(5);
    connect(openFreqBandMenu, &QAction::triggered, this, [&]
    {
        FrequenciesWindow* frequenciesWindow = new FrequenciesWindow(this);
        connect(frequenciesWindow, &FrequenciesWindow::accepted, this, [&]
        {
            m_frequencyFile = InsermLibrary::FrequencyFile();
            m_frequencyFile.Load();
            LoadFrequencyBandsUI(m_frequencyFile.FrequencyBands());
        });
        frequenciesWindow->setAttribute(Qt::WA_DeleteOnClose);
        frequenciesWindow->show();
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
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose Lyon Patient Folder"));
    if (fileName != "")
    {
        m_subjectType = SubjectType::Subject;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewFolder(rootFolderPath);
        ClearPtsForCorrelation();
        SetupComboxBoxElements();
    }
}

void Localizer::LoadSpecificFolder()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setFileMode(QFileDialog::FileMode::AnyFile);
    fileDial->setNameFilters(QStringList()<<"*.trc"<<" *.eeg"<<" *.edf");
    QString fileName = fileDial->getExistingDirectory(this,  tr("Choose folder with one or multiple eeg files : "), tr("C:\\"));
    if (fileName != "")
    {
        m_subjectType = SubjectType::SingleFile;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewFiles(rootFolderPath);
        SetupComboxBoxElements();
    }
}

void Localizer::LoadDatabaseFolder()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setOption(QFileDialog::ShowDirsOnly, true);
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose a folder containing multiple Lyon Patients"));
    if (fileName != "")
    {
        m_subjectType = SubjectType::MultiSubject;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewDatabase(rootFolderPath);
        ClearPtsForCorrelation();
        SetupComboxBoxElements();
    }
}

void Localizer::LoadBidsSubject()
{
    QFileDialog *fileDial = new QFileDialog(this);
    fileDial->setOption(QFileDialog::ShowDirsOnly, true);
    QString fileName = fileDial->getExistingDirectory(this, tr("Choose Bids Subject Folder"));
    if (fileName != "")
    {
        m_subjectType = SubjectType::BidsSubject;
        QString rootFolderPath = QDir(fileName).absolutePath();
        LoadTreeViewBids(rootFolderPath);
        SetupComboxBoxElements();
        bool seemsBids = SeemsToBeBidsSubject(rootFolderPath);
        if(seemsBids)
        {
            m_bidsSubject = ParseBidsSubjectInfo(rootFolderPath);
        }
        else
        {
            QMessageBox::information(this, "Error", "It does not seem to be a valid Bids subject, please check your data");
        }
    }
}

bool Localizer::SeemsToBeBidsSubject(QString rootFolder)
{
    QDir root = QDir(rootFolder);
    bool rootNameOk = root.dirName().contains("sub-");
    bool hasPostSession = false;
    bool hasIeegFolder = false;

    QString NamePostSession = "";
    root.setFilter(QDir::Dirs);
    QStringList entries = root.entryList();
    for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
    {
        QString dirname = *entry;
        if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
        {
            if(dirname.contains("ses-post"))
            {
                NamePostSession = dirname;
                hasPostSession = true;

                if(hasPostSession)
                {
                    QString postSessionIeegFolderToLookFor = rootFolder + "/" + NamePostSession + "/ieeg";

                    QDirIterator it(rootFolder, QStringList() << "anat" << "ieeg", QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                    while(it.hasNext())
                    {
                        QString folder = it.next();
                        if(folder == postSessionIeegFolderToLookFor)
                        {
                            hasIeegFolder = true;
                            break;
                        }
                    }
                }
            }
        }
    }

//    qDebug() << rootNameOk;
//    qDebug() << hasPostSession;
//    qDebug() << hasIeegFolder;

    return rootNameOk && hasPostSession && hasIeegFolder;
}

BidsSubject Localizer::ParseBidsSubjectInfo(QString rootFolder)
{
    std::vector<std::string> tasks;
    std::vector<InsermLibrary::BrainVisionFileInfo> fileInfos;
    QDirIterator it(rootFolder, QStringList() << "*.vhdr", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        QString vhdr = it.next();
        QStringList split = vhdr.split("_task-");
        QString task = split[1].split("_").first();

        tasks.push_back(task.toStdString());
        fileInfos.push_back(InsermLibrary::BrainVisionFileInfo(vhdr.toStdString()));
    }

    if(tasks.size() == fileInfos.size())
    {
        return BidsSubject(rootFolder.toStdString(), tasks, fileInfos);
    }
    else
    {
        return BidsSubject();
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

void Localizer::LoadTreeViewBids(QString rootFolder)
{
    ResetUiCheckboxes();
    disconnect(ui.processButton, nullptr, nullptr, nullptr);
    if (ui.FileTreeView->selectionModel() != nullptr)
        disconnect(ui.FileTreeView->selectionModel(), nullptr, nullptr, nullptr);

    LoadTreeViewUI(rootFolder);

    //==[Event connected to model of treeview]
    connect(ui.FileTreeView, &QTreeView::clicked, this, &Localizer::ModelClicked);
    //==[Event for rest of UI]
    connect(ui.processButton, &QPushButton::clicked, this, &Localizer::ProcessBidsSubjectAnalysis);
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
    if(GetSelectedElementCount(selectedRows) == 0) return -1;

    //Create data structure used by the processing part
    if (currentFiles.size() > 0)
        currentFiles.clear();
    deleteAndNullify1D(currentPat);
    currentPat = new SubjectFolder(m_localFileSystemModel->rootPath().toStdString());

    //check which elements to keep and delete since the ui can show single files with folders
    std::vector<bool> deleteMe = std::vector<bool>(currentPat->ExperimentFolders().size(), true);
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(isRoot && info.isDir())
        {
            int idToKeep = -1;
            for(int j = 0; j < currentPat->ExperimentFolders().size(); j++)
            {
                QString internalPath = QString::fromStdString(currentPat->ExperimentFolders()[j].Path());
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
            currentPat->ExperimentFolders().erase(currentPat->ExperimentFolders().begin() + i);
        }
    }

    return 0;
}

int Localizer::PrepareSingleFiles()
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

    return currentFiles.size() == 0 ? - 1 : 0;
}

std::vector<SubjectFolder*> Localizer::PrepareDBFolders()
{
    std::vector<std::string> locaToSearchFor;
    ChooseLocaWindow* elecWin = new ChooseLocaWindow(nullptr);
    int res = elecWin->exec();
    locaToSearchFor = (res == 1) ? std::vector<std::string>(elecWin->ElementsToLookFor()) : std::vector<std::string>();
    delete elecWin;

    QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
    if(GetSelectedElementCount(selectedRows) == 0) return std::vector<SubjectFolder*>();

    //Create data structure used by the processing part
    if (currentFiles.size() > 0) currentFiles.clear();
    deleteAndNullify1D(currentPat);

    std::vector<SubjectFolder*> subjects;
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(isRoot && info.isDir())
        {
            QString subjectRoot = info.absoluteFilePath();
            try
            {
                SubjectFolder* pat = new SubjectFolder(subjectRoot.toStdString());

                //check which elements to keep and delete
                std::vector<bool> deleteMe = std::vector<bool>(pat->ExperimentFolders().size(), true);
                int idToKeep = -1;
                for(int j = 0; j < pat->ExperimentFolders().size(); j++)
                {
                    if(std::find(locaToSearchFor.begin(), locaToSearchFor.end(), pat->ExperimentFolders()[j].ExperimentLabel()) != locaToSearchFor.end())
                    {
                        deleteMe[j] = false;
                    }
                }

                int ExamCount = static_cast<int>(deleteMe.size());
                for (int j = ExamCount - 1; j >= 0; j--)
                {
                    if (deleteMe[j])
                    {
                        pat->ExperimentFolders().erase(pat->ExperimentFolders().begin() + j);
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

int Localizer::PrepareBidsSubjectFolder()
{
    QStringList extention;
    extention << "vhdr";

    std::vector<std::string> tasksToKeep;
    QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
    for (int i = 0; i < selectedRows.size(); i++)
    {
        bool isRoot = selectedRows[i].parent() == ui.FileTreeView->rootIndex();
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedRows[i]);
        if(!isRoot && info.isFile())
        {
            if(extention.contains(info.suffix().toLower()))
            {
                QStringList split = info.absoluteFilePath().split("_task-");
                QString task = split[1].split("_").first();
                tasksToKeep.push_back(task.toStdString());
            }
        }
    }

    std::vector<bool> deleteMe = std::vector<bool>(m_bidsSubject.Tasks().size(), true);
    for (int i = 0; i < tasksToKeep.size(); i++)
    {
        std::vector<std::string>::iterator it = std::find(m_bidsSubject.Tasks().begin(), m_bidsSubject.Tasks().end(), tasksToKeep[i]);
        if (it != m_bidsSubject.Tasks().end()) //if the pair already exists
        {
            int index = std::distance(m_bidsSubject.Tasks().begin(), it);
            deleteMe[index] = false;
        }
    }

    if(tasksToKeep.size() == 0) { qDebug() << "no selected "; return -1; }

    workingCopy = BidsSubject(m_bidsSubject);
    int ExamCount = static_cast<int>(deleteMe.size());
    for (int i = ExamCount - 1; i >= 0; i--)
    {
        if (deleteMe[i])
        {
            qDebug() << "Should delete " << m_bidsSubject.Tasks()[i].c_str();
            workingCopy.DeleteTask(m_bidsSubject.Tasks()[i]);
        }
    }

    return 0;
}

void Localizer::InitProgressBar()
{
    ui.progressBar->reset();
    nbDoneTask = 0;
    nbTaskToDo = 0;

    int nbFolderSelected = GetSelectedElementCount(ui.FileTreeView->selectionModel()->selectedRows());
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

void Localizer::InitMultiSubjectProgresBar(std::vector<SubjectFolder*> subjects)
{
    ui.progressBar->reset();
    nbDoneTask = 0;
    nbTaskToDo = 0;

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
        int examCount = static_cast<int>(subjects[i]->ExperimentFolders().size());

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
    std::vector<InsermLibrary::FrequencyBand> frequencyBands = m_frequencyFile.FrequencyBands();
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

        //TODO : maybe something more clean but since we are not dependant on user input , meh
        std::string smoothingIDStr = ui.StatSmoothingComboBox->currentText().toStdString();
        smoothingIDStr.erase(std::remove_if(smoothingIDStr.begin(), smoothingIDStr.end(), [](char c) { return !std::isdigit(c); }), smoothingIDStr.end());
        int num = std::stoi(smoothingIDStr);
        analysisOpt[i].smoothingIDToUse = (num == 0) ? 0 : (num == 250) ? 1 : (num == 500) ? 2 : (num == 1000) ? 3 : (num == 2500) ? 4 : 5;

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

int Localizer::GetSelectedElementCount(QModelIndexList selectedIndexes)
{
    int nbElementSelected = 0;
    for (int i = 0; i < selectedIndexes.size(); i++)
    {
        QFileInfo info = m_localFileSystemModel->fileInfo(selectedIndexes[i]);
        bool isRoot = false;
        bool gothrough = false;
        switch(m_subjectType)
        {
            case SubjectType::Subject:
            {
                isRoot = selectedIndexes[i].parent() == ui.FileTreeView->rootIndex();
                gothrough = info.isDir() && isRoot;
                break;
            }
            case SubjectType::MultiSubject:
            {
                isRoot = selectedIndexes[i].parent() == ui.FileTreeView->rootIndex();
                gothrough = info.isDir() && isRoot;
                break;
            }
            case SubjectType::SingleFile:
            {
                bool isCorrectFile = info.suffix() == "trc" || info.suffix() == "TRC" || info.suffix() == "eeg" || info.suffix() == "vhdr" || info.suffix() == "edf";
                isRoot = selectedIndexes[i].parent() == ui.FileTreeView->rootIndex();
                gothrough = info.isFile() && isRoot && isCorrectFile;
                break;
            }
            case SubjectType::BidsSubject:
            {
                //TODO : add edf later when validated
                bool isCorrectFile = info.suffix() == "vhdr";
                isRoot = selectedIndexes[i].parent().parent().parent() == ui.FileTreeView->rootIndex();
                gothrough = info.isFile() && isRoot && isCorrectFile ;
                break;
            }
            default:
            {
                gothrough = false;
                break;
            }
        }

        if (gothrough)
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
    QString label = "";
    switch(m_subjectType)
    {
        case SubjectType::Subject:
        {
            label = " experiment folders";
            break;
        }
        case SubjectType::MultiSubject:
        {
            label = " patient folders";
            break;
        }
        case SubjectType::SingleFile:
        {
            label = " single files";
            break;
        }
        case SubjectType::BidsSubject:
        {
            label = " experiment files";
            break;
        }
        default:
        {
            label = " UNKNOWN LABEL";
            break;
        }
    }

    ui.FolderCountLabel->setText(QString::number(count) + label + " selected for Analysis");
}

void Localizer::ModelClicked(const QModelIndex &current)
{
    QModelIndexList selectedIndexes = ui.FileTreeView->selectionModel()->selectedRows();
    int selectedElementCount = GetSelectedElementCount(selectedIndexes);
    SetLabelCount(selectedElementCount);
}

void Localizer::ShowFileTreeContextMenu(QPoint point)
{
    if(m_subjectType != SubjectType::Subject) return;

    QStringList extention;
    extention << "trc" << "eeg" << "vhdr" << "edf";

    QMenu* contextMenu = new QMenu();

    QModelIndex index = ui.FileTreeView->indexAt(point);
    QFileInfo selectedElementInfo = QFileInfo(m_localFileSystemModel->filePath(index));
    QString suffix = selectedElementInfo.suffix().toLower();

    bool isRootFile = (m_subjectType == SubjectType::Subject) ? (index.parent().parent() == ui.FileTreeView->rootIndex()) : (index.parent() == ui.FileTreeView->rootIndex());
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
                QString fileName = QInputDialog::getText(this, "New Name", "Choose New File Name (without extension)", QLineEdit::Normal, "New Micromed File", nullptr);
                if (!fileName.isEmpty())
                {
                    QString dir = QFileInfo(m_localFileSystemModel->filePath(indexes[0])).dir().absolutePath();
                    if(QFileInfo::exists(dir + "/" + fileName + ".TRC"))
                    {
                        QMessageBox::information(this, "NopNopNopNopNop", "File already exists, please check the name.");
                    }
                    else
                    {
                        ProcessMicromedFileConcatenation(files, dir, fileName);
                    }
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
    }
    //Note : intead of using this => connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
    //we delete the menu at the end so whether there was an action or not it is cleaned correctly . This is
    //due to the need of having gui for user interactions
    delete contextMenu;
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

void Localizer::DealWithCCFToggle()
{
    m_CCFToggle = !m_CCFToggle;
    if(m_CCFToggle)
    {
        ui.BypassCCFPushButton->setText("Will Bypass CCF");
    }
    else
    {
        ui.BypassCCFPushButton->setText("Will Prompt CCF");
    }
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
        std::vector<InsermLibrary::FileType> filePriority = std::vector<InsermLibrary::FileType>(m_GeneralOptionsFile->FileExtensionsFavorite());

        //Should probably senbd back the struct here and not keep a global variable
        int result = PreparePatientFolder();
        if(result != -1)
        {
            FileHealthCheckerWindow *fileHealthWindow = new FileHealthCheckerWindow(*currentPat, nullptr);
            int res = fileHealthWindow->exec();
            if(res == 1)
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
            delete fileHealthWindow;
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
        int result = PrepareSingleFiles();
        if(result != -1)
        {
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
            QMessageBox::critical(this, "No Files Selected", "You need to select at least one eeg file in the user interface");
        }
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
        std::vector<InsermLibrary::FileType> filePriority = std::vector<InsermLibrary::FileType>(m_GeneralOptionsFile->FileExtensionsFavorite());

        //Should probably senbd back the struct here and not keep a global variable
        m_MultipleSubjects = PrepareDBFolders();
        if(m_MultipleSubjects.size() > 0)
        {
            FileHealthCheckerWindow *fileHealthWindow = new FileHealthCheckerWindow(m_MultipleSubjects, nullptr);
            int res = fileHealthWindow->exec();
            if(res == 1)
            {
                InitMultiSubjectProgresBar(m_MultipleSubjects);

                thread = new QThread;
                worker = new MultiSubjectWorker(m_MultipleSubjects, analysisOptions, optstat, optpic, filePriority, PtsFilePath);

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
                connect(worker, &IWorker::finished, this, &Localizer::CleanUpAfterMultiSubjectAnalysis);

                //=== Launch Thread and lock possible second launch
                worker->moveToThread(thread);
                thread->start();
                isAlreadyRunning = true;
            }
            delete fileHealthWindow;
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

void Localizer::ProcessBidsSubjectAnalysis()
{
    if (isAlreadyRunning)
    {
        QMessageBox::critical(this, "Analysis already running", "Please wait until the current analysis if finished");
        return;
    }

    //Get info from ui and check that at least one task can be analyzed
    InsermLibrary::picOption optpic = picOpt->getPicOption();
    InsermLibrary::statOption optstat = optStat->getStatOption();
    InitProgressBar();
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();
    //for now no file priority, if we add edf and/or other we will put the system back in place

    int result = PrepareBidsSubjectFolder();
    if(result == -1)
    {
        QMessageBox::critical(this, "No Tasks Selected", "You need to select at least one task to process in the user interface");
        return;
    }

    thread = new QThread;
    worker = new BidsSubjectWorker(workingCopy, analysisOptions, optstat, optpic, PtsFilePath);

    //=== Event update displayer
    connect(worker, &IWorker::sendLogInfo, this, &Localizer::DisplayLog);
    connect(worker->GetLoca(), &InsermLibrary::LOCA::sendLogInfo, this, &Localizer::DisplayLog);
    //connect(worker->GetLoca(), &InsermLibrary::LOCA::incrementAdavnce, this, &Bidsalyzer::UpdateProgressBar);

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

void Localizer::UpdateProgressBar(int advancement)
{
    nbDoneTask += advancement;
    ui.progressBar->setValue((nbDoneTask / nbTaskToDo) * 100);
}

void Localizer::CancelAnalysis()
{
    if (isAlreadyRunning)
    {
        m_lockLoop.lockForWrite();
        thread->quit();
        thread->wait();
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
    if(QFileInfo::exists(ConnectCleanerFile.c_str()) && m_CCFToggle)
    {
        std::vector<std::string> uncorrectedLabels;
        std::vector<int> states;
        std::vector<std::string> correctedLabels;
        LoadCCFFile(ConnectCleanerFile.c_str(), uncorrectedLabels, states, correctedLabels);
        worker->SetExternalParameters(states, correctedLabels, 1);
        emit MontageDone(1);
    }
    else
    {
        ConnectCleaner *elecWin = new ConnectCleaner(ElectrodeList, ConnectCleanerFile.c_str(), nullptr);
        int res = elecWin->exec();
        if(res == 1)
        {
            worker->SetExternalParameters(elecWin->IndexToDelete(), elecWin->CorrectedLabel(), elecWin->OperationToDo());
            emit MontageDone(res);
        }
        else
        {
            CancelAnalysis();
        }
        delete elecWin;
    }
}

void Localizer::LoadCCFFile(std::string path, std::vector<std::string> & uncorrectedLabels, std::vector<int> & states, std::vector<std::string> & correctedLabels)
{
    std::vector<std::string> rawFile = EEGFormat::Utility::ReadTextFile(path);
    for (uint i = 0; i < rawFile.size(); i++)
    {
        std::vector<std::string> rawLine = EEGFormat::Utility::Split<std::string>(rawFile[i], " ");
        if(rawLine.size() != 3)
        {
            std::cerr << "Error, line from connect cleaner file should contain three elements" << std::endl;
            continue;
        }

        if(std::stoi(rawLine[0]) == 0)
        {
            states.push_back(i);
        }
        uncorrectedLabels.push_back(rawLine[1]);
        correctedLabels.push_back(rawLine[2]);
    }
}

void Localizer::CleanUpAfterMultiSubjectAnalysis()
{
    for(int i = 0; i < m_MultipleSubjects.size(); i++)
    {
        delete m_MultipleSubjects[i];
    }
    m_MultipleSubjects.clear();

    isAlreadyRunning = false;
}
