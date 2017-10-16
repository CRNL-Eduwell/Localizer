#include "eegContainer.h"

InsermLibrary::dataContainer::dataContainer(vector<int> frequencyBand, samplingInformation samplingInfo)
{
	sampInfo.samplingFrequency = samplingInfo.samplingFrequency;
	sampInfo.downsampledFrequency = samplingInfo.downsampledFrequency;
	sampInfo.downsampFactor = samplingInfo.downsampFactor;
	sampInfo.nbSample = samplingInfo.nbSample;

	arrayLength = sampInfo.nbSample;
	arrayDownLength = arrayLength / sampInfo.downsampFactor;
	frequencyLength = (int)frequencyBand.size();

	bipData.resize(5, vector<float>(arrayLength));
	hilData.resize(5, vector<float>(arrayLength));

	fftFront.resize(5, new FFTINFO(-1, arrayLength));
	fftBack.resize(5, new FFTINFO(1, arrayLength));
	downData.resize(5, vector<float>(arrayDownLength));
	meanData.resize(5, vector<float>(arrayDownLength));
	convoData.resize(6, vector<vector<float>>(5, vector<float>(arrayDownLength)));

	for (int i = 0; i < frequencyBand.size() - 1; i++)
		firData.push_back(new FIRINFO(frequencyBand[i], frequencyBand[i + 1], sampInfo.samplingFrequency, sampInfo.nbSample));
}

InsermLibrary::dataContainer::~dataContainer()
{
	//vector manage all destructors
}

InsermLibrary::eegContainer::eegContainer(ELANFile* elan, int downsampFrequency, int nbFreqBand)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);

	for (int i = 0; i < nbFreqBand; i++)
		elanFrequencyBand.push_back(new elan_struct_t());

	elanFile = elan;
	getElectrodeFromElanFile(elanFile);
	//==
	sampInfo.samplingFrequency = (int)elanFile->samplingFrequency();
	sampInfo.downsampledFrequency = downsampFrequency;
	sampInfo.downsampFactor = sampInfo.samplingFrequency / sampInfo.downsampledFrequency; //verifier si bonne fréquence d'échantillonage ( non 2^n factor)
	sampInfo.nbSample = elanFile->nbSample();
	//==
	originalFilePath = elan->filePath();
	originalFilePath.replace(originalFilePath.end() - 4, originalFilePath.end(), "");
	//==
	calculateSmoothing();
	//== get triggers after last beginningCode that indicates beginning of expe
	int beginValue = 0;
	vector<int> indexBegin = findIndexes(elanFile->triggers, 99);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;

	if (elanFile->triggers.size() > 0)
	{
		deleteAndNullify1D(triggEeg);
		triggEeg = new TRIGGINFO(&elanFile->triggers[beginValue], elanFile->triggers.size() - beginValue);
		deleteAndNullify1D(triggEegDownsampled);
		triggEegDownsampled = new TRIGGINFO(&elanFile->triggers[beginValue], elanFile->triggers.size() - beginValue, sampInfo.downsampFactor);
	}
}

InsermLibrary::eegContainer::eegContainer(TRCFile* trc, int downsampFrequency, int nbFreqBand)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);

	for (int i = 0; i < nbFreqBand; i++) 
		elanFrequencyBand.push_back(new elan_struct_t());

	trcFile = trc;
	getElectrodeFromTRCFile(trcFile);
	//==
	sampInfo.samplingFrequency = (int)trc->header().samplingRate;
	sampInfo.downsampledFrequency = downsampFrequency;
	sampInfo.downsampFactor = sampInfo.samplingFrequency / sampInfo.downsampledFrequency; //verifier si bonne fréquence d'échantillonage ( non 2^n factor)
	sampInfo.nbSample = trcFile->nbSample();
	//==
	originalFilePath = trcFile->filePath();
	originalFilePath.replace(originalFilePath.end() - 4, originalFilePath.end(), "");
	//==
	calculateSmoothing();
	//== get triggers after last beginningCode that indicates beginning of expe
	int beginValue = 0;
	vector<int> indexBegin = findIndexes(trcFile->triggers(), 99);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;

	if (trcFile->triggers().size() > 0)
	{
		deleteAndNullify1D(triggEeg);
		triggEeg = new TRIGGINFO(&trcFile->triggers()[beginValue], trcFile->triggers().size() - beginValue);
		deleteAndNullify1D(triggEegDownsampled);
		triggEegDownsampled = new TRIGGINFO(&trcFile->triggers()[beginValue], trcFile->triggers().size() - beginValue, sampInfo.downsampFactor);
	}
}

