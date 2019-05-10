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
#include "Worker.h"
#include "LOCA.h"
#include "ui_localizer.h"
#include <QCoreApplication>
#include "DeselectableTreeView.h"

using namespace std;

class Localizer : public QMainWindow
{
	Q_OBJECT

public:
	Localizer(QWidget *parent = 0);
	~Localizer();

private:
	void reSetupGUI();
	void getUIelement();
	void deactivateUISingleFiles();
	void connectSignals();
	void connectMenuBar();
	void loadPatientFolder();
	void loadSingleFile();
	void LoadTreeView(patientFolder *pat);
	void LoadTreeView(vector<singleFile> currentFiles);
	void LoadTreeViewUI(QString initialFolder);
	void updateGUIFrame(locaFolder currentLoca);
	void updateGUIFrame(singleFile currentFiles);
	void updateQFrame(string fileLooked, QFrame *frameFile);
	void reInitStructFolder();
	void reInitStructFiles();
	void reInitProgressBar(userOption *optionUser);
	void getUIAnalysisOption(patientFolder *pat);
	void getUIAnalysisOption(vec1<singleFile> &files);
	void getAnalysisCheckBox(vector<locaAnalysisOption> &anaOption);
	void deleteUncheckedFiles(vector<locaAnalysisOption> &anaOption, patientFolder *pat);
	void deleteUncheckedFiles(vector<locaAnalysisOption> &anaOption, vec1<singleFile> &files);

private slots:
	void SetFolderLabelCount(int count);
	void ModelClicked(const QModelIndex &current);
	void ShowFileTreeContextMenu(QPoint point);
	void linkFreqCheckBox();
	void processFolderAnalysis();
	void processSingleAnalysis();
	void processERPAnalysis();
	void processConvertToElan();
	void displayLog(QString info);
	void updateProgressBar(int divider);
	void cancelAnalysis();
	void receiveContainerPointer(eegContainer *eegCont);
	void UpdateFolderPostAna();
	void UpdateSinglePostAna();

	void loadConcat();

signals:
	void bipDone(int);

private:
	//==Visualisation
	QFileSystemModel *m_localFileSystemModel = nullptr;
	//==Data for analysis
	userOption userOpt;
	patientFolder* currentPat = nullptr;
	patientFolder* savePat = nullptr;
	vector<singleFile> currentFiles;
	vector<singleFile> saveFiles;
	//==Thread and Worker
	QReadWriteLock m_lockLoop;  
	QThread* thread = nullptr;
	Worker* worker = nullptr;
	bool isAlreadyRunning = false;
	//==UI
	float nbDoneTask = 0;
	int nbTaskToDo = 0;
	picOptions *picOpt = nullptr;
	optionsStats *optStat = nullptr;
	optionsPerf *optPerf = nullptr;
	form *optLoca = nullptr;
	concatenator *concatFiles = nullptr;
	uiUserElement* uiElement = nullptr;
	Ui::LocalizerClass ui;
	QStringList inputArguments;
};

#endif // LOCALIZER_H