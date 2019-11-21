#include "FileConverterWorker.h"

FileConverterWorker::FileConverterWorker(std::vector<std::string>& eegFiles, std::vector<std::string>& convertionType)
{
    m_EegFiles = std::vector<std::string>(eegFiles);
    m_ConvertionType = std::vector<std::string>(convertionType);
}

FileConverterWorker::~FileConverterWorker()
{

}

void FileConverterWorker::Process()
{
    int fileCount = m_EegFiles.size();
	for (int i = 0; i < fileCount; i++)
	{
        EEGFormat::IFile* current = CreateGenericFile(m_EegFiles[i].c_str(), true);
        std::string oldType = EEGFormat::Utility::GetFileExtension(m_EegFiles[i]);
        std::string newType = m_ConvertionType[i];
		std::transform(newType.begin(), newType.end(), newType.begin(), ::tolower);
		std::string outputDirectory = EEGFormat::Utility::GetDirectoryPath(current->DefaultFilePath());
		std::string fileName = EEGFormat::Utility::GetFileName(current->DefaultFilePath(), false);

		if (newType.compare("trc") == 0)
		{
			EEGFormat::MicromedFile* newFile = new EEGFormat::MicromedFile(*current);
			newFile->SaveAs(outputDirectory, fileName);
			EEGFormat::Utility::DeleteAndNullify(newFile);
		}
		else if (newType.compare("eeg") == 0)
		{
			EEGFormat::ElanFile* newFile = new EEGFormat::ElanFile(*current);

			std::string headerPath = outputDirectory + fileName + ".eeg.ent";
			std::string dataPath = outputDirectory + fileName + ".eeg";
			std::string posPath = outputDirectory + fileName + "_raw.pos";
			std::string notePath = outputDirectory + fileName + ".notes.txt";

			newFile->SaveAs(headerPath, dataPath, notePath, posPath);
			EEGFormat::Utility::DeleteAndNullify(newFile);
		}
		else if (newType.compare("vhdr") == 0)
		{
			EEGFormat::BrainVisionFile* newFile = new EEGFormat::BrainVisionFile(*current);
			newFile->SaveAs(outputDirectory, fileName);
			EEGFormat::Utility::DeleteAndNullify(newFile);
		}
		else if (newType.compare("edf") == 0)
		{
			emit sendLogInfo("It is not possible to create edf file yet.");
		}
		else
		{
			emit sendLogInfo("File type unknown, aborting convertion for " + QString::fromStdString(fileName));
		}

		EEGFormat::Utility::DeleteAndNullify(current);

		emit sendLogInfo("End of conversion from " + QString::fromStdString(oldType) + " to " + QString::fromStdString(newType));
	}

	emit finished();
}
