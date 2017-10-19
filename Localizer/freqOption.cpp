#include "freqOption.h"

using namespace std;
using namespace InsermLibrary;

InsermLibrary::freqOption::freqOption(string pathFreqFile)
{
	char buffer[256];
	int count = 0;
	frequency f;

	ifstream fichierFreq(pathFreqFile, ios::out);
	while (fichierFreq.getline(buffer, 256))
	{
		switch (count % 2)
		{
		case 0:
			f = frequency();
			f.freqName = buffer;
			count++;
			break;
		case 1:
			vector<string> splitValue = split<string>(buffer, ":");
			int fMin = atoi(splitValue[0].c_str());
			int step = atoi(splitValue[1].c_str());
			int fMax = atoi(splitValue[2].c_str());

			for (int i = 0; i <= ((fMax - fMin) / step); i++)
			{
				f.freqBandValue.push_back(fMin + (i * step));
			}
			count++;

			f.freqFolderName.append("f").append(splitValue[0].c_str()).append("f").append(splitValue[2].c_str());
			frequencyBands.push_back(f);
			break;
		}
	}


}

InsermLibrary::freqOption::~freqOption()
{

}