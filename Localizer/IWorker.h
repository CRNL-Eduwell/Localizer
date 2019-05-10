#ifndef IWORKER_H
#define IWORKER_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QThread>

#include "Wrapper.h"
#include "eegContainer.h"

#include "LOCA.h"

class IWorker : public QObject
{
	Q_OBJECT

public:
	IWorker();
	~IWorker();
	inline InsermLibrary::LOCA* GetLoca() { return m_loca; }

protected:
	eegContainer* GetEegContainer(std::string currentFilePath, bool shouldExtractData, int nbFreqBand);

private:

public slots:
	virtual void Process() = 0;

signals:
	void sendLogInfo(QString);
	void finished();
	void sendContainerPointer(eegContainer *eegCont);

public:
	int bipCreated = -1;

protected:
	std::vector<int> m_electrodeToDeleteMemory;
	InsermLibrary::LOCA *m_loca = nullptr;

private:

};


#endif