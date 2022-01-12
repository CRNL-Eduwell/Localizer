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

// Return a list of trigger for the wanted experiment
// Use flagCode = -1 to get the full trigger list
std::vector<InsermLibrary::Trigger> InsermLibrary::TriggerContainer::GetTriggerForExperiment(PROV *myprovFile, int flagCode, int downSamplingFactor)
{
	std::vector<Trigger> triggers = GetTriggerList(flagCode, downSamplingFactor);
	if (myprovFile != nullptr && triggers.size() != 0)
	{
		if (myprovFile->changeCodeFilePath != "")
		{
			RenameTriggersForExperiment(myprovFile, triggers);
		}
	}
	return triggers;
}

std::vector<InsermLibrary::Trigger> InsermLibrary::TriggerContainer::GetTriggerList(int flagCode, int downSamplingFactor)
{
	int beginvalue = 0;
	if (flagCode != -1)
		beginvalue = FindFirstIndexAfter(flagCode);
	std::vector<InsermLibrary::Trigger> triggers;
	if (m_originalTriggers.size() > 0)
	{
		for (int i = beginvalue; i < m_originalTriggers.size(); i++)
		{
			Trigger currentTrigger(m_originalTriggers[i], m_originalSamplingFrequency);
			currentTrigger.UpdateFrequency(m_originalSamplingFrequency / downSamplingFactor);
			triggers.push_back(currentTrigger);
		}
	}
	return triggers;
}

void InsermLibrary::TriggerContainer::ProcessEventsForExperiment(PROV *myprovFile, int flagCode, int downSaplingFactor)
{
	m_processedTriggers = GetTriggerList(flagCode, downSaplingFactor);
	if (m_processedTriggers.size() == 0)
		return;

	if (myprovFile->changeCodeFilePath != "")
	{
		RenameTriggersForExperiment(myprovFile, m_processedTriggers);
	}
	PairStimulationWithResponses(m_processedTriggers, myprovFile);
	DeleteTriggerNotInExperiment(m_processedTriggers, myprovFile);
	if (myprovFile->getSecondaryCodes()[0][0] != 0) //At this point , if there is secondary code, we need to check if all have been paired correctly 
	{
		DeleteTriggerNotPaired(m_processedTriggers);
	}
	if (m_processedTriggers.size() == 0)
		return;

	m_subGroupStimTrials = SortTrialsForExperiment(m_processedTriggers, myprovFile);
	if (myprovFile->invertmapsinfo != "")
	{
		SwapStimulationsAndResponses(myprovFile);
	}
}

