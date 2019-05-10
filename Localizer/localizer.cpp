#include "localizer.h"

Localizer::Localizer(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	reSetupGUI();
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

void Localizer::reSetupGUI()
{
	for (int i = 0; i < userOpt.freqOption.frequencyBands.size(); i++)
	{
		ui.freqTabWidget->addTab(new freqwidget(), QString(userOpt.freqOption.frequencyBands[i].freqName.c_str()));
	}
	optPerf = new optionsPerf();
	optStat = new optionsStats();
	picOpt = new picOptions();
	optLoca = new form();
	getUIelement();
	ui.progressBar->reset();
}

void Localizer::getUIelement()
{
	uiElement = new uiUserElement();
	for (int i = 0; i < ui.freqTabWidget->count(); i++)
	{
		uiElement->eeg2envCheckBox.push_back(ui.freqTabWidget->widget(i)->findChild<QCheckBox *>(QString("eeg2envCheckBox"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm0Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm0Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm250Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm250Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm500Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm500Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm1000Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm1000Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm2500Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm2500Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->sm5000Frame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("sm5000Frame"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->dsPOSFrame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("posDSFrame"), Qt::FindChildOption::FindChildrenRecursively));
		//===
		uiElement->trialmapGroupBox.push_back(ui.freqTabWidget->widget(i)->findChild<QGroupBox *>(QString("trialmapGroupBox"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->trialmapCheckBox.push_back(ui.freqTabWidget->widget(i)->findChild<QCheckBox *>(QString("trialMapCheckBox"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->trialmapFrame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("trialMapFrame"), Qt::FindChildOption::FindChildrenRecursively));
		//===
		uiElement->env2plotGroupBox.push_back(ui.freqTabWidget->widget(i)->findChild<QGroupBox *>(QString("env2plotGroupBox"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->env2plotCheckBox.push_back(ui.freqTabWidget->widget(i)->findChild<QCheckBox *>(QString("env2plotCheckBox"), Qt::FindChildOption::FindChildrenRecursively));
		uiElement->env2plotFrame.push_back(ui.freqTabWidget->widget(i)->findChild<QFrame *>(QString("env2plotFrame"), Qt::FindChildOption::FindChildrenRecursively));
	}
}

void Localizer::deactivateUISingleFiles()
{
	for (int i = 0; i < ui.freqTabWidget->count(); i++)
	{
		uiElement->trialmapGroupBox[i]->setEnabled(false);
		uiElement->env2plotGroupBox[i]->setEnabled(false);
	}
}

void Localizer::connectSignals()
{
	connectMenuBar();

	ui.FileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.FileTreeView, &QTreeView::customContextMenuRequested, this, &Localizer::ShowFileTreeContextMenu);
	connect((DeselectableTreeView*)ui.FileTreeView, &DeselectableTreeView::ResetNbFolder, this, [&]() { SetFolderLabelCount(0); });

	connect(ui.sameanaCheckBox, SIGNAL(clicked()), this, SLOT(linkFreqCheckBox()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelAnalysis()));
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
			currentFiles.push_back(singleFile(fileNames[i].toStdString(), ui.freqTabWidget->count()));

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
	updateGUIFrame(pat->localizerFolder()[0]);
	ui.FolderCountLabel->setText("0 patient folders selected for Analysis");
}

void Localizer::LoadTreeView(vector<singleFile> currentFiles)
{
	deactivateUISingleFiles();
	disconnect(ui.processButton, 0, 0, 0);
	if (currentFiles.size() > 0)
	{
		QString initialFolder = currentFiles[0].rootFolder().c_str();
		LoadTreeViewUI(initialFolder);
		connect(ui.processButton, SIGNAL(clicked()), this, SLOT(processSingleAnalysis()));
		updateGUIFrame(currentFiles[0]);
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

void Localizer::updateGUIFrame(locaFolder currentLoca)
{
	for (int i = 0; i < ui.freqTabWidget->count(); i++)
	{
		if (currentLoca.frequencyFolders().size() == 0)
		{
			updateQFrame("", uiElement->sm0Frame[i]);
			updateQFrame("", uiElement->sm250Frame[i]);
			updateQFrame("", uiElement->sm500Frame[i]);
			updateQFrame("", uiElement->sm1000Frame[i]);
			updateQFrame("", uiElement->sm2500Frame[i]);
			updateQFrame("", uiElement->sm5000Frame[i]);
			updateQFrame("", uiElement->dsPOSFrame[i]);
			//===
			updateQFrame("", uiElement->trialmapFrame[i]);
			//===
			updateQFrame("", uiElement->env2plotFrame[i]);
		}
		else
		{
			for (int j = 0; j < currentLoca.frequencyFolders().size(); j++)
			{
				if (currentLoca.frequencyFolders()[j].frequencyName() == userOpt.freqOption.frequencyBands[i].freqFolderName)
				{
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM0_ELAN), uiElement->sm0Frame[i]);
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM250_ELAN), uiElement->sm250Frame[i]);
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM500_ELAN), uiElement->sm500Frame[i]);
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM1000_ELAN), uiElement->sm1000Frame[i]);
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM2500_ELAN), uiElement->sm2500Frame[i]);
					updateQFrame(currentLoca.frequencyFolders()[j].filePath(SM5000_ELAN), uiElement->sm5000Frame[i]);
					updateQFrame(currentLoca.filePath(POS_DS_ELAN), uiElement->dsPOSFrame[i]);

					//===
					if (currentLoca.frequencyFolders()[j].hasTrialMap())
					{
						updateQFrame("trigg", uiElement->trialmapFrame[i]);
					}
					//===
					if (currentLoca.frequencyFolders()[j].hasEnvBar())
					{
						updateQFrame("trigg", uiElement->env2plotFrame[i]);
					}
				}
			}
		}
	}
}

void Localizer::updateGUIFrame(singleFile currentFiles)
{
	for (int i = 0; i < currentFiles.frequencyFolders().size(); i++)
	{
		updateQFrame(currentFiles.frequencyFolders()[i].sm0eeg, uiElement->sm0Frame[i]);
		updateQFrame(currentFiles.frequencyFolders()[i].sm250eeg, uiElement->sm250Frame[i]);
		updateQFrame(currentFiles.frequencyFolders()[i].sm500eeg, uiElement->sm500Frame[i]);
		updateQFrame(currentFiles.frequencyFolders()[i].sm1000eeg, uiElement->sm1000Frame[i]);
		updateQFrame(currentFiles.frequencyFolders()[i].sm2500eeg, uiElement->sm2500Frame[i]);
		updateQFrame(currentFiles.frequencyFolders()[i].sm5000eeg, uiElement->sm5000Frame[i]);
	}
}

void Localizer::updateQFrame(string fileLooked, QFrame *frameFile)
{
	QString styleSheetRed("background:rgb(255, 0, 0)");
	QString styleSheetGreen("background:rgb(0, 255, 0)");

	if (fileLooked != "")
		frameFile->setStyleSheet(styleSheetGreen);
	else
		frameFile->setStyleSheet(styleSheetRed);
}

void Localizer::reInitStructFolder()
{
	if (savePat == nullptr)
		savePat = new patientFolder(currentPat);

	if (userOpt.anaOption.size() == currentPat->localizerFolder().size())
	{
		deleteAndNullify1D(currentPat);
		currentPat = new patientFolder(savePat);
	}
}

void Localizer::reInitStructFiles()
{
	if (saveFiles.size() == 0)
		saveFiles = vector<singleFile>(currentFiles);

	if (userOpt.anaOption.size() == saveFiles.size())
	{
		saveFiles.clear();
		saveFiles = vector<singleFile>(currentFiles);
	}
}

void Localizer::reInitProgressBar(userOption *optionUser)
{
	nbTaskToDo = 0;
	for (int i = 0; i < optionUser->anaOption.size(); i++)
	{
		if (optionUser->anaOption[i].localizer)
		{
			for (int j = 0; j < optionUser->anaOption[i].anaOpt.size(); j++)
			{
				bool processFreq = optionUser->anaOption[i].anaOpt[j].eeg2env;
				bool processEnv = optionUser->anaOption[i].anaOpt[j].env2plot;
				bool processMap = optionUser->anaOption[i].anaOpt[j].trialmat;

				if (processFreq || processEnv || processMap)
				{
					nbTaskToDo++; //eeg2env, wheter we need to compute or load
					if (processEnv) { nbTaskToDo++; }
					if (processMap) { nbTaskToDo++; }
				}
			}
		}
	}
	//cout << "[==============]" << endl;
	//cout << "Number of Loca : " << nbLoca << endl;
	//cout << "Number of tasks : " << nbTaskToDo << endl;

	ui.progressBar->reset();
	nbDoneTask = 0;
}

void Localizer::getUIAnalysisOption(patientFolder *pat)
{
	uiElement->analysis(userOpt.anaOption, pat->localizerFolder().size());
	getAnalysisCheckBox(userOpt.anaOption);
	deleteUncheckedFiles(userOpt.anaOption, pat);
	optStat->getStatOption(&userOpt.statOption);
	picOpt->getPicOption(&userOpt.picOption);
	optPerf->getPerfLoca(userOpt.locaPerf);
}

void Localizer::getUIAnalysisOption(vec1<singleFile> &files)
{
	uiElement->analysis(userOpt.anaOption, files.size());
	getAnalysisCheckBox(userOpt.anaOption);
	deleteUncheckedFiles(userOpt.anaOption, files);
}

void Localizer::getAnalysisCheckBox(vector<locaAnalysisOption> &anaOption)
{
	//QListWidget *a = ui.TRCListWidget->item(0)->listWidget();
	//for (int i = 0; i < a->count(); i++)
	//{
	//	if (a->item(i)->checkState() == Qt::CheckState::Checked)
	//		anaOption[i].localizer = true;
	//	else if (a->item(i)->checkState() == Qt::CheckState::Unchecked)
	//		anaOption[i].localizer = false;
	//}
}

void Localizer::deleteUncheckedFiles(vector<locaAnalysisOption> &anaOption, patientFolder *pat)
{
	int sizeLoca = anaOption.size() - 1;
	for (int i = sizeLoca; i >= 0; i--)
	{
		if (!anaOption[i].localizer)
		{
			pat->localizerFolder().erase(pat->localizerFolder().begin() + i);
			anaOption.erase(anaOption.begin() + i);
		}
	}
}

void Localizer::deleteUncheckedFiles(vector<locaAnalysisOption> &anaOption, vec1<singleFile> &files)
{
	int sizeLoca = anaOption.size() - 1;
	for (int i = sizeLoca; i >= 0; i--)
	{
		if (!anaOption[i].localizer)
		{
			files.erase(files.begin() + i);
			anaOption.erase(anaOption.begin() + i);
		}
	}
}

//=== Slots	
void Localizer::SetFolderLabelCount(int count)
{
	QString label = count > 1 ? "folder" : "folders";
	ui.FolderCountLabel->setText(QString::number(count) + " patient " + label + " selected for Analysis");
}

void Localizer::ModelClicked(const QModelIndex &current)
{
	int nbFolderSelected = 0;
	QModelIndexList selectedIndexes = ui.FileTreeView->selectionModel()->selectedRows();
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
	}

	SetFolderLabelCount(nbFolderSelected);
}

