#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include "LOCA.h"

using namespace std;
using namespace InsermLibrary;

class Worker : public QObject 
{
	Q_OBJECT

public:
	Worker(OptionLOCA *p_LOCAOpt, vector<vector<double>> p_freqBandValue, vector <vector<bool>> p_anaDetails, vector<string> p_trc, vector<string> p_prov, string folderPatient, vector<string> p_tasks, vector<string> p_exptasks);
	~Worker();
	LOCA * returnLoca();

public slots:
	void process();
	void BipDoneeee(bool);
signals:
	void finished();
	void error(QString err);
	void sendLogInfo(QString);
	void sendElanPointer(ELAN *p_elan);
	void upScroll(int value);

private:
	LOCA *loca = nullptr;
	OptionLOCA *LOCAOpt = nullptr;
	bool bip = false;
	LOCAANALYSISOPTION **locaAnaOpt = nullptr;
	ELAN *elan = nullptr;
	int numberFiles = 0;
};

#endif // WORKER_H
