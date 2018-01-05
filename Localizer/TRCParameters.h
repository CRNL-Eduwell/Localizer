#ifndef _TRCPARAMETERS_H
#define _TRCPARAMETERS_H

#define MAX_EVENT 100
#define AVERAGE_FREE 108
#define MAX_CAN_VIEW 128
#define MAX_CAN 256
#define MAX_LAB 640
#define MAX_NOTE 200
#define MAX_FLAG 100
#define MAX_SEGM 100
#define MAX_MONT 30
#define MAX_SAMPLE 128
#define MAX_HISTORY 30
#define MAX_FILE 1024
#define MAX_TRIGGER 8192

#include <iostream>
#include <vector>

using namespace std;

namespace MicromedLibrary
{
	struct digitalTriggers
	{													//Offset
		unsigned long int triggerSample;				//0
		unsigned short int triggerValue;				//4
	};

	struct MarkerPair
	{													//Offset
		long int begin;									//0
		long int end;									//4
	};

	struct eventsMarker
	{													//Offset
		char description[64];							//0
		vector<MarkerPair> selection;					//64
	};

	struct dVideoFiles
	{													//Offset
		unsigned long DV_Begin;							//0
	};

	struct offlineAverageProcess
	{													//Offset
		unsigned long int meanTrace;					//0
		unsigned long int meanFile;						//4
		unsigned long int meanPrestim;					//8
		unsigned long int meanPoststim;					//12
		unsigned long int meanType;						//16
		unsigned char freeFurtherUse[AVERAGE_FREE];		//20
	};

	struct compressionDescription
	{													//Offset
		char compressionStuff[10];						//0
	};

	struct inputOfMontages
	{													//Offset
		unsigned short int nonInverting;				//0
		unsigned short int inverting;					//2
	};

	struct montagesOfTrace
	{													//Offset
		unsigned short int lines;						//0
		unsigned short int sectors;						//2
		unsigned short int baseTime;					//4
		unsigned short int notch;						//6
		unsigned char colour[MAX_CAN_VIEW];				//8
		unsigned char selection[MAX_CAN_VIEW];			//136
		char description[64];							//264
		inputOfMontages inputs[MAX_CAN_VIEW];			//328
		unsigned long int highPassFilter[MAX_CAN_VIEW]; //840
		unsigned long int lowPassFilter[MAX_CAN_VIEW];  //1352
		unsigned long int reference[MAX_CAN_VIEW];		//1864
		unsigned char free[1720];						//2376 
	};

	struct impedanceFile
	{													//Offset
		unsigned char positive;							//0
		unsigned char negative;							//1
	};

	struct reducedFileDescription
	{													//Offset
		unsigned long int time;							//0
		unsigned long int sample;						//4
	};

	struct selectionFlag
	{													//Offset
		long int begin;									//0
		long int end;									//4
	};

	struct operatorNote
	{													//Offset -- Size
		unsigned long int sample;						//0
		string comment;									//4      -- 40
	};

	struct electrode
	{													//Offset -- Size
		unsigned char status;							//0
		unsigned char type;								//1
		string positiveInputLabel;						//2
		string negativeInputLabel;						//8
		long int logicMinimum;							//14
		long int logicMaximum;							//18
		long int logicGround;							//22
		long int physicMinimum;							//26
		long int physicMaximum;							//30
		unsigned short int measurementUnit;				//34
		unsigned short int prefilteringHighPassLimit;	//36
		unsigned short int prefilteringHighPassType;	//38
		unsigned short int prefilteringLowPassLimit;	//40
		unsigned short int prefilteringLowPassType;		//42
		unsigned short int rateCoefficient;				//44
		unsigned short int position;					//46
		unsigned short int idExtractFile;
		float lattitude;								//48
		float longitude;								//52
		unsigned char presentInMap;						//56
		unsigned char isInAvg;							//57
		string description;								//58
		float coordinate3DX;							//90
		float coordinate3DY;							//94
		float coordinate3DZ;							//98
		unsigned short int corrdinateType;				//102
		string freeFurtherUse;							//104    -- 24
	};

	struct descriptorArea
	{													//Offset
		string name;									//0
		unsigned long int startOffset;					//8
		unsigned long int length;						//12
	};

