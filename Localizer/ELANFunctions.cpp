#include "ELANFunctions.h"

ELANFile *InsermLibrary::ELANFunctions::micromedToElan(TRCFile *trc)
{
	ELANFile *newElanFile = new ELANFile(trc->filePath());
	newElanFile->eegFilePath.replace(newElanFile->eegFilePath.end() - 4, newElanFile->eegFilePath.end(), ".eeg");

	ef_init_structure(newElanFile->elanStruct);
	newElanFile->elanStruct->version = ELAN_HISTO;

	newElanFile->elanStruct->measure_channel_nb = 1;
	ef_alloc_measure_label_list(newElanFile->elanStruct);
	strcpy_s(newElanFile->elanStruct->measure_channel_label_list[0], "(s)EEG Data");

	newElanFile->elanStruct->chan_nb = trc->header().numberStoredChannels;
	ef_alloc_channel_list(newElanFile->elanStruct);
	for (int i = 0; i < newElanFile->nbChannels(); i++)
	{
		strcpy_s(newElanFile->elanStruct->chan_list[i].lab, trc->electrodes()[i].positiveInputLabel.c_str());
		strcpy_s(newElanFile->elanStruct->chan_list[i].type, "V2");
		strcpy_s(newElanFile->elanStruct->chan_list[i].unit,  TRCFunctions::signalMeasurementUnitText(trc->electrodes()[i].measurementUnit).c_str());
	}

	newElanFile->elanStruct->has_eeg = EF_YES;
	newElanFile->elanStruct->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
	newElanFile->elanStruct->eeg.samp_nb = trc->nbSample();
	newElanFile->elanStruct->eeg.sampling_freq = trc->header().samplingRate;

	initOrigStructElanStruct(newElanFile->elanStruct);

	newElanFile->elanStruct->orig_info->has_eeg_info = EF_YES;
	newElanFile->elanStruct->orig_info->eeg_info.orig_datatype = ORIG_EEG_DATATYPE_16BITS;

	int physicalMaximum = 3200;
	int physicalMinimum = -3200;
	int digitalMaximum = 32767;
	int digitalMinimum = -32768;

	newElanFile->elanStruct->orig_info->eeg_info.eeg_convADC = new double[newElanFile->nbChannels()];
	newElanFile->elanStruct->orig_info->eeg_info.eeg_offsetADC = new double[newElanFile->nbChannels()];
	for (int i = 0; i < newElanFile->nbChannels(); i++)
	{
		newElanFile->elanStruct->orig_info->eeg_info.eeg_convADC[i] = ((double)physicalMaximum - physicalMinimum) / (digitalMaximum - digitalMinimum);
		newElanFile->elanStruct->orig_info->eeg_info.eeg_offsetADC[i] = ((double)(physicalMinimum * digitalMaximum) - (physicalMaximum * digitalMinimum)) / (digitalMaximum - digitalMinimum);
	}

	ef_alloc_data_array(newElanFile->elanStruct);
	for (int i = 0; i < newElanFile->nbChannels(); i++)
	{
		for (int j = 0; j < newElanFile->elanStruct->eeg.samp_nb; j++)
		{
			newElanFile->elanStruct->eeg.data_float[0][i][j] = trc->eegDataAllChanels()[i][j] * 10; // mul by 10 why ?
		}
	}

	newElanFile->getElectrodes();

	for (int i = 0; i < trc->triggers().size(); i++)
		newElanFile->triggers.push_back(eventElanFile(trc->triggers()[i].triggerSample, trc->triggers()[i].triggerValue));

	return newElanFile;
}

void InsermLibrary::ELANFunctions::deleteOneOrSeveralElectrodesAndData(ELANFile *elan, vector<int> indexToDelete)
{
	if (indexToDelete.size() > 0)
	{
		if (elan->flatEEGData() == nullptr)
		{
			readDataAllChannels(elan, elan->eegFilePath);
			createNewElanStructForCopy(elan, indexToDelete);
			elan->getElectrodes();
			return;
		}
		else
		{
			createNewElanStructForCopy(elan, indexToDelete);
			elan->getElectrodes();
			return;
		}
	}
}

