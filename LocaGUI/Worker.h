#ifndef WORKER_H
#define WORKER_H

#include <QtWidgets/QMainWindow>
#include <QDir>
#include "LOCA.h"

class Worker : public QObject 
{
	Q_OBJECT

public:
	Worker(std::vector<std::vector<double>> p_freqBandValue, std::vector <std::vector<bool>> p_anaDetails, std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks);
	~Worker();
	InsermLibrary::LOCA * returnLoca();

public slots:
	void process();
	void BipDoneeee(bool);
signals:
	void finished();
	void error(QString err);
	void sendLogInfo(QString);
	void sendElanPointer(InsermLibrary::ELAN *p_elan);
	
private:
	InsermLibrary::LOCA *loca = nullptr;
	bool bip = false;
	InsermLibrary::LOCAANALYSISOPTION **locaAnaOpt = nullptr;
	InsermLibrary::ELAN *elan = nullptr;
	int numberFiles = 0;
};

#endif // WORKER_H
