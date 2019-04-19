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

InsermLibrary::eegContainer::eegContainer(EEGFormat::IFile* file, int downsampFrequency, int nbFreqBand)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);
	
	elanFrequencyBand.resize(nbFreqBand, std::vector<EEGFormat::ElanFile*>(6));

	m_file = file;
	GetElectrodes(m_file);
	//==
	sampInfo.samplingFrequency = m_file->SamplingFrequency();
	sampInfo.downsampledFrequency = downsampFrequency;
	sampInfo.downsampFactor = sampInfo.samplingFrequency / sampInfo.downsampledFrequency; //verifier si bonne fréquence d'échantillonage ( non 2^n factor)
	sampInfo.nbSample = Data().size() > 0 ? Data()[0].size() : 0;

	std::cout << "nbSample" << sampInfo.nbSample << std::endl;
	//==
	originalFilePath = m_file->DefaultFilePath();
	originalFilePath.replace(originalFilePath.end() - 4, originalFilePath.end(), "");
	//==
	calculateSmoothing();
	//== get triggers after last beginningCode that indicates beginning of expe
	int beginValue = 0;
	vector<int> indexBegin = findIndexes(m_file->Triggers(), 99);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;

	if (m_file->Triggers().size() > 0)
	{
		deleteAndNullify1D(triggEeg);
		triggEeg = new TRIGGINFO(&m_file->Triggers()[beginValue], m_file->Triggers().size() - beginValue);
		deleteAndNullify1D(triggEegDownsampled);
		triggEegDownsampled = new TRIGGINFO(&m_file->Triggers()[beginValue], m_file->Triggers().size() - beginValue, sampInfo.downsampFactor);
	}
}

InsermLibrary::eegContainer::~eegContainer()
{
	deleteAndNullify1D(m_file);
	fftwf_cleanup_threads();
}

void InsermLibrary::eegContainer::deleteElectrodes(vector<int> elecToDelete)
{
	if (m_file != nullptr)
	{
		//Data is supposed to be loaded at this point
		//Convert analog to digital => still usefull ? 
		m_file->DeleteElectrodesAndData(elecToDelete);
		//Copy data in vector eeg data
	}
}

