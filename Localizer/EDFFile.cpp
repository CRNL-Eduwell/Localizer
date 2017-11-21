#include "EDFFile.h"

using namespace std;

InsermLibrary::EDFFile::EDFFile(std::string filePath)
{
	try
	{
		m_filePath = filePath;
		readHeader();
		m_dataLength = m_fileLength - m_header.bytesNumber;
		m_samplingFreq = (float)m_electrodes[0].samplesNumber / m_header.recordDuration;
	}
	catch (system_error& e)
	{
		cerr << "Error reading .EDF File" << endl;
		cerr << e.code().message().c_str() << endl;
	}
}

InsermLibrary::EDFFile::~EDFFile()
{

}

void InsermLibrary::EDFFile::readEvents()
{
	auto it = std::find_if(m_electrodes.begin(), m_electrodes.end(), [](Edf_electrode& currentElec)
	{
		return currentElec.label == "EDFAnnotations";
	});

	if (it != end(m_electrodes))
	{
		vector<string> events;
		ifstream sr(m_filePath, ios::binary);
		events.resize(m_header.recordsNumber);
		for (int i_r = 0; i_r < m_header.recordsNumber; ++i_r)
		{
			int nbSamp = m_electrodes[it->id].samplesNumber;
			events[i_r].resize(nbSamp * 2);

			int idEvent = m_header.bytesNumber + (((it->id + 1) * nbSamp * 2)  * i_r) + (it->id * nbSamp * 2);
			events[i_r] = binaryStringExtraction(idEvent, m_samplingFreq * 2, sr);
		}
		sr.close();

		for (int i_r = 0; i_r < events.size(); ++i_r)
		{
			if (events[i_r].find("MARQUEUR") != std::string::npos || events[i_r].find("Appel") != std::string::npos || events[i_r].find("EXT") != std::string::npos)
			{
				string delimiter = ".";
				delimiter = delimiter.append(string(1, (char)(20)));
				vector<string> splitdata = split<string>(events[i_r], delimiter);

				for (int i_rr = 0; i_rr < splitdata.size(); i_rr++)
				{
					if (splitdata[i_rr].find("MARQUEUR") != std::string::npos)
					{
						vector<string> splitCode = split<string>(splitdata[i_rr], "MARQUEUR");
						int code = stoi(splitCode[0]);
						int sample = (int)(i_r * m_samplingFreq + (std::atof(string("0.").append(splitdata[i_rr - 1].c_str()).c_str()) * m_samplingFreq));

						m_events.push_back(Edf_event(splitdata[i_rr], code, sample));
					}
					else if (splitdata[i_rr].find("Appel") != std::string::npos)
					{
						vector<string> splitCode = split<string>(splitdata[i_rr], "Appel patient");
						int code = 1;
						int sample = (int)(i_r * m_samplingFreq + (std::atof(string("0.").append(splitdata[i_rr - 1].c_str()).c_str()) * m_samplingFreq));

						m_events.push_back(Edf_event(splitdata[i_rr], code, sample));
					}
					else if (splitdata[i_rr].find("EXT") != std::string::npos)
					{
						vector<string> splitCode = split<string>(splitdata[i_rr], "EXT");
						int code = stoi(splitCode[0]);
						int sample = (int)(i_r * m_samplingFreq + (std::atof(string("0.").append(splitdata[i_rr - 1].c_str()).c_str()) * m_samplingFreq));

						m_events.push_back(Edf_event(splitdata[i_rr], code, sample));

					}
				}
			}
		}

		//delete the "EDF Annotation" electrodes since 
		//now we don't need it anymore
		m_electrodes.erase(m_electrodes.begin() + it->id);
	}
}

void InsermLibrary::EDFFile::readEEGData()
{
	short int *tab_data_int = new short int[m_dataLength];
	FILE *edfFile;
	fopen_s(&edfFile, m_filePath.c_str(), "r");
	fseek(edfFile, m_header.bytesNumber, SEEK_SET);
	fread(tab_data_int, sizeof(short int), m_dataLength, edfFile);
	fclose(edfFile);

	m_eegAllChanels.resize(m_electrodes.size(), std::vector<float>(m_dataLength / ((m_electrodes.size() + 1) * 2)));

	for (int j = 0; j < m_header.recordsNumber; j++)
	{
		for (int i = 0; i < m_electrodes.size(); i++)
		{
			for (int k = 0; k < m_samplingFreq; k++)
			{
				m_eegAllChanels[i][k + (j*m_samplingFreq)] = tab_data_int[k + (i*(int)m_samplingFreq) + (j * ((int)m_electrodes.size() + 1) * (int)m_samplingFreq)];
			}
		}
	}
	delete[] tab_data_int;
}

