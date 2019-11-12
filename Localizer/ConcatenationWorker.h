#ifndef CONCATENATIONWORKER_H
#define CONCATENATIONWORKER_H

#include <QCoreApplication>
#include "IWorker.h"
#include "../../EEGFormat/EEGFormat/Utility.h"
#include "../../EEGFormat/EEGFormat/MicromedFile.h"

class ConcatenationWorker : public IWorker
{
	Q_OBJECT

public:
	ConcatenationWorker(std::vector<std::string>& trcFiles, std::string& directoryPath, std::string& fileName);
	~ConcatenationWorker();
	void Process();

private:
    std::vector<std::string> m_EegFiles;
    std::string m_DirectoryPath = "";
    std::string m_FileName = "";
};

#endif
