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
		enum SortingChoice { Code = 'C', Latency = 'L' };

	public:
		TriggerContainer(std::vector<EEGFormat::ITrigger>& triggers, int samplingFrequnecy);
		~TriggerContainer();
		inline int ProcessedTriggerCount()
		{
			return m_processedTriggers.size();
		}
		inline std::vector<Trigger>& ProcessedTriggers() { return m_processedTriggers; }
		inline const std::vector<Trigger>& ProcessedTriggers() const { return m_processedTriggers; }
		inline const std::vector<int>& SubGroupStimTrials() const { return m_subGroupStimTrials; }		
		std::vector<Trigger> GetTriggerList(int flagCode, int downSamplingFactor = 1);
		void ProcessEventsForExperiment(PROV *myprovFile, int flagCode = 99, int downSaplingFactor = 1);

	private:		
		int FindFirstIndexAfter(int flagCode = 99);
		std::vector<int> FindIndexes(int searchValue);
		void PairStimulationWithResponses(std::vector<Trigger>& triggers, PROV *myprovFile);
		void DeleteTriggerNotInExperiment(std::vector<Trigger>& triggers, PROV *myprovFile);
		void DeleteTriggerNotPaired(std::vector<Trigger>& triggers);
		std::vector<int> SortTrialsForExperiment(std::vector<Trigger>& triggers, PROV *myprovFile);

	private:
		std::vector<int> m_subGroupStimTrials;
		std::vector<Trigger> m_processedTriggers;
		int m_originalSamplingFrequency = 0;
		std::vector<EEGFormat::ITrigger> m_originalTriggers;
	};
}

#endif //_TRIGGERCONTAINER_H