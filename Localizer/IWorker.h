#ifndef IWORKER_H
#define IWORKER_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QThread>

#include "Wrapper.h"

#include "LOCA.h"

class IWorker : public QObject
{
	Q_OBJECT

public:
	IWorker();
	~IWorker();
    inline InsermLibrary::LOCA* GetLoca() { return m_Loca; }
    void ExtractElectrodeList(std::string currentFilePath);
    void SetExternalParameters(std::vector<int> IndexToDelete, std::vector<std::string> CorrectedLabels );

protected:
    InsermLibrary::eegContainer* GetEegContainer(std::string currentFilePath, bool shouldExtractData, int nbFreqBand);

private :
    void CorrectElectrodeLabels(EEGFormat::IFile* file);

public slots:
	virtual void Process() = 0;

signals:
	void finished();
    void sendLogInfo(QString);
	void incrementAdavnce(int divider);
    void sendElectrodeList(std::vector<std::string> ElectrodeList);

protected:
    InsermLibrary::LOCA *m_Loca = nullptr;
    std::vector<int> m_IndexToDelete;
    std::vector<std::string> m_CorrectedLabels;
};


#endif
