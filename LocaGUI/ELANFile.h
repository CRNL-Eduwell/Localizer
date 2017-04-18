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
	public :
		vector<eventElanFile> triggers;
		vector<elecElanFile> electrodes;
	private :
		elan_struct_t *elanStruct = nullptr;
		string eegFilePath = "";
	};
}
#endif