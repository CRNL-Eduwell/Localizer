/*--------------------------------------------------------*/
/* Header and type definitions for Elan files read/write. */
/* v: 1.00    Aut.: PEA                                   */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1       */
/*--------------------------------------------------------*/
#ifndef _elanfile_h
#define _elanfile_h

#if defined(_WIN32) || defined(_WIN64)
	#include "elan_define.h"	
#else
	#include "../../include/elan_define.h"
#endif

#include "ef_origcommon.h"


/* Error values. */
/*---------------*/
#define EF_NOERR 0
#define EF_ERR_OPEN_READ -1
#define EF_ERR_READ_HEADER -2
#define EF_ERR_READ_DATA -3
#define EF_ERR_OPEN_WRITE -4
#define EF_ERR_WRITE_DATA -5
#define EF_ERR_BAD_PARM -6


#define EF_LITTLE_ENDIAN 0
#define X86 EF_LITTLE_ENDIAN
#define EF_BIG_ENDIAN 1
#define EF_PDP_ENDIAN 2

#define EF_YES 1
#define EF_NO 0

#define UNDEF_VERSION -1
#define HIST_VERSION 0
#define HDF5_VERSION 1

#define CURRENT_VERSION 1
#define CURRENT_RELEASE 0

#define EF_DATA_FLOAT 0
#define EF_DATA_DOUBLE 1

#define EF_EEG_CONTINUOUS 0
#define EF_EEG_EPOCH 1

#define COMMENT_SIZE 512
#define LABEL_SIZE 256


typedef char str_label_t[LABEL_SIZE];

#define CoordSystemNb 1
extern str_label_t CoordSystemLabelList[];
enum {CoordSystemSphericalHead=0};


#define ChannelTypeNb 1
extern str_label_t ChannelTypeLabelList[];
enum {ChannelTypeEEG=0};


typedef struct
{
	str_label_t lab;
	int val_nb;
	float *val_list;
} coord_t;

typedef struct
{
	str_label_t lab;
	str_label_t type;
	str_label_t unit;
	int coord_nb;
	coord_t *coord_list;
} chan_t;

typedef struct
{
	int data_type; /* 0=float32, 1=double (float64) */
	int samp_nb; /* Number of samples. */
	float sampling_freq;
	int flag_cont_epoch; /* 0=continuous, 1=epoched. */
	/* Epoched specific data attributes. */
	int prestim_samp_nb;
	int poststim_samp_nb;
	int event_code;
	int epoch_nb; /* Epoch nb. */
	
	/* Continuous specific data attributes. */
	int event_nb;
	int *event_code_list;
	int *event_sample_list;
	
	/* Data. */
	float ***data_float; /* measures x channels x samples */
	float *flat_float; /* size = measures x channels x samples */
	double ***data_double; /* measures x channels x samples */
	double *flat_double; /* size = measures x channels x samples */
} eeg_struct_t;

typedef struct
{
	int data_type; /* 0=float32, 1=double (float64) */
	int samp_nb; /* Number of samples. */
	float sampling_freq;
	int prestim_samp_nb;
	int event_code;
	int event_nb; /* Averaged event nb. */
	
	int *other_events_nb; /* Number of events (other than trigger) for each measure. */
	int **other_events_list; /* Time list of other events for each measure. */
	
	float ***data_float; /* measures x channels x samples */
	float *flat_float; /* size = measures x channels x samples */
	double ***data_double;
	double *flat_double; /* size = measures x channels x samples */
} ep_struct_t;


enum {NoWavelet=0, MorletWavelet, GaborWavelet};

typedef struct
{
	int data_type; /* 0=float32, 1=double (float64) */
	int samp_nb; /* Number of samples. */
	float sampling_freq;
	int prestim_samp_nb;
	int event_code;
	int event_nb; /* Averaged event nb. */
	int wavelet_type; /* Gabor, Morlet, ... */
	int blackman_window;
	int freq_nb;
	float *freq_array;
	float *wavelet_char; /* Wavelet characteristic for each frequency. */

	int *other_events_nb; /* Number of events (other than trigger) for each measure. */
	int **other_events_list; /* Time list of other events for each measure. */

	float ****data_float; /* measures x channels x frequency x samples */
	float *flat_float; /* size = measures x channels x frequency x samples */
	double ****data_double; /* measures x channels x frequency x samples */
	double *flat_double; /* size = measures x channels x frequency x samples */
} tf_struct_t;


