#ifndef _TRIGGER_H
#define _TRIGGER_H

#include "ITrigger.h"

namespace InsermLibrary
{
	class Trigger
	{
	public:
		Trigger(EEGFormat::ITrigger mainEvent, int samplingFrequency);
		Trigger(EEGFormat::ITrigger mainEvent, EEGFormat::ITrigger response, int samplingFrequency);
		~Trigger();
		inline EEGFormat::ITrigger MainEvent()
		{
			return m_mainEvent;
		}
		inline EEGFormat::ITrigger Reponse()
		{
			return m_response;
		}
		inline void Response(int code, long sample)
		{
			m_response.Code(code);
			m_response.Sample(sample);
		}
		inline void Response(EEGFormat::ITrigger response)
		{
			m_response.Code(response.Code());
			m_response.Sample(response.Sample());
		}
		inline int SamplingFrequency()
		{
			return m_samplingFrequency;
		}
		inline int MainCode()
		{
			return m_mainEvent.Code();
		}
		inline long MainSample()
		{
			return m_mainEvent.Sample();
		}
		inline int ResponseCode()
		{
			return m_response.Code();
		}
		inline long ResponseSample()
		{
			m_response.Sample();
		}
		inline int ReactionTimeInSample()
		{
			return m_response.Sample() - m_mainEvent.Sample();
		}
		inline int ReactionTimeInMilliSeconds()
		{
			int mainTime = 1000 * ((float)m_mainEvent.Sample() / m_samplingFrequency);
			int responseTime = 1000 * ((float)m_response.Sample() / m_samplingFrequency);
			return responseTime - mainTime;
		}
		void UpdateFrequency(int newFrequency);

	private:
		int m_samplingFrequency = 0;
		EEGFormat::ITrigger m_mainEvent;
		EEGFormat::ITrigger m_response;
	};

	inline bool operator==(int code, Trigger &a)
	{
		return a.MainCode() == code;
	}
	inline bool operator!=(int code, Trigger &a)
	{
		return !(a.MainCode() == code);
	}
}
#endif