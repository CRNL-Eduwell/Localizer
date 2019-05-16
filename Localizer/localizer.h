#ifndef LOCALIZER_H
#define LOCALIZER_H

#include "Utility.h"
#include "../../EEGFormat/EEGFormat/Utility.h"

#include <QtWidgets/QMainWindow>
#include <QCoreApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QShortcut>
#include <QtUiTools>
#include <QUiLoader>
#include <QListWidgetItem>

//==Ui headers
#include "ui_localizer.h"
#include "DeselectableTreeView.h"
#include "optionsParameters.h"
#include "patientFolder.h"
#include "singleFile.h"
#include "optionsPerf.h"
#include "optionsStats.h"
#include "chooseElec.h"
#include "picOptions.h"
#include "form.h"
#include "AboutDycog.h"
#include "ErpProcessor.h"
#include "FileConverterProcessor.h"
#include "LOCA.h"
//==Tools & files headers
#include "FrequencyFile.h"
#include "FrequencyBand.h"
#include "FrequencyBandAnalysisOpt.h"
//==Workers
#include "IWorker.h"
#include "PatientFolderWorker.h"
#include "SingleFilesWorker.h"
#include "FileConverterWorker.h"
#include "ErpWorker.h"
#include "ConcatenationWorker.h"

using namespace std;

class Localizer : public QMainWindow
{
	Q_OBJECT

public:
	Localizer(QWidget *parent = 0);
	~Localizer();

private:
	void ReSetupGUI();
	void LoadFrequencyBandsUI(const std::vector<FrequencyBand>& FrequencyBands);
	void DeactivateUIForSingleFiles();
	void connectSignals();
	void connectMenuBar();
	void loadPatientFolder();
	void loadSingleFile();
	void LoadTreeView(patientFolder *pat);
	void LoadTreeView(vector<singleFile> currentFiles);
	void LoadTreeViewUI(QString initialFolder);
	void PreparePatientFolder();
	void PrepareSingleFiles();
	void InitProgressBar();
	std::vector<FrequencyBandAnalysisOpt> GetUIAnalysisOption();
	int GetNbElement(QModelIndexList selectedIndexes);

private slots:
	void SetLabelCount(int count);
	void ModelClicked(const QModelIndex &current);
	void ShowFileTreeContextMenu(QPoint point);
	void ToggleAllBands();
	void processFolderAnalysis();
	void processSingleAnalysis();
	void processERPAnalysis(QList<QString> examCorrespondance);
	void processFileConvertion(QList<QString> newFileType);
	void ProcessMicromedFileConcatenation(QList<QString> files, QString directoryPath, QString fileName);
	void DisplayLog(QString info, Qt::GlobalColor color = Qt::GlobalColor::black);
	void UpdateProgressBar(int divider);
	void CancelAnalysis();
	void receiveContainerPointer(eegContainer *eegCont);

signals:
	void bipDone(int);

private:
	//==Visualisation
	QFileSystemModel *m_localFileSystemModel = nullptr;
	//==Data for analysis
	FrequencyFile *m_frequencyFile = nullptr;
	bool m_isPatFolder = false;
	//userOption userOpt;
	patientFolder* currentPat = nullptr;
	vector<singleFile> currentFiles;
	//==Thread and Worker
	QReadWriteLock m_lockLoop;  
	QThread* thread = nullptr;
	IWorker* worker = nullptr;
	bool isAlreadyRunning = false;
	//==UI
	float nbDoneTask = 0;
	int nbTaskToDo = 0;
	picOptions *picOpt = nullptr;
	optionsStats *optStat = nullptr;
	optionsPerf *optPerf = nullptr;
	form *optLoca = nullptr;

	Ui::LocalizerClass ui;
	QStringList inputArguments;
};

#endif // LOCALIZER_H