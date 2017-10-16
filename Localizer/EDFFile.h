#ifndef _EDFFILE_H
#define _EDFFILE_H

#include <iostream>
#include <vector>
#include <fstream>
#include <ostream>
#include <string>

#include "EDFParameters.h"
#include "Utility.h"

namespace InsermLibrary
{
	class EDFFile
	{
	public:
		EDFFile(std::string filePath);
		~EDFFile();
		inline Edf_header Header() const { return m_header; };
		inline float SamplingFrequency() const { return m_samplingFreq; };
		inline std::vector<Edf_electrode> Electrodes() const { return m_electrodes; };
		inline std::vector<Edf_electrode> & Electrodes() { return m_electrodes; };
		inline std::vector<Edf_event> Events() const { return m_events; };
		inline std::string filePath() const { return m_filePath; };
		inline std::vector<std::vector<float>> & eegData() { return m_eegAllChanels; };
		void readEvents();
		void readEEGData();

	private:
		unsigned char binaryCharExtraction(int positionInFile, std::ifstream &sr);
		unsigned long binaryBytesExtraction(int positionInFile, int numberOfBytes, std::ifstream &sr);
		std::string binaryStringExtraction(int positionInFile, int numberOfChar, std::ifstream &sr);
		int getFileSize(std::ifstream &sr);
		void readHeader();

	private:
		Edf_header m_header;
		std::vector<Edf_electrode> m_electrodes;
		std::vector<Edf_event> m_events;
		std::vector<std::vector<float>> m_eegAllChanels;
		int m_fileLength, m_dataLength;
		float m_samplingFreq;
		std::string m_filePath;
	};
}

#endif