InsermLibrary::eegContainer::eegContainer(EDFFile* edf, int downsampFrequency, int nbFreqBand)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);

	for (int i = 0; i < nbFreqBand; i++)
		elanFrequencyBand.push_back(new elan_struct_t());

	edfFile = edf;
	getElectrodeFromEDFFile(edfFile);
	//==
	sampInfo.samplingFrequency = (int)edf->SamplingFrequency();
	sampInfo.downsampledFrequency = downsampFrequency;
	sampInfo.downsampFactor = sampInfo.samplingFrequency / sampInfo.downsampledFrequency; //verifier si bonne fréquence d'échantillonage ( non 2^n factor)
	sampInfo.nbSample = edf->Header().recordsNumber * edf->SamplingFrequency();
	//==
	originalFilePath = edf->filePath();
	originalFilePath.replace(originalFilePath.end() - 4, originalFilePath.end(), "");
	//==
	calculateSmoothing();
	//== get triggers after last beginningCode that indicates beginning of expe
	int beginValue = 0;
	vector<int> indexBegin = findIndexes(edf->Events(), 99);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;

	if (edf->Events().size() > 0)
	{
		deleteAndNullify1D(triggEeg);
		triggEeg = new TRIGGINFO(&edf->Events()[beginValue], edf->Events().size() - beginValue);
		deleteAndNullify1D(triggEegDownsampled);
		triggEegDownsampled = new TRIGGINFO(&edf->Events()[beginValue], edf->Events().size() - beginValue, sampInfo.downsampFactor);
	}
}

InsermLibrary::eegContainer::~eegContainer()
{
	deleteAndNullify1D(trcFile);
	deleteAndNullify1D(elanFile);
	deleteAndNullify1D(edfFile);

	fftwf_cleanup_threads();
}

void InsermLibrary::eegContainer::deleteElectrodes(vector<int> elecToDelete)
{
	if (trcFile != nullptr)
	{
		TRCFunctions::deleteOneOrSeveralElectrodesAndData(trcFile, elecToDelete);
		TRCFunctions::convertAnalogDataToDigital(trcFile);
		for (int i = 0; i < trcFile->eegDataAllChanels().size(); i++)
			eegData.push_back(move(trcFile->eegDataAllChanels()[i]));
	}
	else if (elanFile != nullptr)
	{
		ELANFunctions::deleteOneOrSeveralElectrodesAndData(elanFile, elecToDelete);
		ELANFunctions::convertELANAnalogDataToDigital(elanFile);
		eegData.resize(elanFile->nbChannels(), vector<float>(sampInfo.nbSample));
		for (int i = 0; i < elanFile->nbChannels(); i++)
		{
			for (int j = 0; j < sampInfo.nbSample; j++)
				eegData[i][j] = elanFile->EEGData()[0][i][j];
		}
	}
	else if (edfFile != nullptr)
	{
		EDFFunctions::deleteOneOrSeveralElectrodesAndData(edfFile, elecToDelete);
		for (int i = 0; i < edfFile->eegData().size(); i++)
			eegData.push_back(move(edfFile->eegData()[i]));
	}
}

