#include "HilbertEnveloppe.h"

using Framework::Filtering::Linear::Convolution;

//Note : Vérifier si il y'a besoin de garder un vecteur de données Fréquentielle par fréquence, vu que ce n'est pas
//calculé en même temps on peut garder un seul vecteur de sm0 à sm5000
void HilbertEnveloppe::Process(eegContainer* EegContainer, int IndexFrequencyData, vector<int> FrequencyBand)
{
	thread thr[5];
	int NumberOfSample = EegContainer->Data().size() > 0 ? EegContainer->Data()[0].size() : 0;
	int NumberOfElement = EegContainer->BipoleCount();
	int NumberOfFrequencyBins = FrequencyBand.size();
	DataContainer dataCont = DataContainer(EegContainer->SamplingFrequency(), EegContainer->DownsampledFrequency(), NumberOfSample, FrequencyBand);
	CalculateSmoothingCoefficients(EegContainer->SamplingFrequency(), EegContainer->DownsampledFrequency());
	InitOutputDataStructure(EegContainer);

	std::vector<EEGFormat::ElanFile*> FrequencyBandFiles = EegContainer->elanFrequencyBand[IndexFrequencyData];
	for (int i = 0; i < NumberOfElement / 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			int idCurrentBip = (i * 5) + j;
			for (int k = 0; k < NumberOfSample; k++)
			{
				std::pair<int, int> currentBipole = EegContainer->Bipole(idCurrentBip);
				dataCont.bipData[j][k] = EegContainer->Data()[currentBipole.first][k] - EegContainer->Data()[currentBipole.second][k];
			}
		}

		for (int j = 0; j < NumberOfFrequencyBins - 1; j++)
		{
			thr[0] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, 0, j);
			thr[1] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, 1, j);
			thr[2] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, 2, j);
			thr[3] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, 3, j);
			thr[4] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, 4, j);

			thr[0].join();
			thr[1].join();
			thr[2].join();
			thr[3].join();
			thr[4].join();
		}

		thr[0] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, 0);
		thr[1] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, 1);
		thr[2] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, 2);
		thr[3] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, 3);
		thr[4] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, 4);

		thr[0].join();
		thr[1].join();
		thr[2].join();
		thr[3].join();
		thr[4].join();

		for (int j = 0; j < FrequencyBandFiles.size(); j++)
		{
			for (int k = 0; k < 5; k++)
			{
				for (int l = 0; l < FrequencyBandFiles[j]->Data(EEGFormat::DataConverterType::Digital)[(i * 5) + k].size(); l++)
				{
					FrequencyBandFiles[j]->Data(EEGFormat::DataConverterType::Digital)[(i * 5) + k][l] = dataCont.convoData[j][k][l];
				}
			}
		}
	}

	if (NumberOfElement % 5 != 0)
	{
		for (int i = 0; i < NumberOfElement % 5; i++)
		{
			for (int j = 0; j < NumberOfSample; j++)
			{
				int idCurrentBip = (NumberOfElement / 5) * 5;
				std::pair<int, int> currentBipole = EegContainer->Bipole(idCurrentBip + i);
				dataCont.bipData[i][j] = EegContainer->Data()[currentBipole.first][j] - EegContainer->Data()[currentBipole.second][j];
			}
		}

		for (int i = 0; i < NumberOfFrequencyBins - 1; i++)
		{
			for (int j = 0; j < NumberOfElement % 5; j++)
			{
				thr[j] = thread(&HilbertEnveloppe::HilbertDownSampSumData, this, &dataCont, j, i);
				thr[j].join();
			}
		}

		for (int i = 0; i < NumberOfElement % 5; i++)
		{
			thr[i] = thread(&HilbertEnveloppe::MeanConvolveData, this, &dataCont, i);
			thr[i].join();
		}

		for (int i = 0; i < FrequencyBandFiles.size(); i++)
		{
			for (int j = 0; j < NumberOfElement % 5; j++)
			{
				int currentId = (NumberOfElement - (NumberOfElement % 5)) + j;
				for (int k = 0; k < FrequencyBandFiles[i]->Data(EEGFormat::DataConverterType::Digital)[currentId].size(); k++)
				{
					FrequencyBandFiles[i]->Data(EEGFormat::DataConverterType::Digital)[currentId][k] = dataCont.convoData[i][j][k];
				}
			}
		}
	}

}

