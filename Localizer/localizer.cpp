#include "localizer.h"

Localizer::Localizer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	ReSetupGUI();
	connectSignals();

	inputArguments = QCoreApplication::arguments();
	if (inputArguments.count() > 1)
	{
		if (inputArguments[1].compare("MicromedExternalCall") == 0)
		{
			QString fileName = inputArguments[2];
			if (fileName != "")
			{
				if (currentFiles.size() > 0)
					currentFiles.clear();

				deleteAndNullify1D(currentPat);
				currentPat = new patientFolder(fileName.toStdString());
				LoadTreeView(currentPat);
			}
		}
	}
}

Localizer::~Localizer()
{

}

void Localizer::ReSetupGUI()
{
	m_frequencyFile = new FrequencyFile();
	m_frequencyFile->Load();
	LoadFrequencyBandsUI(m_frequencyFile->FrequencyBands());

	optPerf = new optionsPerf();
	optStat = new optionsStats();
	picOpt = new picOptions();
	optLoca = new form();

	ui.progressBar->reset();
}

void Localizer::LoadFrequencyBandsUI(const std::vector<FrequencyBand>& FrequencyBands)
{
	ui.FrequencyListWidget->clear();
	int FrequencyCount = FrequencyBands.size();
	for (int i = 0; i < FrequencyCount; i++)
	{
		QString Label = QString::fromStdString(FrequencyBands[i].Label());

		QListWidgetItem *currentBand = new QListWidgetItem(ui.FrequencyListWidget);
		currentBand->setText(Label);
		currentBand->setFlags(currentBand->flags() | Qt::ItemIsUserCheckable); // set checkable flag
		currentBand->setCheckState(Qt::Unchecked); // AND initialize check state
	}
}

void Localizer::DeactivateUIForSingleFiles()
{
	ui.Env2plotCheckBox->setEnabled(false);
	ui.TrialmatCheckBox->setEnabled(false);
}

void Localizer::connectSignals()
{
	connectMenuBar();

	ui.FileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.FileTreeView, &QTreeView::customContextMenuRequested, this, &Localizer::ShowFileTreeContextMenu);
	connect((DeselectableTreeView*)ui.FileTreeView, &DeselectableTreeView::ResetNbFolder, this, [&]() { SetFolderLabelCount(0); });

	connect(ui.AllBandsCheckBox, &QCheckBox::clicked, this, &Localizer::ToggleAllBands);
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(CancelAnalysis()));
}

void Localizer::connectMenuBar()
{
	//===Fichier
	QAction* openPatFolder = ui.menuFichier->actions().at(0);
	connect(openPatFolder, &QAction::triggered, this, [&] { loadPatientFolder(); });
	QAction* openSingleFile = ui.menuFichier->actions().at(1);
	connect(openSingleFile, &QAction::triggered, this, [&] { loadSingleFile(); });
	//===Outils
	QAction* openTRCSecator = ui.menuOutils->actions().at(0);
	connect(openTRCSecator, &QAction::triggered, this, [&] { QMessageBox::information(this, "Secator", "Something"); });
	QAction* openConcatenator = ui.menuOutils->actions().at(1);
	connect(openConcatenator, &QAction::triggered, this, &Localizer::loadConcat);
	//===Configuration
	QAction* openPerfMenu = ui.menuConfiguration->actions().at(0);
	connect(openPerfMenu, &QAction::triggered, this, [&] { optPerf->exec(); });
	QAction* openStatMenu = ui.menuConfiguration->actions().at(1);
	connect(openStatMenu, &QAction::triggered, this, [&] { optStat->exec(); });
	QAction* openPicMenu = ui.menuConfiguration->actions().at(2);
	connect(openPicMenu, &QAction::triggered, this, [&] { picOpt->exec(); });
	QAction* openLocaMenu = ui.menuConfiguration->actions().at(3);
	connect(openLocaMenu, &QAction::triggered, this, [&] { optLoca->exec(); });
	//===Aide
	QAction* openAbout = ui.menuAide->actions().at(0);
	connect(openAbout, &QAction::triggered, this, [&]
	{
		AboutDycog about;
		about.exec();
	});
}

void Localizer::loadPatientFolder()
{
	QFileDialog *fileDial = new QFileDialog(this);
	fileDial->setFileMode(QFileDialog::FileMode::DirectoryOnly);
	QString fileName = fileDial->getExistingDirectory(this, tr("Choose Patient Folder"));
	if (fileName != "")
	{
		if (currentFiles.size() > 0)
			currentFiles.clear();

		deleteAndNullify1D(currentPat);
		currentPat = new patientFolder(fileName.toStdString());
		LoadTreeView(currentPat);
	}
}

