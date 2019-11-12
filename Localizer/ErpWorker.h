#ifndef ERPWORKER_H
#define ERPWORKER_H

#include "IWorker.h"
#include "optionsParameters.h" //pour stat et pic options, a separer
#include <QCoreApplication>

class ErpWorker : public IWorker
{
	Q_OBJECT

public:
    ErpWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& provFiles, InsermLibrary::picOption picOption);
	~ErpWorker();
	void Process();

private:
    InsermLibrary::eegContainer* ExtractData(std::string currentFile);

private:
    std::vector<std::string> m_EegFiles;
    std::vector<std::string> m_ProvFiles;
};

#endif