void InsermLibrary::eegContainer::bipolarizeData()
{
	if (bipoles.size() > 0)
		bipoles.clear();

	int totalPlot = 0;

	for (int i = 0; i < electrodes.size(); i++)
	{
		for (int j = 1; j < electrodes[i].id.size(); j++)
		{
			if ((electrodes[i].id[j] - electrodes[i].id[j - 1]) == 1)
			{
				bipoles.push_back(bipole(totalPlot + j, totalPlot + (j - 1)));
				bipoles[bipoles.size() - 1].positivLabel = flatElectrodes[electrodes[i].idOrigFile[j]];
				bipoles[bipoles.size() - 1].negativLabel = flatElectrodes[electrodes[i].idOrigFile[j - 1]];
			}
		}
		totalPlot += (int)electrodes[i].id.size();
	}
}

void InsermLibrary::eegContainer::getElectrodes()
{
	if (trcFile != nullptr)
	{
		getElectrodeFromTRCFile(trcFile);
	}
	else if (elanFile != nullptr)
	{
		getElectrodeFromElanFile(elanFile);
	}
	else if (edfFile != nullptr)
	{
		getElectrodeFromEDFFile(edfFile);
	}
}

void InsermLibrary::eegContainer::getElectrodeFromElanFile(ELANFile* elan)
{
	if (electrodes.size() > 0)
		electrodes.clear();

	if (flatElectrodes.size() > 0)
		flatElectrodes.clear();

	string elecNameStringTemp = "%#";
	for (int i = 0; i < elan->nbChannels(); i++)
	{
		string result = "";
		int resId = -1;

		int goodId = idSplitDigiAndNum(elan->electrodes[i].name);

		if (goodId != -1)
		{
			result = elan->electrodes[i].name.substr(0, goodId);
			resId = stoi(elan->electrodes[i].name.substr(goodId, elan->electrodes[i].name.size()));
		}
		else
		{
			result = elan->electrodes[i].name;
		}

		if (result.find(elecNameStringTemp) != std::string::npos && (result.length() == elecNameStringTemp.length()))
		{
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
			/*cout << result << " et " << resId << endl;*/
		}
		else
		{
			elecNameStringTemp = result;
			electrodes.push_back(elecContainer());
			electrodes[electrodes.size() - 1].label = result;
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
			//cout << "[=====]"<< endl;
			//cout << result << " et " << resId << endl;
		}
	}
}

void InsermLibrary::eegContainer::getElectrodeFromTRCFile(TRCFile* trc)
{
	if (electrodes.size() > 0)
		electrodes.clear();

	if (flatElectrodes.size() > 0)
		flatElectrodes.clear();

	string elecNameStringTemp = "%#";
	for (int i = 0; i < trc->electrodes().size(); i++)
	{
		string result = "";
		int resId = -1;

		int goodId = idSplitDigiAndNum(trc->electrodes()[i].positiveInputLabel);

		if (goodId != -1)
		{
			result = trc->electrodes()[i].positiveInputLabel.substr(0, goodId);
			resId = stoi(trc->electrodes()[i].positiveInputLabel.substr(goodId, trc->electrodes()[i].positiveInputLabel.size()));
		}
		else
		{
			result = trc->electrodes()[i].positiveInputLabel;
		}

		if (result.find(elecNameStringTemp) != std::string::npos && (result.length() == elecNameStringTemp.length()))
		{
			/*cout << result << " et " << resId << endl;*/
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
		}
		else
		{
			//cout << "[=====]"<< endl;
			//cout << result << " et " << resId << endl;
			elecNameStringTemp = result;
			electrodes.push_back(elecContainer());
			electrodes[electrodes.size() - 1].label = result;
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
		}
	}
}