void Localizer::loadSingleFile()
{
	QFileDialog *fileDial = new QFileDialog(this);
	fileDial->setFileMode(QFileDialog::FileMode::ExistingFile);
	QStringList fileNames = fileDial->getOpenFileNames(this, tr("Choose File to analyse : "), tr("C:\\"), QString("*.trc *.eeg *.edf"));
	if (fileNames.count() > 0)
	{
		if (currentFiles.size() > 0)
			currentFiles.clear();

		deleteAndNullify1D(currentPat);
		for (int i = 0; i < fileNames.size(); i++)
			currentFiles.push_back(singleFile(fileNames[i].toStdString(), ui.FrequencyListWidget->count()));

		LoadTreeView(currentFiles);
	}
}

void Localizer::LoadTreeView(patientFolder *pat)
{
	disconnect(ui.processButton, 0, 0, 0);
	if (ui.FileTreeView->selectionModel() != nullptr)
		disconnect(ui.FileTreeView->selectionModel(), 0, 0, 0);

	QString initialFolder = pat->rootFolder().c_str();
	LoadTreeViewUI(initialFolder);

	//==[Event connected to model of treeview]
	connect(ui.FileTreeView, &QTreeView::clicked, this, &Localizer::ModelClicked);
	//==[Event for rest of UI]
	connect(ui.processButton, SIGNAL(clicked()), this, SLOT(processFolderAnalysis()));
	ui.FolderCountLabel->setText("0 patient folders selected for Analysis");
}

