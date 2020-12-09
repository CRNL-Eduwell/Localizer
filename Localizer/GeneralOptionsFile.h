#pragma once

#include <iostream>
#include "Utility.h"
#include "ITxtFile.h"
#include <QCoreApplication>

namespace InsermLibrary
{
	class GeneralOptionsFile : public ITxtFile
	{
	public:
		GeneralOptionsFile(const std::string& filePath = QCoreApplication::applicationDirPath().toStdString() + "/Resources/Config/generalOptions.txt");	
		inline std::vector<InsermLibrary::FileExt>& FileExtensionsFavorite()
		{
			return m_fileExtensions;
		}
		void Load();
		void Save();

	private:
		std::vector<InsermLibrary::FileExt> m_fileExtensions;
	};
}