void InsermLibrary::eegContainer::getElectrodeFromEDFFile(EDFFile* edf)
{
	if (electrodes.size() > 0)
		electrodes.clear();

	if (flatElectrodes.size() > 0)
		flatElectrodes.clear();

	string elecNameStringTemp = "%#";
	for (int i = 0; i < edf->Electrodes().size(); i++)
	{
		string result = "";
		int resId = -1;

		int goodId = idSplitDigiAndNum(edf->Electrodes()[i].label);

		if (goodId != -1)
		{
			result = edf->Electrodes()[i].label.substr(0, goodId);
			resId = stoi(edf->Electrodes()[i].label.substr(goodId, edf->Electrodes()[i].label.size()));
		}
		else
		{
			result = edf->Electrodes()[i].label;
		}

		if (result.find(elecNameStringTemp) != std::string::npos && (result.length() == elecNameStringTemp.length()))
		{
			/*cout << result << " et " << resId << endl;*/
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
		}
		else
		{
			//cout << "[=====]"<< endl;
			//cout << result << " et " << resId << endl;
			elecNameStringTemp = result;
			electrodes.push_back(elecContainer());
			electrodes[electrodes.size() - 1].label = result;
			electrodes[electrodes.size() - 1].id.push_back(resId);
			electrodes[electrodes.size() - 1].idOrigFile.push_back(i);
			flatElectrodes.push_back(result + to_string(resId));
		}
	}
}

int InsermLibrary::eegContainer::idSplitDigiAndNum(string myString)
{
	for (int j = 0; j < myString.size(); j++)
	{
		if (isdigit(myString[j]) && myString[j] != 0)
		{
			return j;
		}
	}
	return -1;
}

void InsermLibrary::eegContainer::calculateSmoothing()
{
	for (int i = 0; i < 6; i++)
	{
		smoothingSample[i] = ((sampInfo.samplingFrequency * smoothingMilliSec[i]) / 1000) / sampInfo.downsampFactor;
	}
}

vector<int> InsermLibrary::eegContainer::findIndexes(vector<digitalTriggers> trigg, int value2find)
{
	vector<int> indexesFound;
	for (int i = 0; i < trigg.size(); i++)
	{
		if (trigg[i].triggerValue == value2find)
		{
			indexesFound.push_back(i);
		}
	}

	return indexesFound;
}

vector<int> InsermLibrary::eegContainer::findIndexes(vector<eventElanFile> trigg, int value2find)
{
	vector<int> indexesFound;
	for (int i = 0; i < trigg.size(); i++)
	{
		if (trigg[i].code == value2find)
		{
			indexesFound.push_back(i);
		}
	}

	return indexesFound;
}

vector<int> InsermLibrary::eegContainer::findIndexes(vector<Edf_event> trigg, int value2find)
{
	vector<int> indexesFound;
	for (int i = 0; i < trigg.size(); i++)
	{
		if (trigg[i].code == value2find)
		{
			indexesFound.push_back(i);
		}
	}

	return indexesFound;
}