void Localizer::ShowFileTreeContextMenu(QPoint point)
{

}

void Localizer::checkMultipleItems(QListWidgetItem * item)
{
	QList<QListWidgetItem *> selecItem = item->listWidget()->selectedItems();

	for (int i = 0; i < selecItem.size(); i++)
	{
		selecItem.at(i)->setCheckState(item->checkState());
	}
}

void Localizer::checkOnEnter(QListWidgetItem * item)
{
	if (Qt::Key::Key_Return)
	{
		Qt::CheckState currentState = item->checkState();
		if (currentState == Qt::CheckState::Checked)
		{
			item->setCheckState(Qt::CheckState::Unchecked);
		}
		else
		{
			item->setCheckState(Qt::CheckState::Checked);
		}
	}
}

void Localizer::linkFreqCheckBox()
{
	if (ui.sameanaCheckBox->checkState() == Qt::CheckState::Checked)
	{
		for (int i = 0; i < ui.freqTabWidget->count(); i++)
		{
			connect(uiElement->eeg2envCheckBox[i], &QCheckBox::toggled, this, [&](bool newValue) {
				for (int j = 0; j < uiElement->eeg2envCheckBox.size(); j++)
				{
					uiElement->eeg2envCheckBox[j]->setChecked(newValue);
				}
			});

			connect(uiElement->trialmapCheckBox[i], &QCheckBox::clicked, this, [&](bool newValue) {
				for (int j = 0; j < uiElement->trialmapCheckBox.size(); j++)
				{
					uiElement->trialmapCheckBox[j]->setChecked(newValue);
				}
			});

			connect(uiElement->env2plotCheckBox[i], &QCheckBox::clicked, this, [&](bool newValue) {
				for (int j = 0; j < uiElement->env2plotCheckBox.size(); j++)
				{
					uiElement->env2plotCheckBox[j]->setChecked(newValue);
				}
			});
		}
	}
	else
	{
		for (int i = 0; i < ui.freqTabWidget->count(); i++)
		{
			disconnect(uiElement->eeg2envCheckBox[i], 0, 0, 0);
			disconnect(uiElement->trialmapCheckBox[i], 0, 0, 0);
			disconnect(uiElement->env2plotCheckBox[i], 0, 0, 0);
		}
	}
}