void Localizer::LoadTreeView(vector<singleFile> currentFiles)
{
	DeactivateUIForSingleFiles();
	disconnect(ui.processButton, 0, 0, 0);
	if (currentFiles.size() > 0)
	{
		QString initialFolder = currentFiles[0].rootFolder().c_str();
		LoadTreeViewUI(initialFolder);
		connect(ui.processButton, SIGNAL(clicked()), this, SLOT(processSingleAnalysis()));
	}
	else
	{
		QMessageBox::information(this, "Are you sure ? ", "There is no file in this folder.");
	}
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

void Localizer::PreparePatientFolder()
{
	//Get row id's and sort them because they can be selected in whatever order
	QModelIndex rootIndex = ui.FileTreeView->rootIndex();
	int ExamCount = m_localFileSystemModel->rowCount(rootIndex);

	std::vector<pair<int, bool>> sortedIdToKeep;
	for (int i = 0; i < ExamCount; i++)
	{
		sortedIdToKeep.push_back(make_pair(i,false));
	}
	QModelIndexList selectedRows = ui.FileTreeView->selectionModel()->selectedRows();
	for (int i = 0; i < selectedRows.size(); i++)
	{
		for (int j = 0; j < sortedIdToKeep.size(); j++)
		{
			if (selectedRows[i].row() == sortedIdToKeep[j].first)
			{
				sortedIdToKeep[j].second = true;
				continue;
			}
		}
	}
	std::sort(sortedIdToKeep.begin(), sortedIdToKeep.end(), [&](pair<int, bool> a, pair<int, bool> b)
	{
		return a.first < b.first;
	});

	//Create data structure
	qDebug() << "Root path : " << m_localFileSystemModel->rootPath();
	deleteAndNullify1D(currentPat);
	currentPat = new patientFolder(m_localFileSystemModel->rootPath().toStdString());

	//Remove unwanted exam
	for (int i = ExamCount - 1; i >= 0; i--)
	{
		if (sortedIdToKeep[i].second == false)
		{
			currentPat->localizerFolder().erase(currentPat->localizerFolder().begin() + sortedIdToKeep[i].first);
		}
	}
}

//TODO
void Localizer::PrepareSingleFiles()
{
	//if (saveFiles.size() == 0)
	//	saveFiles = vector<singleFile>(currentFiles);

	//if (userOpt.anaOption.size() == saveFiles.size())
	//{
	//	saveFiles.clear();
	//	saveFiles = vector<singleFile>(currentFiles);
	//}
}

void Localizer::InitProgressBar()
{
	ui.progressBar->reset();
	nbDoneTask = 0;
	nbTaskToDo = 0;

	int nbFolderSelected = GetNbPatientFolder(ui.FileTreeView->selectionModel()->selectedRows());
	int nbFrequencyBands = ui.FrequencyListWidget->selectionModel()->selectedRows().size();

	ui.Eeg2envCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo++; //eeg2env, wheter we need to compute or load
	ui.Env2plotCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;
	ui.TrialmatCheckBox->isChecked() ? nbTaskToDo++ : nbTaskToDo;

	nbTaskToDo *= nbFolderSelected * nbFrequencyBands;
}

std::vector<FrequencyBandAnalysisOpt> Localizer::GetUIAnalysisOption()
{
	std::vector<FrequencyBand> frequencyBands = m_frequencyFile->FrequencyBands();
	std::vector<int> indexes;
	for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
	{
		if (ui.FrequencyListWidget->item(i)->checkState() == Qt::CheckState::Checked)
			indexes.push_back(i);
	}
	std::vector<FrequencyBandAnalysisOpt> analysisOpt = std::vector<FrequencyBandAnalysisOpt>(indexes.size());

	for (int i = 0; i < indexes.size(); i++)
	{
		//	- env2plot and / or trial mat
		analysisOpt[i].eeg2env2 = ui.Eeg2envCheckBox->isChecked();
		analysisOpt[i].env2plot = ui.Env2plotCheckBox->isChecked();
		analysisOpt[i].trialmat = ui.TrialmatCheckBox->isChecked();

		//	- what frequency bands data is 
		QString label = ui.FrequencyListWidget->item(indexes[i])->text();
		std::vector<FrequencyBand>::iterator it = std::find_if(frequencyBands.begin(), frequencyBands.end(), [&](const FrequencyBand &c) 
		{
			return (c.Label() == label.toStdString()); 
		});
		if (it != frequencyBands.end())
		{
			int index = distance(it, frequencyBands.begin());
			analysisOpt[i].Band = FrequencyBand(frequencyBands[indexes[i]]);
		}
	}

	return analysisOpt;
}

int Localizer::GetNbPatientFolder(QModelIndexList selectedIndexes)
{
	int nbFolderSelected = 0;
	for (int i = 0; i < selectedIndexes.size(); i++)
	{
		bool isRoot = selectedIndexes[i].parent() == ui.FileTreeView->rootIndex();
		if (m_localFileSystemModel->isDir(selectedIndexes[i]) && isRoot)
		{
			if (ui.FileTreeView->selectionModel()->isSelected(selectedIndexes[i]))
				nbFolderSelected++;
			else
				nbFolderSelected--;
		}
		else
		{
			ui.FileTreeView->selectionModel()->setCurrentIndex(selectedIndexes[i], QItemSelectionModel::Deselect);
		}
	}
	return nbFolderSelected;
}

//=== Slots	
void Localizer::SetFolderLabelCount(int count)
{
	QString label = count > 1 ? "folder" : "folders";
	ui.FolderCountLabel->setText(QString::number(count) + " patient " + label + " selected for Analysis");
}

void Localizer::ModelClicked(const QModelIndex &current)
{
	QModelIndexList selectedIndexes = ui.FileTreeView->selectionModel()->selectedRows();
	int nbFolderSelected = GetNbPatientFolder(selectedIndexes);
	SetFolderLabelCount(nbFolderSelected);
}

void Localizer::ShowFileTreeContextMenu(QPoint point)
{

}

void Localizer::ToggleAllBands()
{
	Qt::CheckState status = ui.AllBandsCheckBox->checkState();
	for (int i = 0; i < ui.FrequencyListWidget->count(); i++)
	{
		ui.FrequencyListWidget->item(i)->setCheckState(status);
	}
}

void Localizer::processFolderAnalysis()
{
	if (currentPat != nullptr)
	{
		if (!isAlreadyRunning)
		{
			//Data Struct info
			picOption optpic = picOpt->getPicOption();
			statOption optstat = optStat->getStatOption();

			//UI
			InitProgressBar();
			std::vector<FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();

			//Should probably senbd back the struct here and not keep a global variable
			PreparePatientFolder();
			thread = new QThread;
			worker = new PatientFolderWorker(*currentPat, analysisOptions, optstat, optpic);

			//=== Event update displayer
			connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
			connect(worker->GetLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
			connect(worker->GetLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(UpdateProgressBar(int)));

			//=== 
			connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
			connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

			//=== Event From worker and thread
			connect(thread, SIGNAL(started()), worker, SLOT(Process()));
			connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
			connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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
	else
	{
		QMessageBox::information(this, "Error", "Load Data ...");
	}
}

//TODO
void Localizer::processSingleAnalysis()
{
	if (currentFiles.size() > 0)
	{
		if (!isAlreadyRunning)
		{
			InitProgressBar();
			std::vector<FrequencyBandAnalysisOpt> analysisOptions = GetUIAnalysisOption();
			PrepareSingleFiles();

			thread = new QThread;
			// ATTENTION CA NE MARCHE PLUS ICI 
			//worker = new Worker(currentFiles, &userOpt, -1);
			worker = nullptr;

			//=== Event update displayer
			connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
			connect(worker->GetLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
			connect(worker->GetLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(UpdateProgressBar(int)));

			//=== 
			connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
			//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
			connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

			//=== Event From worker and thread
			connect(thread, SIGNAL(started()), worker, SLOT(processAnalysis()));
			connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
			connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });
			//connect(worker, &IWorker::finished, this, &Localizer::UpdateSinglePostAna);

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
	else
	{
		QMessageBox::information(this, "Error", "Load Data ...");
	}
}

//TODO
void Localizer::processERPAnalysis()
{
	if (currentPat != nullptr || currentFiles.size() > 0)
	{
		if (!isAlreadyRunning)
		{
			QModelIndex index = QModelIndex();// ui.TRCListWidget->currentIndex();
			if (index.isValid())
			{
				nbTaskToDo = 1;
				nbDoneTask = 0;
				ui.progressBar->reset();

				picOption opt = picOpt->getPicOption();
				thread = new QThread;

				// ATTENTION CA NE MARCHE PLUS ICI 
				//worker = new Worker(&currentPat->localizerFolder()[index.row()], &userOpt);
				worker = nullptr;

				//=== Event update displayer
				connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
				connect(worker->GetLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
				connect(worker->GetLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(UpdateProgressBar(int)));

				//=== 
				connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
				//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
				connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

				//=== Event From worker and thread
				connect(thread, SIGNAL(started()), worker, SLOT(processERP()));
				connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
				connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
				connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
				connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

				//=== Launch Thread and lock possible second launch
				worker->moveToThread(thread);
				thread->start();
				isAlreadyRunning = true;
			}
			else
			{
				QMessageBox::information(this, "Error", "Click On A File before having maps");
			}
		}
		else
		{
			QMessageBox::information(this, "Error", "Analysis already running");
		}
	}
	else
	{
		QMessageBox::information(this, "Error", "Load Data ...");
	}
}

//TODO
void Localizer::processConvertToElan()
{
	if (currentPat != nullptr || currentFiles.size() > 0)
	{
		if (!isAlreadyRunning)
		{
			QModelIndex index = QModelIndex();// ui.TRCListWidget->currentIndex();
			if (index.isValid())
			{
				nbTaskToDo = 1;
				nbDoneTask = 0;
				ui.progressBar->reset();

				thread = new QThread;

				// ATTENTION CA NE MARCHE PLUS ICI 
				worker = nullptr;
				//if (currentPat != nullptr)
				//	worker = new Worker(&currentPat->localizerFolder()[index.row()], &userOpt);
				//else if (currentFiles.size() > 0)
				//	worker = new Worker(currentFiles, &userOpt, index.row());

				//=== Event update displayer
				connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
				connect(worker->GetLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));

				//=== 
				connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
				//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
				connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

				//=== Event From worker and thread
				connect(thread, SIGNAL(started()), worker, SLOT(processToELAN()));
				connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
				connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
				connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
				connect(worker, &IWorker::finished, this, [&] { ui.progressBar->setValue(100); });
				connect(worker, &IWorker::finished, this, [&] { isAlreadyRunning = false; });

				//=== Launch Thread and lock possible second launch
				worker->moveToThread(thread);
				thread->start();
				isAlreadyRunning = true;
			}
			else
			{
				QMessageBox::information(this, "Error", "Click On A File before converting it");
			}
		}
		else
		{
			QMessageBox::information(this, "Error", "Analysis already running");
		}
	}
	else
	{
		QMessageBox::information(this, "Error", "Load Data ...");
	}
}

void Localizer::DisplayLog(QString messageToDisplay, Qt::GlobalColor color)
{
	ui.messageDisplayer->setTextColor(color);
	ui.messageDisplayer->append(messageToDisplay);
}

void Localizer::UpdateProgressBar(int divider)
{
	nbDoneTask = nbDoneTask + ((float)1 / divider);
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
		m_lockLoop.unlock();
	}
}

void Localizer::receiveContainerPointer(eegContainer *eegCont)
{
	chooseElec *elecWin = new chooseElec(eegCont, 0);
	int res = elecWin->exec();
	//eegCont->bipolarizeData();
	emit bipDone(res);
	delete elecWin;
}

void Localizer::loadConcat()
{
	QModelIndexList list = QModelIndexList(); // ui.TRCListWidget->selectionModel()->selectedIndexes();
	if (list.count() > 1 || list.count() == 0)
	{
		QMessageBox::information(this, "Error", "You Need To Select One Folder");
	}
	else
	{
		concatFiles = new concatenator(currentPat->localizerFolder()[list[0].row()].rootLocaFolder());
		connect(concatFiles, SIGNAL(sendLogInfo(QString)), this, SLOT(DisplayLog(QString)));
		concatFiles->exec();
		deleteAndNullify1D(concatFiles);
		//UpdateFolderPostAna();
	}
}