void InsermLibrary::eegContainer::ToHilbert(elan_struct_t* elanStruct, vector<int> frequencyBand)
{
	thread thr[5];
	initElanFreqStruct(elanStruct);
	dataContainer dataCont = dataContainer(frequencyBand, sampInfo);

	for (int i = 0; i < bipoles.size() / 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			int idCurrentBip = (i * 5) + j;
			for (int k = 0; k < dataCont.arrayLength; k++)
			{
				dataCont.bipData[j][k] = eegData[bipoles[idCurrentBip].positivElecId][k] -
										  eegData[bipoles[idCurrentBip].negativElecId][k];
			}
		}

		for (int j = 0; j < frequencyBand.size() - 1; j++)
		{
			thr[0] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, 0, j);
			thr[1] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, 1, j);
			thr[2] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, 2, j);
			thr[3] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, 3, j);
			thr[4] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, 4, j);

			thr[0].join(); 
			thr[1].join(); 
			thr[2].join();
			thr[3].join(); 
			thr[4].join();
		}

		thr[0] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, 0);
		thr[1] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, 1);
		thr[2] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, 2);
		thr[3] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, 3);
		thr[4] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, 4);

		thr[0].join(); 
		thr[1].join(); 
		thr[2].join();
		thr[3].join();
		thr[4].join();

		for (int j = 0; j < elanStruct->measure_channel_nb; j++)
		{
			for (int k = 0; k < 5; k++)
			{
				for (int l = 0; l < elanStruct->eeg.samp_nb; l++)
				{
					elanStruct->eeg.data_float[j][(i * 5) + k][l] = dataCont.convoData[j][k][l];
				}
			}
		}
	}

	if (bipoles.size() % 5 != 0)
	{
		for (int i = 0; i < bipoles.size() % 5; i++)
		{
			for (int j = 0; j < dataCont.arrayLength; j++)
			{
				int idCurrentBip = (bipoles.size() / 5) * 5;
				dataCont.bipData[i][j] = eegData[bipoles[idCurrentBip + i].positivElecId][j] -
										  eegData[bipoles[idCurrentBip + i].negativElecId][j];
			}
		}

		for (int i = 0; i < frequencyBand.size() - 1; i++)
		{
			for (int j = 0; j < bipoles.size() % 5; j++)
			{
				thr[j] = thread(&InsermLibrary::eegContainer::hilbertDownSampSumData, this, &dataCont, j, i);
				thr[j].join();
			}
		}

		for (int i = 0; i < bipoles.size() % 5; i++)
		{
			thr[i] = thread(&InsermLibrary::eegContainer::meanConvolveData, this, &dataCont, i);
			thr[i].join();
		}

		for (int i = 0; i < elanStruct->measure_channel_nb; i++)
		{
			for (int j = 0; j < bipoles.size() % 5; j++)
			{
				int currentId = (bipoles.size() - (bipoles.size() % 5)) + j;
				for (int k = 0; k < elanStruct->eeg.samp_nb; k++)
				{
					elanStruct->eeg.data_float[i][currentId][k] = dataCont.convoData[i][j][k];
				}
			}
		}
	}

	vector<string> splitOrigFilePath = split<string>(originalFilePath, "/\\");
	string patientName = splitOrigFilePath[splitOrigFilePath.size() - 1];
	string frequencyFolder = "_f" + to_string(frequencyBand[0]) + "f" + to_string(frequencyBand[frequencyBand.size() - 1]);
	string rootFolder = originalFilePath + frequencyFolder + "/";

	struct stat info;
	if (stat(rootFolder.c_str(), &info) != 0)
	{
		cout << "Creating freQ FOLDER" << endl;
		_mkdir(rootFolder.c_str());
	}

	for (int i = 0; i < 6; i++)
	{
		string nameOuputFile = rootFolder + patientName + frequencyFolder + "_ds" + to_string(sampInfo.downsampFactor) + "_sm" + to_string((int)smoothingMilliSec[i]) + ".eeg";
		ELANFunctions::writeOldElanHeader(elanStruct, nameOuputFile);
		ELANFunctions::writeOldElanData(elanStruct, nameOuputFile, i);
	}

	//deleteAndNullify1D(dataCont);
}