#define CompressionTypeNb 2
enum {EF_COMP_NONE=0, EF_COMP_GZIP};

typedef struct
{
	int compress;
	int compressionType;
	int dataType; /* Float or double */
} ef_option_struct_t;


typedef struct
{
	/* COMMON PART. */
	int endian;  /* endian of data stored in file (x86=little=0, big=1). */ 
	int version; /* 0 for historic version, 1=HDF5 (current). */
	int release; /* 0 for historic version, 0 for 1st HDF5 (current). */
	char comment[COMMENT_SIZE]; /* Comment or acquisition system. */
	int chan_nb; /* Number of channels. */
	int measure_channel_nb; /* Number of measures for each channel. */
	str_label_t *measure_channel_label_list; /* Label for each measure. */
	
	/* CHANNEL PART (COMMON). */
	chan_t *chan_list;
	
	/* EEG SPECIFIC PART. */
	int has_eeg; /* Flag to know if EEG data present. */
	eeg_struct_t eeg; /* EEG specific structure. */
	
	/* EP SPECIFIC PART. */
	int has_ep; /* Flag to know if EP data present. */
	ep_struct_t ep;
	
	/* TF SPECIFIC PART. */
	int has_tf; /* Flag to know if TF data present. */
	tf_struct_t tf;
	
	/* ORIGINAL FILE FORMAT SPECIFIC INFORMATIONS. */
	orig_info_t *orig_info;
	
	/* OPTIONS (command line or parameter file). */
	ef_option_struct_t options;
} elan_struct_t;


/* Function prototypes. */
/*----------------------*/
/* General reading functions. */
int ef_read_elan_file(char *filename, elan_struct_t *elan_struct);
int ef_read_elan_header_file(char *filename, elan_struct_t *elan_struct);
int ef_read_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct);
int ef_read_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);

/* Specific reading functions. */
int ef_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
int ef_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
int ef_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
int ef_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
int ef_read_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void ***data_ptr);
int ef_read_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void ****data_ptr);
int ef_read_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void ***data_ptr);
int ef_read_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ****data_ptr);
int ef_read_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void ***data_ptr);
int ef_read_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ****data_ptr);
int ef_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);
int ef_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr);

/* General writing functions. */
int ef_write_elan_file(char *filename, elan_struct_t *elan_struct);
int ef_write_elan_header_file(char *filename, elan_struct_t *elan_struct);
int ef_write_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct);
int ef_write_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);

/* Specific writing functions. */
int ef_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
int ef_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
int ef_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
int ef_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
int ef_write_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void **data_ptr);
int ef_write_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void ***data_ptr);
int ef_write_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void **data_ptr);
int ef_write_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ***data_ptr);
int ef_write_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
int ef_write_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
int ef_write_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **data_ptr);
int ef_write_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);

/* Various tools. */
void ef_init_structure(elan_struct_t *elan_struct);
void ef_set_structure_lastversion(elan_struct_t *elan_struct);
void ef_alloc_data_array(elan_struct_t *elan_struct);
void ef_free_struct(elan_struct_t *elan_struct);
void ef_free_data_array(elan_struct_t *elan_struct);
void *ef_alloc_array_1d(int dim1, int dataSize);
void ef_free_array_1d(void **ptr);
void *ef_alloc_array_2d(int dim1, int dim2, int dataSize);
void ef_free_array_2d(void ***ptr);
void *ef_alloc_array_3d(int dim1, int dim2, int dim3, int dataSize);
void ef_free_array_3d(void ****ptr, int dim1, int dim2, int dim3);
void ef_alloc_measure_label_list(elan_struct_t *elan_struct);
void ef_alloc_channel_list(elan_struct_t *elan_struct);
void ef_alloc_eeg_array_events(elan_struct_t *elan_struct);
void ef_alloc_ep_array_otherevents(elan_struct_t *elan_struct);
void ef_alloc_ep_array_otherevents_list(elan_struct_t *elan_struct);
void ef_alloc_tf_array_byfreq(elan_struct_t *elan_struct);
void ef_alloc_tf_array_otherevents(elan_struct_t *elan_struct);
void ef_alloc_tf_array_otherevents_list(elan_struct_t *elan_struct);
int ef_get_mach_endian();
void ef_copy_elan_struct(elan_struct_t *dest, elan_struct_t src);
void ef_parse_options(int argc, char **argv, elan_struct_t *elan_struct);
void ef_display_header(elan_struct_t *elan_struct);


