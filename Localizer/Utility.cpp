#include "Utility.h"

std::vector<std::string> InsermLibrary::readTxtFile(std::string pathFile)
{
	std::stringstream buffer;
	std::ifstream file(pathFile);
	if (file)
	{
		buffer << file.rdbuf();
		file.close();
		return(split<std::string>(buffer.str(), "\r\n"));
	}
	else
	{
		std::cout << " Error opening : " << pathFile << std::endl;
		return std::vector<std::string>();
	}
}

void InsermLibrary::saveTxtFile(std::vector<QString> data, std::string pathFile)
{
	std::ofstream fichier(pathFile, std::ios::out);
	for (int i = 0; i < data.size(); i++)
	{
		fichier << data[i].toStdString() << std::endl;
	}
	fichier.close();
}

//void InsermLibrary::deblankString(std::string &myString)
//{
//	myString.erase(remove_if(myString.begin(), myString.end(), isspace), myString.end());
//}

std::string InsermLibrary::GetCurrentWorkingDir()
{
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);

	#if defined(_WIN32) || defined(_WIN64)
		replace(current_working_dir.begin(), current_working_dir.end(), '\\', '/');
	#endif
	return current_working_dir;
}