	struct recordingTime
	{													//Offset in File -- Size
		unsigned char hour;								//		131		 -- 1
		unsigned char min;								//		132		 -- 1
		unsigned char sec;								//		133		 -- 1
	};

	struct recordingDate
	{													//Offset in File -- Size
		unsigned char day;								//		128		 -- 1
		unsigned char month;							//		129		 -- 1
		unsigned char year;								//		130		 -- 1
	};

	struct patientData
	{													//Offset in File -- Size
		string surname;									//      64       -- 22
		string name;									//      86       -- 20
		unsigned char month;							//      106      -- 1
		unsigned char day;								//      107      -- 1
		unsigned char year;								//      108      -- 1
		unsigned char reserved[19];						//      109      -- 19
	};

	struct headerType4
	{													//Offset in File -- Size -- Default Values
		string title;									//		0		 -- 32
		string laboratory;							    //		32		 -- 32
		patientData patient;							//		64		 -- 64
		recordingDate recordDate;						//		128		 -- 3
		recordingTime beginTime;						//		131		 -- 3
		recordingTime endTime;
		unsigned short int acquisitionUnit;				//		134		 -- 2
		unsigned short int fileType;					//		136		 -- 2
		unsigned long int adressFirstData;				//		138		 -- 4    -- 648170
		unsigned short int numberStoredChannels;		//		142		 -- 2
		unsigned short int multiplexer;					//		144		 -- 2
		unsigned short int samplingRate;				//		146		 -- 2
		unsigned short int numberBytes;					//		148		 -- 2
		unsigned short int compression;					//		150		 -- 2
		unsigned short int numberMontagesStored;		//		152		 -- 2
		unsigned long int digitalVideoStartSample;		//		154		 -- 4
		unsigned short int mpegDelay;					//		158		 -- 2
		unsigned char reservedFurtherDEV[15];			//		160		 -- 15
		unsigned char headerType;						//		175		 -- 1
		descriptorArea descriptorCode;					//		176		 -- 16   -- Name : "ORDER   " | StartOffset = 640    | length 512
		descriptorArea descriptorElectrode;				//		192		 -- 16   -- Name : "LABCOD  " | StartOffset = 1152   | length 81920
		descriptorArea descriptorNote;					//		208		 -- 16   -- Name : "NOTE    " | StartOffset = 83072  | length 44000
		descriptorArea descriptorFlag;					//		224		 -- 16   -- Name : "FLAGS   " | StartOffset = 127072 | length 800
		descriptorArea descriptorReduction;				//		240		 -- 16   -- Name : "TRONCA  " | StartOffset = 127872 | length 800
		descriptorArea descriptorBeginImpedance;		//		256		 -- 16   -- Name : "IMPED_B " | StartOffset = 128672 | length 512
		descriptorArea descriptorEndImpedance;			//		272		 -- 16   -- Name : "IMPED_E " | StartOffset = 129184 | length 512
		descriptorArea descriptorMontages;				//		288		 -- 16   -- Name : "MONTAGE " | StartOffset = 129696 | length 122880
		descriptorArea descriptorCompression;			//		304		 -- 16   -- Name : "COMPRESS" | StartOffset = 252576 | length 10
		descriptorArea descriptorAverage;				//		320		 -- 16   -- Name : "AVERAGE " | StartOffset = 252586 | length 128
		descriptorArea descriptorHistory;				//		336		 -- 16   -- Name : "HISTORY " | StartOffset = 252714 | length 123392
		descriptorArea descriptorDVideo;				//		352		 -- 16   -- Name : "DVIDEO  " | StartOffset = 376106 | length 16384
		descriptorArea descriptorEventA;				//		368		 -- 16   -- Name : "EVENT A " | StartOffset = 392490 | length 864
		descriptorArea descriptorEventB;				//		384		 -- 16   -- Name : "EVENT B " | StartOffset = 393354 | length 864
		descriptorArea descriptorTrigger;				//		400		 -- 16   -- Name : "TRIGGER " | StartOffset = 394218 | length 49152
		unsigned char reservedFurtherDEV2[224];			//		416		 -- 16
	};
}

#endif