#else


/* Function prototypes. */
/*----------------------*/
/* General reading functions. */
extern int ef_read_elan_file(char *filename, elan_struct_t *elan_struct);
extern int ef_read_elan_header_file(char *filename, elan_struct_t *elan_struct);
extern int ef_read_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct);
extern int ef_read_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);

/* Specific reading functions. */
extern int ef_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
extern int ef_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr);
extern int ef_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
extern int ef_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr);
extern int ef_read_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void ***data_ptr);
extern int ef_read_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void ****data_ptr);
extern int ef_read_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void ***data_ptr);
extern int ef_read_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ****data_ptr);
extern int ef_read_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void ***data_ptr);
extern int ef_read_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ****data_ptr);
extern int ef_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);
extern int ef_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr);

/* General writing functions. */
extern int ef_write_elan_file(char *filename, elan_struct_t *elan_struct);
extern int ef_write_elan_header_file(char *filename, elan_struct_t *elan_struct);
extern int ef_write_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct);
extern int ef_write_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);

/* Specific writing functions. */
extern int ef_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
extern int ef_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
extern int ef_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
extern int ef_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
extern int ef_write_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void *data_ptr);
extern int ef_write_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void *data_ptr);
extern int ef_write_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void **data_ptr);
extern int ef_write_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ***data_ptr);
extern int ef_write_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
extern int ef_write_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
extern int ef_write_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **data_ptr);
extern int ef_write_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);

/* Various tools. */
extern void ef_init_structure(elan_struct_t *elan_struct);
extern void ef_set_structure_lastversion(elan_struct_t *elan_struct);
extern void ef_alloc_data_array(elan_struct_t *elan_struct);
extern void ef_free_struct(elan_struct_t *elan_struct);
extern void ef_free_data_array(elan_struct_t *elan_struct);
extern void *ef_alloc_array_1d(int dim1, int dataSize);
extern void ef_free_array_1d(void **ptr);
extern void *ef_alloc_array_2d(int dim1, int dim2, int dataSize);
extern void ef_free_array_2d(void **ptr);
extern void *ef_alloc_array_3d(int dim1, int dim2, int dim3, int dataSize);
extern void ef_free_array_3d(void ****ptr, int dim1, int dim2, int dim3);
extern void ef_alloc_measure_label_list(elan_struct_t *elan_struct);
extern void ef_alloc_channel_list(elan_struct_t *elan_struct);
extern void ef_alloc_eeg_array_events(elan_struct_t *elan_struct);
extern void ef_alloc_ep_array_otherevents(elan_struct_t *elan_struct);
extern void ef_alloc_ep_array_otherevents_list(elan_struct_t *elan_struct);
extern void ef_alloc_tf_array_byfreq(elan_struct_t *elan_struct);
extern void ef_alloc_tf_array_otherevents(elan_struct_t *elan_struct);
extern void ef_alloc_tf_array_otherevents_list(elan_struct_t *elan_struct);
extern int ef_get_mach_endian();
extern void ef_copy_elan_struct(elan_struct_t *dest, elan_struct_t src);
extern void ef_parse_options(int argc, char **argv, elan_struct_t *elan_struct);
extern void ef_display_header(elan_struct_t *elan_struct);

#endif