void InsermLibrary::TriggerContainer::RenameTriggersForExperiment(PROV *myprovFile, std::vector<Trigger>& triggers)
{
	std::vector<std::string> rawChangeCodeData = EEGFormat::Utility::ReadTextFile(myprovFile->changeCodeFilePath);

	std::vector<std::vector<std::pair<int, int>>> oldCodes;
	std::vector<std::pair<int, int>> newCodes;
	for (int i = 0; i < rawChangeCodeData.size(); i++)
	{
        std::vector<std::string> currentRawCodes = EEGFormat::Utility::Split<std::string>(rawChangeCodeData[i], "+=");
		std::pair<int, int> currentOldCode = std::make_pair(atoi(&(currentRawCodes[0])[0]), atoi(&(currentRawCodes[1])[0]));
		std::pair<int, int> currenNewCode = std::make_pair(atoi(&(currentRawCodes[2])[0]), atoi(&(currentRawCodes[3])[0]));

		auto it = std::find(newCodes.begin(), newCodes.end(), currenNewCode);
		if (it != newCodes.end()) //if the pair already exists
		{
			int indexOfPair = distance(newCodes.begin(), it);
			oldCodes[indexOfPair].push_back(currentOldCode);
		}
		else
		{
			newCodes.push_back(currenNewCode);
			//==
			std::vector<std::pair<int, int>> newVector;
			newVector.push_back(currentOldCode);
			oldCodes.push_back(newVector);
		}
	}

	int idVisuBloc = 0;
	int TriggerCount = triggers.size();
	for (int k = 0; k < TriggerCount; k++)
	{
		int idVisuBloc = -1;
		int idMain = -1;
		int idSec = -1;

		for (int l = 0; l < oldCodes.size(); l++)
		{
			for (int m = 0; m < oldCodes[l].size(); m++)
			{
				if (triggers[k].MainEvent().Code() == oldCodes[l][m].first)
				{
					idMain = k;
					for (int n = 0; n < myprovFile->visuBlocs.size(); n++)
					{
						if (newCodes[l].first == myprovFile->visuBlocs[n].mainEventBloc.eventCode[0])
							idVisuBloc = n;
					}
				}
			}
		}

		//TODO : add a function that check there is not some triggerd with a different
		//sampling frequnecy, otherwise throw exception
		int memId = 0;
		int triggersSamplingFrequency = triggers[0].SamplingFrequency();
		if (idMain != -1)
		{
			int winSamMin = round((triggersSamplingFrequency * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMin()) / 1000);
			int winSamMax = round((triggersSamplingFrequency * myprovFile->visuBlocs[idVisuBloc].dispBloc.windowMax()) / 1000);

			int CurrentTriggerId = k + 1;
			while (idSec == -1 && CurrentTriggerId < TriggerCount - 1)
			{
				for (int l = 0; l < oldCodes.size(); l++)
				{
					for (int m = 0; m < oldCodes[l].size(); m++)
					{
						if (triggers[idMain].MainCode() == oldCodes[l][m].first && triggers[CurrentTriggerId].MainCode() == oldCodes[l][m].second)
						{
							idSec = CurrentTriggerId;
							memId = l;
							break;
						}
						else if (triggers[CurrentTriggerId].MainCode() == oldCodes[l][m].first && idSec == -1)
						{
							idMain = CurrentTriggerId;
							memId = l;
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
				//bool specialLoca = (currentLoca->localizerName() == "MARA" ||
				//	currentLoca->localizerName() == "MARM" ||
				//	currentLoca->localizerName() == "MARD");
				if (isInWindow /*|| specialLoca*/)
				{
					triggers[idMain].MainCode(newCodes[memId].first);
					triggers[idSec].MainCode(newCodes[memId].second);
				}
			}
		}
	}
}

void InsermLibrary::TriggerContainer::SwapStimulationsAndResponses(PROV *myprovFile)
{
	int TriggerCount = m_processedTriggers.size();
	for (int i = 0; i < TriggerCount; i++)
	{
		m_processedTriggers[i].SwapStimulationAndResponse();
	}

	//This is the new window to visualize data
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
		myprovFile->visuBlocs[i].dispBloc.window(myprovFile->invertmaps.epochWindow[0], myprovFile->invertmaps.epochWindow[1]);
		myprovFile->visuBlocs[i].dispBloc.baseLine(myprovFile->invertmaps.baseLineWindow[0], myprovFile->invertmaps.baseLineWindow[1]);
	}
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

			int CurrentTriggerId = k + 1;
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
					triggers[idMain].Response(triggers[idSec].MainEvent());
				}
				else
				{
					//if it is out of the window but one of the special Loca (MARA, MARM or MARD we want to mark a "fake" reponse 
					//at the maximum window length to keep track of those trials
					if (specialLoca)
					{
						triggers[idMain].Response(triggers[idSec].MainCode(), winMax);
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

std::vector<int> InsermLibrary::TriggerContainer::SortTrialsForExperiment(std::vector<Trigger>& triggers, PROV *myprovFile)
{
	//Sort by MainCode
	std::vector<int> mainEventsCode = myprovFile->getMainCodes();
	std::vector<Trigger> sortedByMainCodeArray;
	for (int i = 0; i < mainEventsCode.size(); i++)
	{
		std::copy_if(triggers.begin(), triggers.end(), std::back_inserter(sortedByMainCodeArray), [&](Trigger trigger) 
		{
			return trigger.MainCode() == mainEventsCode[i];
		});
	}
	triggers = std::vector<Trigger>(sortedByMainCodeArray);

	//get first id of each new main code
	std::vector<int> subGroupStimTrials;
	subGroupStimTrials.push_back(0);
	for (int i = 0; i < triggers.size() - 1; i++)
	{
		if (triggers[i].MainCode() != triggers[i + 1].MainCode())
		{
			subGroupStimTrials.push_back(i + 1);
		}
	}
	subGroupStimTrials.push_back(triggers.size());

	for (int i = 0; i < subGroupStimTrials.size() - 1; i++)
	{
		int beg = subGroupStimTrials[i];
		int end = subGroupStimTrials[i + 1];
		sort(triggers.begin() + beg, triggers.begin() + end,
			[](Trigger a, Trigger b)-> bool
		{
			return (a.MainSample() < b.MainSample());
		});
	}

	//according to the rest sort by what is wanted
	int beg = 0, end = 0;
	for (int i = 0; i < myprovFile->visuBlocs.size(); i++)
	{
        std::string currentSort = myprovFile->visuBlocs[i].dispBloc.sort();
        std::vector<std::string> sortSplited = split<std::string>(currentSort, "_");
		for (int j = 1; j < sortSplited.size(); j++)
		{
			SortingChoice Choice = (SortingChoice)(sortSplited[j][0]);
			switch (Choice)
			{
				//Sort by resp code
			case SortingChoice::Code:
				beg = subGroupStimTrials[i];
				end = subGroupStimTrials[i + 1];
				sort(triggers.begin() + beg, triggers.begin() + end,
					[](Trigger a, Trigger b)-> bool
				{
					return (a.ResponseCode() < b.ResponseCode());
				});
				break;
				//Sort by reaction time
			case SortingChoice::Latency:
				beg = subGroupStimTrials[i];
				end = subGroupStimTrials[i + 1];
				sort(triggers.begin() + beg, triggers.begin() + end,
					[](Trigger m, Trigger n)-> bool
				{
					return  m.ReactionTimeInMilliSeconds() < n.ReactionTimeInMilliSeconds();
				});
				break;
			}
		}
	}

	return subGroupStimTrials;
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