void InsermLibrary::ELANFunctions::convertMicromedAnalogDataToDigital(ELANFile *elan, TRCFile *trc)
{
	for (int i = 0; i < elan->nbChannels(); i++)
	{
		for (int j = 0; j < elan->elanStruct->eeg.samp_nb; j++)
		{
			long mul = (trc->electrodes()[i].logicMaximum - trc->electrodes()[i].logicMinimum) + 1;
			long div = (trc->electrodes()[i].physicMaximum - trc->electrodes()[i].physicMinimum);
			elan->elanStruct->eeg.data_double[0][i][j] = (((double)trc->eegDataAllChanels()[i][j] * mul) / div);
		}
	}
}

//int physicalMaximum = 3200;
//int physicalMinimum = -3200;
//int digitalMaximum = 32767;
//int digitalMinimum = -32768;
//eeg_convADC[i] = ((double)physicalMaximum - physicalMinimum) / (digitalMaximum - digitalMinimum);
//eeg_offsetADC[i] = ((double)(physicalMinimum * digitalMaximum) - (physicalMaximum * digitalMinimum)) / (digitalMaximum - digitalMinimum);
void InsermLibrary::ELANFunctions::convertELANAnalogDataToDigital(ELANFile *elan)
{
	for (int i = 0; i < elan->nbChannels(); i++)
	{
		for (int j = 0; j < elan->elanStruct->eeg.samp_nb; j++)
		{
			elan->elanStruct->eeg.data_float[0][i][j] = (elan->elanStruct->eeg.data_float[0][i][j] - 
												  (float)elan->elanStruct->orig_info->eeg_info.eeg_offsetADC[i]) / 
												  (float)elan->elanStruct->orig_info->eeg_info.eeg_convADC[i];
		}
	}
}

void InsermLibrary::ELANFunctions::convertELANAnalogDataToDigital(elan_struct_t *elan)
{
	for (int i = 0; i < elan->chan_nb; i++)
	{
		for (int j = 0; j < elan->eeg.samp_nb; j++)
		{
			elan->eeg.data_float[0][i][j] = (elan->eeg.data_float[0][i][j] - (float)elan->orig_info->eeg_info.eeg_offsetADC[i]) /
																			 (float)elan->orig_info->eeg_info.eeg_convADC[i];
		}
	}
}

void InsermLibrary::ELANFunctions::convertELANDigitalToAnalog(ELANFile *elan)
{
	for (int i = 0; i < elan->nbChannels(); i++)
	{
		for (int j = 0; j < elan->elanStruct->eeg.samp_nb; j++)
		{
			elan->elanStruct->eeg.data_double[0][i][j] = (elan->elanStruct->eeg.data_double[0][i][j] * (double)elan->elanStruct->orig_info->eeg_info.eeg_convADC[i]) + (double)elan->elanStruct->orig_info->eeg_info.eeg_offsetADC[i];
		}
	}
}

void InsermLibrary::ELANFunctions::readPosFile(ELANFile *elan, string pathPosFile)
{
	vector<string> posData = readTxtFile(pathPosFile);
	for (int i = 0; i < posData.size(); i++)
	{
		vector<string> currentEventData = split<string>(posData[i], "\t");
		elan->triggers.push_back(eventElanFile(atoi(currentEventData[0].c_str()), atoi(currentEventData[1].c_str())));
	}
}

void InsermLibrary::ELANFunctions::readFile(ELANFile *elan, std::string filePath)
{
	ef_read_elan_file((char*)filePath.c_str(), elan->elanStruct);
	elan->getElectrodes();
}

void InsermLibrary::ELANFunctions::readHeader(ELANFile *elan, std::string filePath)
{
	ef_read_elan_header_file((char*)filePath.c_str(), elan->elanStruct);
	elan->getElectrodes();
}

