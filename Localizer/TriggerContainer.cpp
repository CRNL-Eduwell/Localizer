#include "TriggerContainer.h"

InsermLibrary::TriggerContainer::TriggerContainer(std::vector<EEGFormat::ITrigger>& triggers, int samplingFrequnecy)
{
	m_originalSamplingFrequency = samplingFrequnecy;
	int TriggersCount = triggers.size();
	for (int i = 0; i < TriggersCount; i++)
	{
		m_originalTriggers.push_back(EEGFormat::ITrigger(triggers[i]));
	}
}

InsermLibrary::TriggerContainer::~TriggerContainer()
{

}

//flag = -1 to get the full trigger list
std::vector<InsermLibrary::Trigger> InsermLibrary::TriggerContainer::GetTriggerList(int flagCode, int downSamplingFactor)
{
	int beginvalue = 0;
	if(flagCode != -1)
		beginvalue = FindFirstIndexAfter(flagCode);
	std::vector<InsermLibrary::Trigger> triggers;
	if (m_originalTriggers.size() > 0)
	{
		for (int i = beginvalue; i < m_originalTriggers.size(); i++)
		{
			Trigger currentTrigger(m_originalTriggers[i], m_originalSamplingFrequency / downSamplingFactor);
			triggers.push_back(currentTrigger);
		}
	}
	return triggers;
}

void InsermLibrary::TriggerContainer::PairStimulationWithResponses(std::vector<Trigger>& triggers, PROV *myprovFile)
{
	std::vector<int> mainEventsCode = myprovFile->getMainCodes();
	std::vector<std::vector<int>> respEventsCode = myprovFile->getSecondaryCodes();

	int idVisuBloc = 0;
	int TriggerCount = triggers.size();
	for (int k = 0; k < TriggerCount; k++)
	{
		int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;

		for (int l = 0; l < mainEventsCode.size(); l++)
		{
			if (triggers[k].MainEvent().Code() == mainEventsCode[l])
			{
				idMain = k;
				for (int m = 0; m < myprovFile->visuBlocs.size(); m++)
				{
					if (mainEventsCode[l] == myprovFile->visuBlocs[m].mainEventBloc.eventCode[0])
						idVisuBloc = m;
				}
			}
		}

		//TODO : add a function that check there is not some triggerd with a different
		//sampling frequnecy, otherwise throw exception
		int triggersSamplingFrequency = triggers[0].SamplingFrequency();
		if (idMain != -1)
		{
			int winSamMin = round((triggersSamplingFrequency * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMin()) / 1000);
			int winSamMax = round((triggersSamplingFrequency * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMax()) / 1000);

			int CurrentTriggerId = -1; 
			while (idSec == -1 && CurrentTriggerId < TriggerCount - 1)
			{
				for (int l = 0; l < mainEventsCode.size(); l++)
				{
					for (int m = 0; m < respEventsCode[l].size(); m++)
					{
						if (triggers[idMain].MainCode() == mainEventsCode[l] && triggers[CurrentTriggerId].MainCode() == respEventsCode[l][m])
						{
							idSec = CurrentTriggerId;
						}
						else if (triggers[CurrentTriggerId].MainCode() == mainEventsCode[l] && idSec == -1)
						{
							idMain = CurrentTriggerId;
						}
					}
				}
				CurrentTriggerId++;
			}

			if (idMain != -1 && idSec != -1)
			{
				int winMax = triggers[idMain].MainSample() + winSamMax;
				int winMin = triggers[idMain].MainSample() - abs(winSamMin);

				bool isInWindow = (triggers[idSec].MainSample() < winMax) && (triggers[idSec].MainSample() > winMin);
				bool specialLoca = false;
				/*bool specialLoca = (currentLoca->localizerName() == "MARA" ||
					currentLoca->localizerName() == "MARM" ||
					currentLoca->localizerName() == "MARD");*/

				if (isInWindow)
				{
					triggers[idMain].Reponse().Code(triggers[idSec].MainCode());
					triggers[idMain].Reponse().Sample(triggers[idSec].MainSample());
				}
				else
				{
					//if it is out of the window but one of the special Loca (MARA, MARM or MARD we want to mark a "fake" reponse 
					//at the maximum window length to keep track of those trials
					if (specialLoca)
					{
						triggers[idMain].Reponse().Code(triggers[idSec].MainCode());
						triggers[idMain].Reponse().Sample(winMax);
					}
				}
			}
		}
	}
}

//Delete code that are not defined in prov file 
void InsermLibrary::TriggerContainer::DeleteTriggerNotInExperiment(std::vector<Trigger>& triggers, PROV *myprovFile)
{
	int TriggerCount = triggers.size();

	std::vector<int> IdToKeep, IdToDelete;
	for (int i = 0; i < TriggerCount; i++)
	{
		bool KeepCurrentTrigger = false;
		for (int j = 0; j < myprovFile->visuBlocs.size(); j++)
		{
			for (int k = 0; k < myprovFile->visuBlocs[j].mainEventBloc.eventCode.size(); k++)
			{
				if (triggers[i].MainCode() == myprovFile->visuBlocs[j].mainEventBloc.eventCode[k])
				{
					KeepCurrentTrigger = true;
					//break here ? 
				}
			}
		}

		if (KeepCurrentTrigger)
			IdToKeep.push_back(i);
		else
			IdToDelete.push_back(i);
	}

	for (int i = IdToDelete.size() - 1; i >= 0; i--)
		triggers.erase(triggers.begin() + IdToDelete[i]);
}

void InsermLibrary::TriggerContainer::DeleteTriggerNotPaired(std::vector<Trigger>& triggers)
{
	int TriggerCount = triggers.size();
	std::vector<int> IdToDelete;
	for (int i = 0; i < TriggerCount; i++)
	{
		if (triggers[i].ResponseCode() == -1)
		{
			IdToDelete.push_back(i);
		}
	}

	int ToDeleteCount = IdToDelete.size() - 1;
	for (int i = ToDeleteCount; i >= 0; i--)
		triggers.erase(triggers.begin() + IdToDelete[i]);
}

// An interval has start time and end time 
struct Interval {
	int start, end;
};

// Compares two intervals according to staring times. 
bool compareInterval(int provCode, int triggerCode)
{
	return provCode == triggerCode;
}

void InsermLibrary::TriggerContainer::SortTrialsForExperiment(std::vector<Trigger>& triggers, PROV *myprovFile)
{
	vector<int> dataId;
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
		sort(triggers.begin(), triggers.end(),[&](int m, int n)-> bool 
		{
			return m < n; //or use paramA in some way
		});

		if (triggers[0].MainCode() == myprovFile->visuBlocs[i].mainEventBloc.eventCode[0])
		{
		}
	}
}

int InsermLibrary::TriggerContainer::FindFirstIndexAfter(int flagCode)
{
	int beginValue = 0;
	std::vector<int> indexBegin = FindIndexes(flagCode);
	if (indexBegin.size() > 0)
		beginValue = indexBegin[indexBegin.size() - 1] + 1;
	return beginValue;
}

std::vector<int> InsermLibrary::TriggerContainer::FindIndexes(int searchValue)
{
	std::vector<int> indexes;
	for (int i = 0; i < m_originalTriggers.size(); i++)
	{
		if (m_originalTriggers[i].Code() == searchValue)
		{
			indexes.push_back(i);
		}
	}
	return indexes;
}