void Localizer::processFolderAnalysis()
{
	if (currentPat != nullptr)
	{
		if (!isAlreadyRunning)
		{
			ui.progressBar->reset();
			reInitStructFolder();
			getUIAnalysisOption(currentPat);
			reInitProgressBar(&userOpt);

			thread = new QThread;
			worker = new Worker(currentPat, &userOpt);

			//=== Event update displayer
			connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
			connect(worker->getLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
			connect(worker->getLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(updateProgressBar(int)));

			//=== 
			connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
			//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
			connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

			//=== Event From worker and thread
			connect(thread, SIGNAL(started()), worker, SLOT(processAnalysis()));
			connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
			connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(worker, &Worker::finished, this, [&] { isAlreadyRunning = false; });
			connect(worker, &Worker::finished, this, &Localizer::UpdateFolderPostAna);

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

void Localizer::processSingleAnalysis()
{
	if (currentFiles.size() > 0)
	{
		if (!isAlreadyRunning)
		{
			reInitStructFiles();
			getUIAnalysisOption(currentFiles);
			reInitProgressBar(&userOpt);

			thread = new QThread;
			worker = new Worker(currentFiles, &userOpt, -1);

			//=== Event update displayer
			connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
			connect(worker->getLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
			connect(worker->getLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(updateProgressBar(int)));

			//=== 
			connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
			//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
			connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

			//=== Event From worker and thread
			connect(thread, SIGNAL(started()), worker, SLOT(processAnalysis()));
			connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
			connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(worker, &Worker::finished, this, [&] { isAlreadyRunning = false; });
			connect(worker, &Worker::finished, this, &Localizer::UpdateSinglePostAna);

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

				picOpt->getPicOption(&userOpt.picOption);
				thread = new QThread;
				worker = new Worker(&currentPat->localizerFolder()[index.row()], &userOpt);

				//=== Event update displayer
				connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
				connect(worker->getLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
				connect(worker->getLoca(), SIGNAL(incrementAdavnce(int)), this, SLOT(updateProgressBar(int)));

				//=== 
				connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
				//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
				connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

				//=== Event From worker and thread
				connect(thread, SIGNAL(started()), worker, SLOT(processERP()));
				connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
				connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
				connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
				connect(worker, &Worker::finished, this, [&] { isAlreadyRunning = false; });

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

				if (currentPat != nullptr)
					worker = new Worker(&currentPat->localizerFolder()[index.row()], &userOpt);
				else if (currentFiles.size() > 0)
					worker = new Worker(currentFiles, &userOpt, index.row());

				//=== Event update displayer
				connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
				connect(worker->getLoca(), SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));

				//=== 
				connect(worker, SIGNAL(sendContainerPointer(eegContainer*)), this, SLOT(receiveContainerPointer(eegContainer*)));
				//connect(this, &Localizer::bipDone, worker, [&] { worker->bipCreated = true; });
				connect(this, &Localizer::bipDone, worker, [=](int status) { worker->bipCreated = status; });

				//=== Event From worker and thread
				connect(thread, SIGNAL(started()), worker, SLOT(processToELAN()));
				connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
				connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
				connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
				connect(worker, &Worker::finished, this, [&] { ui.progressBar->setValue(100); });
				connect(worker, &Worker::finished, this, [&] { isAlreadyRunning = false; });

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

void Localizer::displayLog(QString messageToDisplay)
{
	ui.messageDisplayer->append(messageToDisplay);
}

void Localizer::updateProgressBar(int divider)
{
	nbDoneTask = nbDoneTask + ((float)1 / divider);
	ui.progressBar->setValue((nbDoneTask / nbTaskToDo) * 100);
}

void Localizer::cancelAnalysis()
{
	if (isAlreadyRunning)
	{
		m_lockLoop.lockForWrite();
		thread->terminate();
		while (!thread->isFinished())
		{
		}
		isAlreadyRunning = false;
		QMessageBox::information(this, "Canceled", "Canceled");
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

void Localizer::UpdateFolderPostAna()
{
	deleteAndNullify1D(savePat);
	currentPat = new patientFolder(currentPat->rootFolder());

	updateGUIFrame(currentPat->localizerFolder()[0]);
	if (savePat != nullptr)
	{
		deleteAndNullify1D(savePat);
		savePat = new patientFolder(currentPat);
	}
}

void Localizer::UpdateSinglePostAna()
{
	vector<string> analyzedPath;
	for (int i = 0; i < currentFiles.size(); i++)
		analyzedPath.push_back(currentFiles[i].filePath(currentFiles[i].fileExtention()));

	if (currentFiles.size() > 0)
		currentFiles.clear();

	for (int i = 0; i < analyzedPath.size(); i++)
		currentFiles.push_back(singleFile(analyzedPath[i], ui.freqTabWidget->count()));

	if (saveFiles.size() > 0)
		saveFiles.clear();

	for (int i = 0; i < saveFiles.size(); i++)
		updateGUIFrame(saveFiles[i]);
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
		connect(concatFiles, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
		concatFiles->exec();
		deleteAndNullify1D(concatFiles);
		UpdateFolderPostAna();
	}
}