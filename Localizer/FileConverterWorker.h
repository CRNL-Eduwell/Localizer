#ifndef FILECONVERTERWORKER_H
#define FILECONVERTERWORKER_H

#include <algorithm>
#include <string>
#include "IWorker.h"

#include <QCoreApplication>

class FileConverterWorker : public IWorker
{
	Q_OBJECT

public:
	FileConverterWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& convertionType);
	~FileConverterWorker();
	void Process();
    void ExtractElectrodeList();

private:
    std::vector<std::string> m_EegFiles;
    std::vector<std::string> m_ConvertionType;
};

#endif