void InsermLibrary::ELANFunctions::readDataAllChannels(ELANFile *elan, std::string filePath)
{
	int sizeEegArray = elan->nbMeasure() * elan->nbChannels() * elan->nbSample();

	ef_alloc_data_array(elan->elanStruct);
	ef_read_elan_data_all_channels_file((char*)filePath.c_str(), elan->elanStruct);
}

void InsermLibrary::ELANFunctions::readBlocDataAllChannels(elan_struct_t *elan, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2])
{
	cout << "Extracting Data" << endl;
	for (int i = 0; i < elan->chan_nb; i++)
	{
		for (int j = 0; j < triggEeg->triggers.size(); j++)
		{
			int trigTime = triggEeg->triggers[j].trigger.sample;
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < (winSam[1] - winSam[0]); k++)
			{
				eegData[i][j][k] = (elan->eeg.data_float[0][i][beginTime + k] - 1000) / 10;
			}
		}
	}
}

void InsermLibrary::ELANFunctions::readBlocDataEventsAllChannels(elan_struct_t *elan, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2])
{
	cout << "Extracting Data" << endl;
	for (int i = 0; i < triggEeg->triggers.size(); i++)
	{
		for (int j = 0; j < winSam[1] - winSam[0]; j++)
		{
			int trigTime = triggEeg->triggers[i].trigger.sample;
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < elan->chan_nb; k++)
			{
				eegData[i][k][j] = (elan->eeg.data_float[0][k][beginTime + j] - 1000) / 10;
			}
		}
	}
}

void InsermLibrary::ELANFunctions::writePosFile(ELANFile *elan, string pathPosFile)
{
	ofstream posFile(pathPosFile, ios::out);
	for (int i = 0; i < elan->triggers.size(); i++)
		posFile << elan->triggers[i].sample << setw(10) << elan->triggers[i].code << setw(10) << "0" << endl;
	posFile.close();
}

void InsermLibrary::ELANFunctions::writeFile(ELANFile *elan, std::string filePath)
{
	switch (elan->elanStruct->version)
	{
	case HIST_VERSION:
		writeOldElanHeader(elan, filePath);
		writeOldElanData(elan, filePath);
		break;

	case HDF5_VERSION:
		ef_write_elan_file((char*)filePath.c_str(), elan->elanStruct);
		break;
	default:
		fprintf(stderr, "ERROR: What the Hell is this version. \n");
	}
}

void InsermLibrary::ELANFunctions::writeHeader(ELANFile *elan, std::string filePath)
{
	switch (elan->elanStruct->version)
	{
	case HIST_VERSION:
		writeOldElanHeader(elan, filePath);
		break;

	case HDF5_VERSION:
		ef_write_elan_header_file((char*)filePath.c_str(), elan->elanStruct);
		break;
	default:
		fprintf(stderr, "ERROR: What the Hell is this version. \n");
	}
}

void InsermLibrary::ELANFunctions::writeDataAllChannels(ELANFile *elan, std::string filePath)
{
	switch (elan->elanStruct->version)
	{
	case HIST_VERSION:
		writeOldElanData(elan, filePath);
		break;

	case HDF5_VERSION:
		ef_write_elan_data_all_channels_file((char*)filePath.c_str(), elan->elanStruct);
		break;
	default:
		fprintf(stderr, "ERROR: What the Hell is this version. \n");
	}
}

