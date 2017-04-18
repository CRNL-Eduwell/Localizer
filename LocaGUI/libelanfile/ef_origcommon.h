/**************************************************/
/* Common declaration for original data file use. */
/**************************************************/
#ifndef _ef_origcommon_h
#define _ef_origcommon_h


/* Type definition for reading/writing original file format. */
/*-----------------------------------------------------------*/
enum {ORIG_EEG_DATATYPE_12BITS=0, ORIG_EEG_DATATYPE_16BITS, ORIG_EEG_DATATYPE_32BITS};
enum {ORIG_TF_TEXT=0, ORIG_TF_BINARY};

#define ORIG_LAB_SIZE 256
typedef char orig_str_t[ORIG_LAB_SIZE];


typedef struct
{
	int orig_datatype;
	
	double *eeg_convADC;
	double *eeg_offsetADC;
	
	void *bufReadPtr;
	int bufReadSize;
} orig_eeg_info_t;

typedef struct
{
	int offset_data;
	double *bufReadPtr;
	int bufReadSize;
} orig_ep_info_t;

typedef struct
{
	int offset_data;
	int orig_version;
	double *bufReadPtr;
	int bufReadSize;
} orig_tf_info_t;

typedef struct
{
	int has_eeg_info;
	orig_eeg_info_t eeg_info;
	
	int has_ep_info;
	orig_ep_info_t ep_info;
	
	int has_tf_info;
	orig_tf_info_t tf_info;
} orig_info_t;


typedef struct
  {
  int ind;
  double theta, phi;
  orig_str_t lab;
  } elec_dat_t;



#else


#endif