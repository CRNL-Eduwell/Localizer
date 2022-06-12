#ifndef LOCALIZER_H
#define LOCALIZER_H

#include "Utility.h"
#include "../../EEGFormat/EEGFormat/Utility.h"
#include <iostream>
#include <string>

#include <QtWidgets/QMainWindow>
#include <QCoreApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QShortcut>
#include <QListWidgetItem>
#include <QFileSystemModel>
#include <QReadWriteLock>
#include <QInputDialog>

//==Ui headers
#include "ui_localizer.h"
#include "DeselectableTreeView.h"
#include "optionsParameters.h"
#include "patientFolder.h"
#include "singleFile.h"
#include "optionsPerf.h"
#include "optionsStats.h"
#include "ConnectCleaner.h"
#include "GeneralOptionsWindow.h"
#include "picOptions.h"
#include "ProtocolWindow.h"
#include "AboutDycog.h"
#include "ErpProcessor.h"
#include "FileConverterProcessor.h"
#include "LOCA.h"
//==Tools & files headers
#include "GeneralOptionsFile.h"
#include "FrequencyFile.h"
#include "FrequencyBand.h"
#include "FrequencyBandAnalysisOpt.h"
//==Workers
#include "IWorker.h"
#include "PatientFolderWorker.h"
#include "SingleFilesWorker.h"
#include "MultiSubjectWorker.h"
#include "FileConverterWorker.h"
#include "ErpWorker.h"
#include "ConcatenationWorker.h"

class Localizer : public QMainWindow
{
	Q_OBJECT

public:
    Localizer(QWidget *parent = nullptr);
	~Localizer();

private:
	void ReSetupGUI();
    void LoadFrequencyBandsUI(const std::vector<InsermLibrary::FrequencyBand>& FrequencyBands);
    void ResetUiCheckboxes();
	void DeactivateUIForSingleFiles();
    void ConnectSignals();
    void ConnectMenuBar();
    void LoadPatientFolder();
    void LoadSpecificFolder();
    void LoadDatabaseFolder();
    void LoadTreeViewFolder(QString rootFolder);
    void LoadTreeViewFiles(QString rootFolder);
    void LoadTreeViewDatabase(QString rootFolder);
	void LoadTreeViewUI(QString initialFolder);
    int PreparePatientFolder();
	void PrepareSingleFiles();
    std::vector<patientFolder> PrepareDBFolders();
	void InitProgressBar();
    void InitMultiSubjectProgresBar(std::vector<patientFolder> subjects);
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> GetUIAnalysisOption();
	int GetSelectedFolderCount(QModelIndexList selectedIndexes);

private slots:
	void SetLabelCount(int count);
	void ModelClicked(const QModelIndex &current);
	void ShowFileTreeContextMenu(QPoint point);
	void SelectPtsForCorrelation();
	void ClearPtsForCorrelation();
	void ToggleAllBands();
    void ProcessFolderAnalysis();
    void ProcessSingleAnalysis();
    void ProcessMultiFolderAnalysis();
    void ProcessERPAnalysis(QList<QString> examCorrespondance);
    void ProcessFileConvertion(QList<QString> newFileType);
	void ProcessMicromedFileConcatenation(QList<QString> files, QString directoryPath, QString fileName);
    void DisplayLog(QString info);
    void DisplayColoredLog(QString info, QColor color = QColor(Qt::GlobalColor::black));
	void UpdateProgressBar(int divider);
	void CancelAnalysis();
    void ReceiveElectrodeList(std::vector<std::string> ElectrodeList, std::string ConnectCleanerFile);

signals:
    void MontageDone(int);

private:
	//==Visualisation
	QFileSystemModel *m_localFileSystemModel = nullptr;
	//==Data for analysis
	InsermLibrary::GeneralOptionsFile *m_GeneralOptionsFile = nullptr;
    InsermLibrary::FrequencyFile *m_frequencyFile = nullptr;
	bool m_isPatFolder = false;
	//userOption userOpt;
	patientFolder* currentPat = nullptr;
	std::vector<singleFile> currentFiles;
	//==Thread and Worker
	QReadWriteLock m_lockLoop;  
	QThread* thread = nullptr;
	IWorker* worker = nullptr;
	bool isAlreadyRunning = false;
	//==UI
	float nbDoneTask = 0;
	float nbTaskToDo = 0;
	std::string PtsFilePath = "";
	picOptions *picOpt = nullptr;
	optionsStats *optStat = nullptr;
	optionsPerf *optPerf = nullptr;
	ProtocolWindow *optLoca = nullptr;
	GeneralOptionsWindow *generalOptionsWindow = nullptr;
	//==
	Ui::LocalizerClass ui;
	QStringList inputArguments;
};

#endif // LOCALIZER_H
