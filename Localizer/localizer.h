#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QtWidgets/QMainWindow>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QShortcut>

#include <QtUiTools>
#include <QUiLoader>

#include "Utility.h"
#include "optionsParameters.h"
#include "patientFolder.h"
#include "singleFile.h"
#include "optionsPerf.h"
#include "optionsStats.h"
#include "freqwidget.h"
#include "chooseElec.h"
#include "picOptions.h"
#include "form.h"
#include "AboutDycog.h"
#include "concatenator.h"
//#include "Worker.h"
#include "LOCA.h"
#include "ui_localizer.h"
#include <QCoreApplication>
#include "DeselectableTreeView.h"
#include "FrequencyBand.h"
#include "FrequencyFile.h"
#include "FrequencyBandAnalysisOpt.h"
#include "IWorker.h"
#include "PatientFolderWorker.h"
#include "SingleFilesWorker.h"
#include "ErpWorker.h"
#include "FileConverterWorker.h"
#include <QListWidgetItem>
#include "../../EEGFormat/EEGFormat/Utility.h"
#include "ErpProcessor.h"
#include "FileConverterProcessor.h"

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
	void DisplayLog(QString info, Qt::GlobalColor color = Qt::GlobalColor::black);
	void UpdateProgressBar(int divider);
	void CancelAnalysis();
	void receiveContainerPointer(eegContainer *eegCont);
	void loadConcat();

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
	concatenator *concatFiles = nullptr;

	Ui::LocalizerClass ui;
	QStringList inputArguments;
};

#endif // LOCALIZER_H