void InsermLibrary::ELANFunctions::writeOldElanHeader(elan_struct_t *elan, std::string filePath)
{
	int compteur = 0;
	std::string eegFileName = filePath.append(".ent");
	std::ofstream eegFile(eegFileName, std::ios::binary);   // on ouvre le fichier en écriture	
	if (eegFile)
	{
		eegFile << "V2\n";
		eegFile << "Conversion from Micromed file\n";
		eegFile << "by eeg2env C++\n";
		eegFile << "XX:XX:XX" << "\n";
		eegFile << "XX:XX:XX" << "\n";
		eegFile << "-1\n";
		eegFile << "Reserved\n";
		eegFile << "-1\n";
		eegFile << ((double)1 / elan->eeg.sampling_freq) << "\n";
		eegFile << (elan->chan_nb + 2) << "\n";;

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << elan->chan_list[i].lab << ".-1\n";
		}

		eegFile << "NUM1\n";
		eegFile << "NUM2\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << "electrode EEG\n";
		}

		eegFile << "dateur echantillon\n";
		eegFile << "type evenement et byte info\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << elan->chan_list[i].unit << "\n";
		}
		eegFile << "none\n";
		eegFile << "none\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << -3200 << "\n";
		}
		eegFile << "-1\n";
		eegFile << "-1\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << "+" << 3200 << "\n";
		}
		eegFile << "+1\n";
		eegFile << "+1\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << -32768 << "\n";						//logic min 
		}
		eegFile << -32768 << "\n";
		eegFile << -32768 << "\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << 32767 << "\n";;						//logic max 										
		}
		eegFile << 32767 << "\n";
		eegFile << 32767 << "\n";

		for (int i = 0; i < elan->chan_nb; i++)
		{
			eegFile << "passe-haut X Hz passe-bas X Hz\n";
		}
		eegFile << "sans\n";
		eegFile << "sans\n";

		for (int i = 0; i < elan->chan_nb + 2; i++)
		{
			eegFile << "1\n";
		}

		for (int i = 0; i < elan->chan_nb + 2; i++)
		{
			eegFile << "reserved\n";
		}
	}
	else
	{
		std::cout << "Problème d'ouverture du fichier !" << std::endl;
	}
}

void InsermLibrary::ELANFunctions::writeOldElanData(elan_struct_t *elan, std::string filePath, int idMeasure)
{
	vector<char> dataBuffer;
	short tempValue = 0;

	for (int j = 0; j < elan->eeg.samp_nb; j++)
	{
		for (int k = 0; k < elan->chan_nb; k++) //nb de canaux totaux restant																							                                                      
		{
			tempValue = elan->eeg.data_float[idMeasure][k][j];
			char *c = (char*)&tempValue;
			std::swap(c[0], c[1]);

			dataBuffer.push_back((char)c[0]);
			dataBuffer.push_back((char)c[1]);
		}
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
	}

	std::string eegFileName = filePath;
	std::ofstream eegFile(eegFileName, std::ios::binary);   // on ouvre le fichier en écriture		
	if (eegFile)
	{
		eegFile.write(&dataBuffer[0], dataBuffer.size());
		eegFile.close();
	}
	else
	{
		std::cout << "Problème d'ouverture du fichier !" << std::endl;
	}
}

void InsermLibrary::ELANFunctions::exportEEGDataCSV(string csvFilePath, ELANFile *elan)
{
	ofstream fichierCsv(csvFilePath, ios::out);

	for (int j = 0; j < elan->electrodes.size(); j++)
	{
		fichierCsv << elan->electrodes[j].name.c_str() << ";";
	}
	fichierCsv << endl;

	for (int i = 0; i < elan->samplingFrequency() * 10; i++)
	{
		for (int j = 0; j < elan->nbChannels(); j++)
		{
			fichierCsv << elan->EEGData()[0][j][i] << ";";
		}
		fichierCsv << endl;
	}
	fichierCsv.close();
}

void InsermLibrary::ELANFunctions::exportTEEGDataCSVOneChanel(string csvFilePath, ELANFile *elan, int index)
{
	ofstream fichierCsv(csvFilePath, ios::out);
	fichierCsv << elan->electrodes[index].name.c_str() << ";" << endl;
	for (int i = 0; i < elan->samplingFrequency() * 10; i++)
	{
		fichierCsv << elan->EEGData()[0][index][i] << ";" << endl;
	}
	fichierCsv.close();
}

