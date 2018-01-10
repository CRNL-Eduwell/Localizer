#ifndef _ELANFILE_H
#define _ELANFILE_H

#include <iostream>
#include "TRCFunctions.h"

extern "C"							
{									
	#include "libelanfile\elanfile.h"
}

#define ELAN_HISTO 0
#define ELAN_HDF5  1

using namespace std;
using namespace MicromedLibrary;

namespace InsermLibrary
{
	struct elecElanFile
	{
		string name = "";
		string unit = "";
		int logicMinimum = 0;
		int logicMaximum = 0;
		int physicMinimum = 0;
		int physicMaximum = 0;
	};

	struct eventElanFile
	{
		eventElanFile(int sample, int code)
		{
			this->sample = sample;
			this->code = code;
		}

		int sample = -1;
		int code = -1;
	};

	struct noteElanFile
	{
		noteElanFile(int sample, std::string note)
		{
			this->sample = sample;
			this->note = note;
		}

		int sample = -1;
		std::string note;
	};

	class ELANFile
	{
		friend class ELANFunctions;

	public:
		ELANFile(string pathEEGFile);
		~ELANFile();
		int nbSample();
		int nbChannels();
		int nbMeasure();
		double samplingFrequency();
		string filePath();
		float *flatEEGData();
		float ***EEGData();
	private:
		void delElanStruct();
		void getElectrodes();
		void getElectrodes(std::vector<MicromedLibrary::electrode> & trcElectrodes);
	public :
		vector<eventElanFile> triggers;
		vector<noteElanFile> notes;
		vector<elecElanFile> electrodes;
	private :
		elan_struct_t *elanStruct = nullptr;
		string eegFilePath = "";
	};
}
#endif