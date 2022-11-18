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
//#include "patientFolder.h"
#include "SubjectFolder.h"
#include "singleFile.h"
#include "optionsPerf.h"
#include "optionsStats.h"
#include "ConnectCleaner.h"
#include "GeneralOptionsWindow.h"
#include "picOptions.h"

//TODO DOING
#include "ProtocolsWindow.h"
//===
#include "FileHealthCheckerWindow.h"
#include "ProtocolWindow.h"
#include "ChooseLocaWindow.h"
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
    int PrepareSingleFiles();
    std::vector<SubjectFolder*> PrepareDBFolders();
	void InitProgressBar();
    void InitMultiSubjectProgresBar(std::vector<SubjectFolder*> subjects);
    std::vector<InsermLibrary::FrequencyBandAnalysisOpt> GetUIAnalysisOption();
	int GetSelectedFolderCount(QModelIndexList selectedIndexes);

private slots:
	void SetLabelCount(int count);
	void ModelClicked(const QModelIndex &current);
	void ShowFileTreeContextMenu(QPoint point);
	void SelectPtsForCorrelation();
	void ClearPtsForCorrelation();
    void DealWithCCFToggle();
	void ToggleAllBands();
    void ProcessFolderAnalysis();
    void ProcessSingleAnalysis();
    void ProcessMultiFolderAnalysis();
    void ProcessERPAnalysis(QList<QString> examCorrespondance);
    void ProcessFileConvertion(QList<QString> newFileType);
	void ProcessMicromedFileConcatenation(QList<QString> files, QString directoryPath, QString fileName);
    void DisplayLog(QString info);
    void DisplayColoredLog(QString info, QColor color = QColor(Qt::GlobalColor::black));
    void UpdateProgressBar(int advancement);
	void CancelAnalysis();
    void ReceiveElectrodeList(std::vector<std::string> ElectrodeList, std::string ConnectCleanerFile);
    void LoadCCFFile(std::string path, std::vector<std::string> & uncorrectedLabels, std::vector<int> & states, std::vector<std::string> & correctedLabels);
    void CleanUpAfterMultiSubjectAnalysis();

signals:
    void MontageDone(int);
    void BypassCCF();

private:
	//==Visualisation
	QFileSystemModel *m_localFileSystemModel = nullptr;
	//==Data for analysis
	InsermLibrary::GeneralOptionsFile *m_GeneralOptionsFile = nullptr;
    InsermLibrary::FrequencyFile m_frequencyFile;
	bool m_isPatFolder = false;
	//userOption userOpt;
    //patientFolder* currentPat = nullptr;
    SubjectFolder* currentPat = nullptr;
	std::vector<singleFile> currentFiles;
    std::vector<SubjectFolder*> m_MultipleSubjects;
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
	//==
	Ui::LocalizerClass ui;
	QStringList inputArguments;

    bool m_CCFToggle = false;
	ErpProcessor* erpWindow = nullptr;
};

#endif // LOCALIZER_H