//=== Private 
//ef_orig_init_struct from ef_origread.c
void InsermLibrary::ELANFunctions::initOrigStructElanStruct(elan_struct_t *elanStruct)
{
	//newElanStruct->version = HIST_VERSION;
	//newElanStruct->release = 0;
	//newElanStruct->endian = EF_BIG_ENDIAN;

	//newElanStruct->has_eeg = EF_NO;
	//newElanStruct->eeg.data_type = EF_DATA_double;
	elanStruct->has_ep = EF_NO;
	elanStruct->ep.data_type = EF_DATA_DOUBLE;
	elanStruct->has_tf = EF_NO;
	elanStruct->tf.data_type = EF_DATA_DOUBLE;

	elanStruct->orig_info = (orig_info_t *)calloc(1, sizeof(orig_info_t));
	ELAN_CHECK_ERROR_ALLOC(elanStruct->orig_info, "for storing original file format informations.");

	elanStruct->orig_info->has_eeg_info = EF_NO;
	elanStruct->orig_info->eeg_info.bufReadPtr = NULL;
	elanStruct->orig_info->eeg_info.bufReadSize = 0;

	elanStruct->orig_info->has_ep_info = EF_NO;
	elanStruct->orig_info->ep_info.bufReadPtr = NULL;
	elanStruct->orig_info->ep_info.bufReadSize = 0;

	elanStruct->orig_info->has_tf_info = EF_NO;
	elanStruct->orig_info->tf_info.offset_data = 0;
	elanStruct->orig_info->tf_info.bufReadPtr = NULL;
	elanStruct->orig_info->tf_info.bufReadSize = 0;

}

void InsermLibrary::ELANFunctions::createNewElanStructForCopy(ELANFile *elan, vector<int> indexToDelete)
{
	elan_struct_t *newElanStruct = new elan_struct_t();
	ef_init_structure(newElanStruct);

	newElanStruct->measure_channel_nb = 1;
	ef_alloc_measure_label_list(newElanStruct);
	strcpy_s(newElanStruct->measure_channel_label_list[0], "(s)EEG Data");

	int count = 0;
	newElanStruct->chan_nb = elan->elanStruct->chan_nb - (int)indexToDelete.size();
	ef_alloc_channel_list(newElanStruct);
	for (int i = 0; i < elan->nbChannels() - 1; i++)
	{
		int pos = (int)(find(indexToDelete.begin(), indexToDelete.end(), i) - indexToDelete.begin());
		if (!(pos < indexToDelete.size() - 1))
		{
			sprintf_s(newElanStruct->chan_list[count].lab, elan->elanStruct->chan_list[i].lab);
			sprintf_s(newElanStruct->chan_list[count].type, elan->elanStruct->chan_list[i].type);
			sprintf_s(newElanStruct->chan_list[count].unit, elan->elanStruct->chan_list[i].unit);
			count++;
		}
	}

	newElanStruct->has_eeg = EF_YES;
	newElanStruct->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
	newElanStruct->eeg.samp_nb = elan->elanStruct->eeg.samp_nb;
	newElanStruct->eeg.sampling_freq = elan->elanStruct->eeg.sampling_freq;

	//====
	initOrigStructElanStruct(newElanStruct);

	newElanStruct->orig_info->has_eeg_info = EF_YES;
	newElanStruct->orig_info->eeg_info.orig_datatype = elan->elanStruct->orig_info->eeg_info.orig_datatype;

	count = 0;
	newElanStruct->orig_info->eeg_info.eeg_convADC = new double[newElanStruct->chan_nb];
	newElanStruct->orig_info->eeg_info.eeg_offsetADC = new double[newElanStruct->chan_nb];
	for (int i = 0; i < elan->nbChannels() - 1; i++)
	{
		int pos = (int)(find(indexToDelete.begin(), indexToDelete.end(), i) - indexToDelete.begin());
		if (!(pos < indexToDelete.size() - 1))
		{
			newElanStruct->orig_info->eeg_info.eeg_convADC[count] = elan->elanStruct->orig_info->eeg_info.eeg_convADC[i];
			newElanStruct->orig_info->eeg_info.eeg_offsetADC[count] = elan->elanStruct->orig_info->eeg_info.eeg_offsetADC[i];
			count++;
		}
	}
	//====
	count = 0;
	ef_alloc_data_array(newElanStruct);
	for (int i = 0; i < elan->nbChannels() - 1; ++i)
	{
		int pos = (int)(find(indexToDelete.begin(), indexToDelete.end(), i) - indexToDelete.begin());
		if (!(pos < indexToDelete.size() - 1))
		{
			for (int j = 0; j < newElanStruct->eeg.samp_nb; j++)
			{
				newElanStruct->eeg.data_float[0][count][j] = elan->elanStruct->eeg.data_float[0][i][j];
			}
			count++;
		}
	}

	elan->delElanStruct();
	elan->elanStruct = newElanStruct;
}