void InsermLibrary::eegContainer::GetElectrodes()
{
	if (m_file != nullptr)
	{
		GetElectrodes(m_file);
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

void InsermLibrary::eegContainer::GetElectrodes(EEGFormat::IFile* edf)
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

		int goodId = idSplitDigiAndNum(edf->Electrodes()[i].Label());

		if (goodId != -1)
		{
			result = edf->Electrodes()[i].Label().substr(0, goodId);
			resId = stoi(edf->Electrodes()[i].Label().substr(goodId, edf->Electrodes()[i].Label().size()));
		}
		else
		{
			result = edf->Electrodes()[i].Label();
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

std::vector<int> InsermLibrary::eegContainer::findIndexes(std::vector<EEGFormat::ITrigger> & trigg, int value2find)
{
	std::vector<int> indexesFound;
	for (int i = 0; i < trigg.size(); i++)
	{
		if (trigg[i].Code() == value2find)
		{
			indexesFound.push_back(i);
		}
	}

	return indexesFound;
}

void InsermLibrary::eegContainer::ToHilbert(int IdFrequency, vector<int> frequencyBand)
{
	thread thr[5];
	initElanFreqStruct();
	dataContainer dataCont = dataContainer(frequencyBand, sampInfo);

	std::cout << bipoles.size() / 5 << std::endl;
	for (int i = 0; i < bipoles.size() / 5; i++)
	{
		std::cout << "bip " << i << std::endl;

		for (int j = 0; j < 5; j++)
		{
			int idCurrentBip = (i * 5) + j;
			for (int k = 0; k < dataCont.arrayLength; k++)
			{
				dataCont.bipData[j][k] = Data()[bipoles[idCurrentBip].positivElecId][k] - Data()[bipoles[idCurrentBip].negativElecId][k];
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

		for (int j = 0; j < elanFrequencyBand[IdFrequency].size(); j++)
		{
			for (int k = 0; k < 5; k++)
			{
				for (int l = 0; l < elanFrequencyBand[IdFrequency][j]->Data(EEGFormat::DataConverterType::Digital)[(i * 5) + k].size(); l++)
				{
					elanFrequencyBand[IdFrequency][j]->Data(EEGFormat::DataConverterType::Digital)[(i * 5) + k][l] = dataCont.convoData[j][k][l];
				}
			}
		}
	}

	std::cout << "end debut" << std::endl;

	if (bipoles.size() % 5 != 0)
	{
		for (int i = 0; i < bipoles.size() % 5; i++)
		{
			std::cout << "bip " << i << std::endl;

			for (int j = 0; j < dataCont.arrayLength; j++)
			{
				int idCurrentBip = (bipoles.size() / 5) * 5;
				dataCont.bipData[i][j] = Data()[bipoles[idCurrentBip + i].positivElecId][j] - Data()[bipoles[idCurrentBip + i].negativElecId][j];
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

		for (int i = 0; i < elanFrequencyBand[IdFrequency].size(); i++)
		{
			for (int j = 0; j < bipoles.size() % 5; j++)
			{
				int currentId = (bipoles.size() - (bipoles.size() % 5)) + j;
				for (int k = 0; k < elanFrequencyBand[IdFrequency][i]->Data(EEGFormat::DataConverterType::Digital)[currentId].size(); k++)
				{
					elanFrequencyBand[IdFrequency][i]->Data(EEGFormat::DataConverterType::Digital)[currentId][k] = dataCont.convoData[i][j][k];
				}
			}
		}
	}

	std::cout << "end calc" << std::endl;
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
		string nameOuputFile = rootFolder + patientName + frequencyFolder + "_ds" + to_string(sampInfo.downsampFactor) + "_sm" + to_string((int)smoothingMilliSec[i]);// +".eeg";
		elanFrequencyBand[IdFrequency][i]->SaveAs(nameOuputFile + ".eeg.ent", nameOuputFile + ".eeg", "", "");
		//ELANFunctions::writeOldElanHeader(elanStruct, nameOuputFile);
		//ELANFunctions::writeOldElanData(elanStruct, nameOuputFile, i);
	}

	//deleteAndNullify1D(dataCont);
}

//file is sm0 file usually
void InsermLibrary::eegContainer::readBlocDataAllChannels(EEGFormat::ElanFile* file, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2])
{
	cout << "Extracting Data" << endl;
	for (int i = 0; i < file->ElectrodeCount(); i++)
	{
		for (int j = 0; j < triggEeg->triggers.size(); j++)
		{
			int trigTime = triggEeg->triggers[j].trigger.sample;
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < (winSam[1] - winSam[0]); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + k < 0)
					eegData[i][j][k] = 0;
				else
					eegData[i][j][k] = (file->Data(EEGFormat::DataConverterType::Digital)[i][beginTime + k] - 1000) / 10;
			}
		}
	}
}

//file is sm0 file usually
void InsermLibrary::eegContainer::readBlocDataEventsAllChannels(EEGFormat::ElanFile* file, TRIGGINFO *triggEeg, vector<vector<vector<float>>> &eegData, int winSam[2])
{
	cout << "Extracting Data" << endl;
	for (int i = 0; i < triggEeg->triggers.size(); i++)
	{
		for (int j = 0; j < winSam[1] - winSam[0]; j++)
		{
			int trigTime = triggEeg->triggers[i].trigger.sample;
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < file->ElectrodeCount(); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					eegData[i][k][j] = 0;
				else
					eegData[i][k][j] = (file->Data(EEGFormat::DataConverterType::Digital)[k][beginTime + j] - 1000) / 10;
			}
		}
	}
}


void InsermLibrary::eegContainer::initElanFreqStruct()
{
	std::vector<EEGFormat::IElectrode> electrodes = m_file->Electrodes();
	for (int i = 0; i < elanFrequencyBand.size(); i++)
	{
		for (int j = 0; j < elanFrequencyBand[i].size(); j++)
		{
			elanFrequencyBand[i][j] = new EEGFormat::ElanFile();
			elanFrequencyBand[i][j]->ElectrodeCount((int)bipoles.size());
			elanFrequencyBand[i][j]->SamplingFrequency(sampInfo.downsampledFrequency);
			elanFrequencyBand[i][j]->Electrodes(electrodes);
			//Define type of elec : label + "EEG" + "uV"
			elanFrequencyBand[i][j]->Data(EEGFormat::DataConverterType::Digital).resize((int)bipoles.size(), std::vector<float>(sampInfo.nbSample / sampInfo.downsampFactor));
		}
	}
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