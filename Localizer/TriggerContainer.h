#ifndef _TRIGGERCONTAINER_H
#define _TRIGGERCONTAINER_H

#include "ITrigger.h"
#include "Trigger.h"
#include "PROV.h"

namespace InsermLibrary
{
	class TriggerContainer
	{
	public:
		TriggerContainer(std::vector<EEGFormat::ITrigger>& triggers, int samplingFrequnecy);
		~TriggerContainer();
		std::vector<Trigger> GetTriggerList(int flagCode, int downSamplingFactor = 1);
		void PairStimulationWithResponses(std::vector<Trigger>& triggers, PROV *myprovFile);
		void DeleteTriggerNotInExperiment(std::vector<Trigger>& triggers, PROV *myprovFile);
		void DeleteTriggerNotPaired(std::vector<Trigger>& triggers);
		void SortTrialsForExperiment(std::vector<Trigger>& triggers, PROV *myprovFile);

	private:
		int FindFirstIndexAfter(int flagCode = 99);
		std::vector<int> FindIndexes(int searchValue);

	private:
		int m_originalSamplingFrequency = 0;
		std::vector<EEGFormat::ITrigger> m_originalTriggers;
	};
}

#endif //_TRIGGERCONTAINER_H