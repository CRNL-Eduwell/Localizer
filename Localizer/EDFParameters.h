#ifndef _EDFPARAMETERS_H
#define _EDFPARAMETERS_H

#include <iostream>

namespace InsermLibrary
{
	struct Edf_header
	{									//Offset in File -- Size
		int version;					//      0		 --  8
		std::string patientID;			//		8		 --  80
		std::string recordingID;		//		88		 --  80
		std::string startDate;			//		168		 --  8
		std::string startTime;			//		176		 --  8
		int bytesNumber;				//		184		 --  8
		std::string reserved;			//		192		 --  44
		int recordsNumber;				//		236		 --  8
		int recordDuration;				//		244		 --  8
		int signalNumber;				//		252		 --  4
	};

	struct Edf_electrode
	{
		int id;
		std::string label;				//	256 + (i*16) --  16
		std::string transducerType;		//	Calculate	 --  80
		std::string dimension;			//	Calculate	 --  8
		int physicalMinimum;			//	Calculate	 --  8
		int physicalMaximum;			//	Calculate	 --  8
		int digitalMinimum;				//	Calculate	 --  8
		int digitalMaximum;				//	Calculate	 --  8
		std::string prefiltering;		//	Calculate	 --  80
		int samplesNumber;				//	Calculate	 --  8
		std::string reserved;			//	Calculate	 --  32
	};

	struct Edf_event
	{
		Edf_event(std::string p_label, int p_code , int p_sample)
		{
			label = p_label;
			code = p_code;
			sample = p_sample;
		}

		std::string label;
		int code;
		int sample;
	};
}

#endif