unsigned char InsermLibrary::EDFFile::binaryCharExtraction(int positionInFile, ifstream &sr)
{
	unsigned char buf;
	sr.clear();
	sr.seekg(positionInFile, std::ios::beg);
	buf = (unsigned char)(sr.get());
	return buf;
}

unsigned long InsermLibrary::EDFFile::binaryBytesExtraction(int positionInFile, int numberOfBytes, ifstream &sr)
{
	unsigned long valueOfBytes = 0;
	unsigned char ucharValue;
	int count = 0;

	/*According to count value, we shift ucharValue from 8,16,24,32 ...*/
	while (count < numberOfBytes)
	{
		ucharValue = binaryCharExtraction(positionInFile, sr);
		valueOfBytes = valueOfBytes + unsigned long(ucharValue << (8 * count));

		count++;
		positionInFile++;
	}
	return valueOfBytes;
}

string InsermLibrary::EDFFile::binaryStringExtraction(int positionInFile, int numberOfChar, ifstream &sr)
{
	char charValue = ' ';
	int count = 0;
	vector<char> myExtractedString;

	sr.clear();
	sr.seekg(positionInFile, ios::beg);
	for (int i = 0; i < numberOfChar; i++)
	{
		sr.get(charValue);
		if (charValue >= 0)
		{
			myExtractedString.push_back(charValue);
		}
	}
	return string(myExtractedString.begin(), myExtractedString.end());
}

int InsermLibrary::EDFFile::getFileSize(ifstream &sr)
{
	sr.clear();
	sr.seekg(0, std::ios::end);
	return (int)sr.tellg();
}

void InsermLibrary::EDFFile::readHeader()
{
	ifstream sr(m_filePath, ios::binary);
	m_fileLength = getFileSize(sr);
	m_header.version = stoi(binaryStringExtraction(0, 8, sr));
	m_header.patientID = binaryStringExtraction(8, 80, sr);
	m_header.recordingID = binaryStringExtraction(88, 80, sr);
	m_header.startDate = binaryStringExtraction(168, 8, sr);
	m_header.startTime = binaryStringExtraction(176, 8, sr);
	m_header.bytesNumber = stoi(binaryStringExtraction(184, 8, sr));
	m_header.reserved = binaryStringExtraction(192, 44, sr);
	m_header.recordsNumber = stoi(binaryStringExtraction(236, 8, sr));
	m_header.recordDuration = stoi(binaryStringExtraction(244, 8, sr));
	m_header.signalNumber = stoi(binaryStringExtraction(252, 4, sr));

	m_electrodes.resize(m_header.signalNumber);
	for (int i = 0; i < m_header.signalNumber; i++)
	{
		m_electrodes[i].id = i;
		deblankString(m_electrodes[i].label = binaryStringExtraction(256 + (i * 16), 16, sr));
	}

	int currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].transducerType = binaryStringExtraction(currentPosition + (i * 80), 80, sr);

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].dimension = binaryStringExtraction(currentPosition + (i * 8), 8, sr);

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].physicalMinimum = stoi(binaryStringExtraction(currentPosition + (i * 8), 8, sr));

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].physicalMaximum = stoi(binaryStringExtraction(currentPosition + (i * 8), 8, sr));

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].digitalMinimum = stoi(binaryStringExtraction(currentPosition + (i * 8), 8, sr));

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].digitalMaximum = stoi(binaryStringExtraction(currentPosition + (i * 8), 8, sr));

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].prefiltering = binaryStringExtraction(currentPosition + (i * 80), 80, sr);

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].samplesNumber = stoi(binaryStringExtraction(currentPosition + (i * 8), 8, sr));

	currentPosition = (int)sr.tellg();
	for (int i = 0; i < m_header.signalNumber; i++)
		m_electrodes[i].reserved = binaryStringExtraction(currentPosition + (i * 32), 32, sr);

	sr.close();
}