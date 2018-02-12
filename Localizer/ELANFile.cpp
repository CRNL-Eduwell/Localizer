#include "ELANFile.h"

InsermLibrary::ELANFile::ELANFile(std::string currentFilePath)
{
	eegFilePath = currentFilePath;
	elanStruct = new elan_struct_t();
}

InsermLibrary::ELANFile::~ELANFile()
{
	delElanStruct();
}

int InsermLibrary::ELANFile::nbSample()
{
	return elanStruct->eeg.samp_nb;
}

int InsermLibrary::ELANFile::nbChannels()
{
	return elanStruct->chan_nb;
}

int InsermLibrary::ELANFile::nbMeasure()
{
	return elanStruct->measure_channel_nb;
}

double InsermLibrary::ELANFile::samplingFrequency()
{
	return elanStruct->eeg.sampling_freq;
}

string InsermLibrary::ELANFile::filePath()
{
	return eegFilePath;
}

float* InsermLibrary::ELANFile::flatEEGData()
{
	return elanStruct->eeg.flat_float;
}

float*** InsermLibrary::ELANFile::EEGData()
{
	return elanStruct->eeg.data_float;
}

void InsermLibrary::ELANFile::delElanStruct()
{
	if (elanStruct->chan_nb > 0 && flatEEGData() != nullptr)
	{
		ef_free_data_array(elanStruct);
		ef_free_struct(elanStruct);
		deleteAndNullify1D(elanStruct);
	}
	else if (elanStruct->measure_channel_nb == 0)
	{
		ef_free_struct(elanStruct);
		deleteAndNullify1D(elanStruct);
	}
}

void InsermLibrary::ELANFile::getElectrodes()
{
	if (electrodes.size() > 0)
		electrodes.clear();

	for (int i = 0; i < elanStruct->chan_nb; i++)
	{
		elecElanFile currentElec;
		currentElec.name = elanStruct->chan_list[i].lab;
		currentElec.unit = elanStruct->chan_list[i].unit;
		//currentElec.gain = (double)elanStruct->orig_info->eeg_info.eeg_convADC[i];
		//currentElec.offset = (double)elanStruct->orig_info->eeg_info.eeg_offsetADC[i];
		deblankString(currentElec.name);
		deblankString(currentElec.unit);
		electrodes.push_back(currentElec);
	}
}

void InsermLibrary::ELANFile::getElectrodes(std::vector<MicromedLibrary::electrode> & trcElectrodes)
{
	if (electrodes.size() > 0)
		electrodes.clear();

	for (int i = 0; i < elanStruct->chan_nb; i++)
	{
		elecElanFile currentElec;
		currentElec.name = elanStruct->chan_list[i].lab;
		currentElec.unit = elanStruct->chan_list[i].unit;
		//currentElec.gain = (double)elanStruct->orig_info->eeg_info.eeg_convADC[i];
		//currentElec.offset = (double)elanStruct->orig_info->eeg_info.eeg_offsetADC[i];
		currentElec.logicMaximum = trcElectrodes[i].logicMaximum -trcElectrodes[i].logicGround;
		currentElec.logicMinimum = trcElectrodes[i].logicMinimum -trcElectrodes[i].logicGround;
		currentElec.physicMaximum = trcElectrodes[i].physicMaximum;
		currentElec.physicMinimum = trcElectrodes[i].physicMinimum;
		deblankString(currentElec.name);
		deblankString(currentElec.unit);
		electrodes.push_back(currentElec);
	}
}