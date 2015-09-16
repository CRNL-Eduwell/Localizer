#ifndef WORKER_H
#define WORKER_H

#include <QtWidgets/QMainWindow>
#include "LOCA.h"

class Worker : public QObject 
{
	Q_OBJECT

public:
	Worker(std::vector<std::string> p_trc, std::vector<std::string> p_prov, std::string folderPatient, std::vector<std::string> p_tasks, std::vector<std::string> p_exptasks);
	~Worker();

public slots:
	void process();
	void BipDoneeee(bool);
signals:
	void finished();
	void error(QString err);
	void sendLogInfo(QString);
	void sendElanPointer(InsermLibrary::ELAN *p_elan);

private:
	std::vector<std::string> trcFiles, provFiles, tasks, expTasks;
	std::string patientFolder;
	bool bip = false;
};

#endif // WORKER_H