void HilbertEnveloppe::InitOutputDataStructure(eegContainer* EegContainer)
{
	std::vector<EEGFormat::IElectrode*> bipolesList;
	int BipoleCount = EegContainer->BipoleCount();
	for (int i = 0; i < BipoleCount; i++)
	{
		std::pair<int,int> currentBipole = EegContainer->Bipole(i);
		bipolesList.push_back(EegContainer->Electrode(currentBipole.first));
	}

	for (int i = 0; i < EegContainer->elanFrequencyBand.size(); i++)
	{
		for (int j = 0; j < EegContainer->elanFrequencyBand[i].size(); j++)
		{
			EegContainer->elanFrequencyBand[i][j] = new EEGFormat::ElanFile();
			EegContainer->elanFrequencyBand[i][j]->ElectrodeCount((int)bipolesList.size());
			EegContainer->elanFrequencyBand[i][j]->SamplingFrequency(EegContainer->DownsampledFrequency());
			EegContainer->elanFrequencyBand[i][j]->Electrodes(bipolesList);
			//Define type of elec : label + "EEG" + "uV"
			EegContainer->elanFrequencyBand[i][j]->Data(EEGFormat::DataConverterType::Digital).resize((int)bipolesList.size(), std::vector<float>(EegContainer->NbSample() / EegContainer->DownsamplingFactor()));
		}
	}
}

void HilbertEnveloppe::CalculateSmoothingCoefficients(int SamplingFrequency, int DownsamplingFactor)
{
	for (int i = 0; i < 6; i++)
	{
		m_smoothingSample[i] = ((SamplingFrequency * m_smoothingMilliSec[i]) / 1000) / DownsamplingFactor;
	}
}

void HilbertEnveloppe::HilbertDownSampSumData(DataContainer* DataContainer, int threadId, int freqId)
{
	if (freqId == 0)
	{
		for (int i = 0; i < DataContainer->NbSampleDownsampled(); i++)
		{
			DataContainer->meanData[threadId][i] = 0.0f;
		}
	}

	m_mtx.lock();
	DataContainer->Filters[threadId][freqId]->BandPassHilbert(&DataContainer->hilData[threadId][0], &DataContainer->bipData[threadId][0], DataContainer->NbSample());
	m_mtx.unlock();

	//Downsamp
	int downsamplingFactor = DataContainer->NbSample() / DataContainer->NbSampleDownsampled();
	for (int i = 0; i < DataContainer->NbSampleDownsampled(); i++)
	{
		DataContainer->downData[threadId][i] = DataContainer->hilData[threadId][downsamplingFactor * i];
	}

	float mean = 0;
	int value = round(DataContainer->NbSampleDownsampled() / 4);
	for (int i = value; i < 3 * value; i++)
	{
		mean += DataContainer->downData[threadId][i];
	}

	float fmtab = mean / (3 * value - value);

	if (fmtab == 0)
		fmtab = 1;

	for (int i = 0; i < DataContainer->NbSampleDownsampled(); i++)
	{
		DataContainer->downData[threadId][i] = (100 * DataContainer->downData[threadId][i]) / fmtab;
		DataContainer->meanData[threadId][i] += DataContainer->downData[threadId][i];
	}
}

void HilbertEnveloppe::MeanConvolveData(DataContainer* DataContainer, int threadId)
{
	for (int i = 0; i < DataContainer->NbSampleDownsampled(); i++)
	{
		DataContainer->meanData[threadId][i] = (10 * DataContainer->meanData[threadId][i]) / (DataContainer->NbFrequencySlices() - 1);
		DataContainer->convoData[0][threadId][i] = DataContainer->meanData[threadId][i];
	}

	Convolution::MovingAverage(&DataContainer->meanData[threadId][0], &DataContainer->convoData[1][threadId][0], DataContainer->NbSampleDownsampled(), m_smoothingSample[1]);
	Convolution::MovingAverage(&DataContainer->meanData[threadId][0], &DataContainer->convoData[2][threadId][0], DataContainer->NbSampleDownsampled(), m_smoothingSample[2]);
	Convolution::MovingAverage(&DataContainer->meanData[threadId][0], &DataContainer->convoData[3][threadId][0], DataContainer->NbSampleDownsampled(), m_smoothingSample[3]);
	Convolution::MovingAverage(&DataContainer->meanData[threadId][0], &DataContainer->convoData[4][threadId][0], DataContainer->NbSampleDownsampled(), m_smoothingSample[4]);
	Convolution::MovingAverage(&DataContainer->meanData[threadId][0], &DataContainer->convoData[5][threadId][0], DataContainer->NbSampleDownsampled(), m_smoothingSample[5]);
}