void InsermLibrary::ELANFunctions::writeOldElanHeader(ELANFile *elan, std::string filePath)
{
	int compteur = 0;
	std::string eegFileName = filePath.append(".ent");
	std::ofstream eegFile(eegFileName, std::ios::binary);   // on ouvre le fichier en écriture	
	if (eegFile)
	{
		eegFile << "V2\n";
		eegFile << "Conversion from Micromed file\n";
		eegFile << "by eeg2env C++\n";
		eegFile << "XX:XX:XX" << "\n";
		eegFile << "XX:XX:XX" << "\n";
		eegFile << "-1\n";
		eegFile << "Reserved\n";
		eegFile << "-1\n";
		eegFile << ((double)1 / elan->elanStruct->eeg.sampling_freq) << "\n"; 
		eegFile << (elan->elanStruct->chan_nb + 2) << "\n";; 

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << elan->electrodes[i].name << ".-1\n";
		}

		eegFile << "NUM1\n";
		eegFile << "NUM2\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << "electrode EEG\n";
		}

		eegFile << "dateur echantillon\n";
		eegFile << "type evenement et byte info\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << elan->electrodes[i].unit << "\n";
		}
		eegFile << "none\n";
		eegFile << "none\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << -3200 << "\n";
		}
		eegFile << "-1\n";
		eegFile << "-1\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << "+" << 3200 << "\n";
		}
		eegFile << "+1\n";
		eegFile << "+1\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << -32768 << "\n";						//logic min 
		}
		eegFile << -32768 << "\n";
		eegFile << -32768 << "\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << 32767 << "\n";;						//logic max 										
		}
		eegFile << 32767 << "\n";
		eegFile << 32767 << "\n";

		for (int i = 0; i < elan->elanStruct->chan_nb; i++)
		{
			eegFile << "passe-haut X Hz passe-bas X Hz\n";
		}
		eegFile << "sans\n";
		eegFile << "sans\n";

		for (int i = 0; i < elan->elanStruct->chan_nb + 2; i++)
		{
			eegFile << "1\n";
		}

		for (int i = 0; i < elan->elanStruct->chan_nb + 2; i++)
		{
			eegFile << "reserved\n";
		}
	}
	else
	{
		std::cout << "Problème d'ouverture du fichier !" << std::endl;
	}
}

void InsermLibrary::ELANFunctions::writeOldElanData(ELANFile *elan, std::string filePath)
{
	vector<char> dataBuffer;
	int16 tempValue = 0;

	for (int j = 0; j < elan->elanStruct->eeg.samp_nb; j++)
	{
		for (int k = 0; k < elan->elanStruct->chan_nb; k++) //nb de canaux totaux restant																							                                                      
		{
			tempValue = (int16)elan->elanStruct->eeg.data_float[0][k][j];
			char *c = (char*)&tempValue;
			std::swap(c[0], c[1]);

			dataBuffer.push_back((char)c[0]);
			dataBuffer.push_back((char)c[1]);
		}
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
		dataBuffer.push_back((char)0);
	}

	std::string eegFileName = filePath;
	std::ofstream eegFile(eegFileName, std::ios::binary);   // on ouvre le fichier en écriture		
	if (eegFile)
	{
		eegFile.write(&dataBuffer[0], dataBuffer.size());
		eegFile.close();
	}
	else
	{
		std::cout << "Problème d'ouverture du fichier !" << std::endl;
	}
}

