#include "GeneralOptionsFile.h"

InsermLibrary::GeneralOptionsFile::GeneralOptionsFile(const std::string& filePath) : ITxtFile(filePath)
{

}

void InsermLibrary::GeneralOptionsFile::Load()
{
	int LineCount = m_rawTextFileData.size();
	for (int i = 0; i < LineCount; i++)
	{
		if (i == 0) 
		{
			m_fileExtensions = std::vector<InsermLibrary::FileExt>();
			std::vector <std::string> RawFileOrderList = EEGFormat::Utility::Split<std::string>(m_rawTextFileData[i], "-");
			for (int j = 0; j < RawFileOrderList.size(); j++)
			{
				if (RawFileOrderList[j].compare("Micromed") == 0)
				{
					m_fileExtensions.push_back(InsermLibrary::FileExt::TRC);
				}
				else if (RawFileOrderList[j].compare("Elan") == 0)
				{
					m_fileExtensions.push_back(InsermLibrary::FileExt::EEG_ELAN);
				}
				else if (RawFileOrderList[j].compare("BrainVision") == 0)
				{
					m_fileExtensions.push_back(InsermLibrary::FileExt::BRAINVISION);
				}
				else if (RawFileOrderList[j].compare("Edf") == 0)
				{
					m_fileExtensions.push_back(InsermLibrary::FileExt::EDF);
				}
				else
				{
					std::cout << "GeneralOptionsFile::Load() => " << RawFileOrderList[j] << " not supported" << std::endl;
				}
			}
		}
	}
}

void InsermLibrary::GeneralOptionsFile::Save()
{
	std::ofstream optionFileStream(m_originalFilePath, std::ios::out);
	for (int i = 0; i < m_fileExtensions.size(); i++)
	{
		switch (m_fileExtensions[i])
		{
			case InsermLibrary::FileExt::TRC:
			{
				optionFileStream << "Micromed";
				break;
			}
			case InsermLibrary::FileExt::EEG_ELAN:
			{
				optionFileStream << "Elan";
				break;
			}
			case InsermLibrary::FileExt::BRAINVISION:
			{
				optionFileStream << "BrainVision";
				break;
			}
			case InsermLibrary::FileExt::EDF:
			{
				optionFileStream << "Edf";
				break;
			}
		}

		if (i < m_fileExtensions.size() - 1)
		{
			optionFileStream << "-";
		}
	}
	optionFileStream.close();
}