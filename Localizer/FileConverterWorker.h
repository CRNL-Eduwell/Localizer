#ifndef FILECONVERTERWORKER_H
#define FILECONVERTERWORKER_H

#include <algorithm>
#include <string>
#include "../../EEGFormat/EEGFormat/Wrapper.h"
#include "IWorker.h"

#include <QCoreApplication>

class FileConverterWorker : public IWorker
{
	Q_OBJECT

public:
	FileConverterWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& convertionType);
	~FileConverterWorker();
	void Process();

private:
	std::vector<std::string> m_eegFiles;
	std::vector<std::string> m_convertionType;
};

#endif