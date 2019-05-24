#include "eegContainer.h"

using Framework::Filtering::Linear::FirBandPass;
using Framework::Filtering::Linear::Convolution;

InsermLibrary::eegContainer::eegContainer(EEGFormat::IFile* file, int downsampFrequency, int nbFreqBand)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(5);

	elanFrequencyBand.resize(nbFreqBand, std::vector<EEGFormat::IFile*>(6));

	m_file = file;
	GetElectrodes(m_file);
	//==
	m_originalSamplingFrequency = m_file->SamplingFrequency();
	m_downsampledFrequency = downsampFrequency;
	m_nbSample = Data().size() > 0 ? Data()[0].size() : 0;
}

InsermLibrary::eegContainer::~eegContainer()
{
	deleteAndNullify1D(m_file);
	fftwf_cleanup_threads();
}

void InsermLibrary::eegContainer::DeleteElectrodes(vector<int> elecToDelete)
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

void InsermLibrary::eegContainer::BipolarizeElectrodes()
{
	if (m_bipoles.size() > 0)
		m_bipoles.clear();

	int totalPlot = 0;

	for (int i = 0; i < electrodes.size(); i++)
	{
		for (int j = 1; j < electrodes[i].id.size(); j++)
		{
			if ((electrodes[i].id[j] - electrodes[i].id[j - 1]) == 1)
			{
				m_bipoles.push_back(std::make_pair(totalPlot + j, totalPlot + (j - 1)));
			}
		}
		totalPlot += (int)electrodes[i].id.size();
	}
}

void InsermLibrary::eegContainer::SaveFrequencyData(EEGFormat::FileType FileType, int IdFrequency, const std::vector<int>& frequencyBand)
{
	std::string rootFileFolder = EEGFormat::Utility::GetDirectoryPath(m_file->DefaultFilePath());
	std::string patientName = EEGFormat::Utility::GetFileName(m_file->DefaultFilePath(), false);
	std::string frequencyFolder = "_f" + to_string(frequencyBand[0]) + "f" + to_string(frequencyBand[frequencyBand.size() - 1]);
	std::string rootFrequencyFolder = rootFileFolder + "/" + patientName + frequencyFolder + "/";

	struct stat info;
	if (stat(rootFrequencyFolder.c_str(), &info) != 0)
	{
		cout << "Creating freQ FOLDER" << endl;
		_mkdir(rootFrequencyFolder.c_str());
	}

	for (int i = 0; i < 6; i++)
	{
		std::string directory = rootFrequencyFolder;
		std::string baseFileName = patientName + frequencyFolder + "_ds" + to_string(DownsamplingFactor()) + "_sm" + to_string((int)m_smoothingMilliSec[i]);
		switch (FileType)
		{
			case EEGFormat::FileType::Micromed:
			{
				throw std::runtime_error("Micromed File type is not allowed as an output file");
				break;
			}
			case EEGFormat::FileType::Elan:
			{
				EEGFormat::ElanFile* elanFile = new EEGFormat::ElanFile(*elanFrequencyBand[IdFrequency][i]);
				elanFile->SaveAs(rootFrequencyFolder + baseFileName + ".eeg.ent", rootFrequencyFolder + baseFileName + ".eeg", "", "");
				break;
			}
			case EEGFormat::FileType::BrainVision:
			{
				std::string header = rootFrequencyFolder + baseFileName + ".vhdr";
				std::string data = rootFrequencyFolder + baseFileName + ".eeg";
			
				EEGFormat::BrainVisionFile* bvFile = new EEGFormat::BrainVisionFile(*elanFrequencyBand[IdFrequency][i]);
				bvFile->HeaderFilePath(header);
				bvFile->DataFilePath(data);
				bvFile->MarkersFilePath("");
				bvFile->Save();

				EEGFormat::Utility::DeleteAndNullify(bvFile);
				break;
			}
			case EEGFormat::FileType::EuropeanDataFormat:
			{
				throw std::runtime_error("European Data Format file type is not allowed as an output file");
				break;
			}
			default:
			{
				throw std::runtime_error("Output file type not recognized");
				break;
			}
		}
	}
}

//Advised order for filePaths : header-data-events-notes
int InsermLibrary::eegContainer::LoadFrequencyData(std::vector<std::string>& filesPath, int frequencyId, int smoothingId)
{
	std::string concatenatedFiles = "";
	for (int i = 0; i < filesPath.size(); i++)
	{
		concatenatedFiles += filesPath[i];
		if (i < filesPath.size() - 1)
			concatenatedFiles += ";";
	}

	elanFrequencyBand[frequencyId][smoothingId] = CreateGenericFile(concatenatedFiles.c_str(), true);
	if (elanFrequencyBand[frequencyId][smoothingId] == nullptr)
		return -1;
	else
		return 0;

}

void InsermLibrary::eegContainer::GetFrequencyBlocData(vec3<float>& outputEegData, int frequencyId, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2])
{
	int TriggerCount = triggEeg.size();
	if (triggEeg[0].SamplingFrequency() != elanFrequencyBand[frequencyId][smoothingId]->SamplingFrequency())
	{
		for (int i = 0; i < TriggerCount; i++)
		{
			triggEeg[i].UpdateFrequency(elanFrequencyBand[frequencyId][smoothingId]->SamplingFrequency());
		}
	}

	for (int i = 0; i < elanFrequencyBand[frequencyId][smoothingId]->ElectrodeCount(); i++)
	{
		for (int j = 0; j < TriggerCount; j++)
		{
			int trigTime = triggEeg[j].MainSample();
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < (winSam[1] - winSam[0]); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + k < 0)
					outputEegData[i][j][k] = 0;
				else
					outputEegData[i][j][k] = (elanFrequencyBand[frequencyId][smoothingId]->Data(EEGFormat::DataConverterType::Digital)[i][beginTime + k] -1000) / 10;
			}
		}
	}
}

void InsermLibrary::eegContainer::GetFrequencyBlocDataEvents(vec3<float>& outputEegData, int frequencyId, int smoothingId, std::vector<Trigger>& triggEeg, int winSam[2])
{
	int TriggerCount = triggEeg.size();
	if (triggEeg[0].SamplingFrequency() != elanFrequencyBand[frequencyId][smoothingId]->SamplingFrequency())
	{
		for (int i = 0; i < TriggerCount; i++)
		{
			triggEeg[i].UpdateFrequency(elanFrequencyBand[frequencyId][smoothingId]->SamplingFrequency());
		}
	}

	for (int i = 0; i < TriggerCount; i++)
	{
		for (int j = 0; j < winSam[1] - winSam[0]; j++)
		{
			int trigTime = triggEeg[i].MainSample();
			int beginTime = trigTime + winSam[0];

			for (int k = 0; k < elanFrequencyBand[frequencyId][smoothingId]->ElectrodeCount(); k++)
			{
				//to prevent issue in case the first event has been recorded realy quick
				if (beginTime + j < 0)
					outputEegData[i][k][j] = 0;
				else
					outputEegData[i][k][j] = (elanFrequencyBand[frequencyId][smoothingId]->Data(EEGFormat::DataConverterType::Digital)[k][beginTime + j] -1000) / 10;
			}
		}
	}
}

//=== Private :
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

		int goodId = idSplitDigiAndNum(edf->Electrodes()[i]->Label());

		if (goodId != -1)
		{
			result = edf->Electrodes()[i]->Label().substr(0, goodId);
			resId = stoi(edf->Electrodes()[i]->Label().substr(goodId, edf->Electrodes()[i]->Label().size()));
		}
		else
		{
			result = edf->Electrodes()[i]->Label();
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