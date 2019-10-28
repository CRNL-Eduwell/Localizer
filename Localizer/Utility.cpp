#include "Utility.h"

vector<string> InsermLibrary::readTxtFile(string pathFile)
{
	stringstream buffer;
	ifstream file(pathFile);
	if (file)
	{
		buffer << file.rdbuf();
		file.close();
		return(split<string>(buffer.str(), "\r\n"));
	}
	else
	{
		cout << " Error opening : " << pathFile << endl;
		return vector<string>();
	}
}

void InsermLibrary::saveTxtFile(vector<QString> data, string pathFile)
{
	ofstream fichier(pathFile, ios::out);
	for (int i = 0; i < data.size(); i++)
	{
		fichier << data[i].toStdString() << endl;
	}
	fichier.close();
}

//void InsermLibrary::deblankString(std::string &myString)
//{
//	myString.erase(remove_if(myString.begin(), myString.end(), isspace), myString.end());
//}

string InsermLibrary::GetCurrentWorkingDir()
{
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);

	#if defined(_WIN32) || defined(_WIN64)
		replace(current_working_dir.begin(), current_working_dir.end(), '\\', '/');
	#endif
	return current_working_dir;
}
