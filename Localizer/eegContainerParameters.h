#ifndef _EEGCONTAINERPARAMETERS_H
#define _EEGCONTAINERPARAMETERS_H

#include <iostream>
#include <vector>
#include "TRCFunctions.h"
#include "ELANFile.h"
#include "EDFFile.h"

using namespace std;
using namespace MicromedLibrary;

namespace InsermLibrary
{
	struct eventEeg
	{
		eventEeg() {};

		eventEeg(int code, int sample, int time = -1)
		{
			this->code = code;
			this->sample = sample;
			this->time = time;
		}

		~eventEeg() {};

		int code = -1;
		int sample = -1;
		int time = -1;
	};

	struct TRIGG
	{
		TRIGG(eventEeg trigger)
		{
			this->trigger = trigger;
		}

		TRIGG(eventEeg trigger, eventEeg response)
		{
			this->trigger = trigger;
			this->response = response;
		}

		~TRIGG()
		{

		};

		eventEeg trigger;
		eventEeg response;
		int rtSample = -1;
		int rtMs = -1;
	};

	inline bool operator==(int code, TRIGG &a)
	{
		return a.trigger.code == code;
	}

	inline bool operator!=(int code, TRIGG &a)
	{
		return !(a.trigger.code == code);
	}

	struct TRIGGINFO
	{
		TRIGGINFO(TRIGGINFO *p_triggers)
		{
			for (int i = 0; i < p_triggers->triggers.size(); i++)
			{
				triggers.push_back(TRIGG(eventEeg(p_triggers->triggers[i].trigger.code,
					p_triggers->triggers[i].trigger.sample),
					eventEeg(p_triggers->triggers[i].response.code,
						p_triggers->triggers[i].response.sample)));
			}
		}

		TRIGGINFO(TRIGGINFO *p_triggers, vec1<int> idSort)
		{
			for (int i = 0; i < p_triggers->triggers.size(); i++)
			{
				triggers.push_back(TRIGG(eventEeg(p_triggers->triggers[idSort[i]].trigger.code,
					p_triggers->triggers[idSort[i]].trigger.sample),
					eventEeg(p_triggers->triggers[idSort[i]].response.code,
						p_triggers->triggers[idSort[i]].response.sample)));
			}
		}

		TRIGGINFO(digitalTriggers *p_triggers, int nbTriggers, int downFactor = 1)
		{
			for (int i = 0; i < nbTriggers; i++)
			{
				triggers.push_back(TRIGG(eventEeg(p_triggers[i].triggerValue, p_triggers[i].triggerSample / downFactor)));
			}
		}

		TRIGGINFO(eventElanFile *p_triggers, int nbTriggers, int downFactor = 1)
		{
			for (int i = 0; i < nbTriggers; i++)
			{
				triggers.push_back(TRIGG(eventEeg(p_triggers[i].code, p_triggers[i].sample / downFactor)));
			}
		}

		TRIGGINFO(Edf_event *p_triggers, int nbTriggers, int downFactor = 1)
		{
			for (int i = 0; i < nbTriggers; i++)
			{
				triggers.push_back(TRIGG(eventEeg(p_triggers[i].code, p_triggers[i].sample / downFactor)));
			}
		}

		~TRIGGINFO()
		{
			if (triggers.size() > 0)
				triggers.clear();

			if (subGroupStimTrials.size() > 0)
				subGroupStimTrials.clear();

		}

		vector<TRIGG> triggers;
		vector<int> subGroupStimTrials;
	};

	struct elecContainer
	{
		string label;
		vector<int> id;
		vector<int> idOrigFile;
	};

	struct bipole
	{
		bipole(int elecPosid, int elecNegId)
		{
			positivElecId = elecPosid;
			negativElecId = elecNegId;
		}

		int positivElecId;
		int negativElecId;
		string positivLabel;
		string negativLabel;
	};
}

#endif