void InsermLibrary::eegContainer::initElanFreqStruct(elan_struct_t *structToInit)
{
	ef_init_structure(structToInit);
	structToInit->version = ELAN_HISTO;
	structToInit->chan_nb = (int)bipoles.size();
	structToInit->measure_channel_nb = 6;

	ef_alloc_measure_label_list(structToInit);
	strcpy_s(structToInit->measure_channel_label_list[0], "Hilbert Enveloppe -> Smoothing 0 ms");
	strcpy_s(structToInit->measure_channel_label_list[1], "Hilbert Enveloppe -> Smoothing 250 ms");
	strcpy_s(structToInit->measure_channel_label_list[2], "Hilbert Enveloppe -> Smoothing 500 ms");
	strcpy_s(structToInit->measure_channel_label_list[3], "Hilbert Enveloppe -> Smoothing 1000 ms");
	strcpy_s(structToInit->measure_channel_label_list[4], "Hilbert Enveloppe -> Smoothing 2500 ms");
	strcpy_s(structToInit->measure_channel_label_list[5], "Hilbert Enveloppe -> Smoothing 5000 ms");

	ef_alloc_channel_list(structToInit);

	for (int i = 0; i < bipoles.size(); i++)
	{
		sprintf_s(structToInit->chan_list[i].lab, bipoles[i].positivLabel.c_str());
		sprintf_s(structToInit->chan_list[i].type, "EEG");
		sprintf_s(structToInit->chan_list[i].unit, "uV");
	}

	structToInit->has_eeg = EF_YES;
	structToInit->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
	structToInit->eeg.samp_nb = sampInfo.nbSample / sampInfo.downsampFactor;
	structToInit->eeg.sampling_freq = (float)sampInfo.downsampledFrequency;

	ef_alloc_data_array(structToInit);
}

void InsermLibrary::eegContainer::hilbertDownSampSumData(dataContainer *dataCont, int threadId, int freqId)
{
	if (freqId == 0)
	{
		for (int i = 0; i < dataCont->arrayDownLength; i++)
		{
			dataCont->meanData[threadId][i] = 0.0f;
		}
	}

	mtx.lock();
	MATLABFUNC::bandPassHilbertFreq(dataCont->bipData[threadId], dataCont->hilData[threadId],
									 dataCont->firData[freqId], dataCont->fftFront[threadId],
									 dataCont->fftBack[threadId]);
	mtx.unlock();

	//Downsamp
	for (int i = 0; i < dataCont->arrayDownLength; i++)
	{
		dataCont->downData[threadId][i] = dataCont->hilData[threadId][sampInfo.downsampFactor * i];
	}

	float mean = 0;
	int value = round(dataCont->arrayDownLength / 4);
	for (int i = value; i < 3 * value; i++)
	{
		mean += dataCont->downData[threadId][i];
	}

	float fmtab = mean / (3 * value - value);

	if (fmtab == 0)
		fmtab = 1;

	for (int i = 0; i < dataCont->arrayDownLength; i++)
	{
		dataCont->downData[threadId][i] = (100 * dataCont->downData[threadId][i]) / fmtab;
		dataCont->meanData[threadId][i] += dataCont->downData[threadId][i];
	}
}

void InsermLibrary::eegContainer::meanConvolveData(dataContainer *dataCont, int threadId)
{
	for (int i = 0; i < dataCont->arrayDownLength; i++)
	{
		dataCont->meanData[threadId][i] = (10 * dataCont->meanData[threadId][i]) / (dataCont->frequencyLength - 1);
	}

	for (int i = 0; i < dataCont->arrayDownLength; i++)
	{
		dataCont->convoData[0][threadId][i] = dataCont->meanData[threadId][i];
	}

	MATLABFUNC::Convolution2(&dataCont->meanData[threadId][0], dataCont->arrayDownLength,
							 &dataCont->convoData[1][threadId][0], smoothingSample[1]);
	MATLABFUNC::Convolution2(&dataCont->meanData[threadId][0], dataCont->arrayDownLength,
							 &dataCont->convoData[2][threadId][0], smoothingSample[2]);
	MATLABFUNC::Convolution2(&dataCont->meanData[threadId][0], dataCont->arrayDownLength,
							 &dataCont->convoData[3][threadId][0], smoothingSample[3]);
	MATLABFUNC::Convolution2(&dataCont->meanData[threadId][0], dataCont->arrayDownLength,
							 &dataCont->convoData[4][threadId][0], smoothingSample[4]);
	MATLABFUNC::Convolution2(&dataCont->meanData[threadId][0], dataCont->arrayDownLength,
							 &dataCont->convoData[5][threadId][0], smoothingSample[5]);
}