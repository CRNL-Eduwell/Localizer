#ifndef ERPWORKER_H
#define ERPWORKER_H

#include <QCoreApplication>

#include "IWorker.h"
#include "optionsParameters.h" //pour stat et pic options, a separer

class ErpWorker : public IWorker
{
	Q_OBJECT

public:
	ErpWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& provFiles, picOption picOption);
	~ErpWorker();
	void Process();

private:
	eegContainer* ExtractData(std::string currentFile, int idFile);

private:
	std::vector<std::string> m_eegFiles;
	std::vector<std::string> m_provFiles;
};

#endif