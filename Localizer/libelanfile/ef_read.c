/*--------------------------------------------------*/
/* Reading ELAN files module.                       */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_hdf5common.h"


/* Private functions prototypes. */
/*-------------------------------*/
extern int ef_orig_read_header(char *filename, elan_struct_t *elan_struct); /* Defined in ef_origread.c */
extern int ef_orig_read_data_all_channels(char *filename, elan_struct_t *elan_struct); /* Defined in ef_origread.c */
extern int ef_orig_read_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum); /* Defined in ef_origread.c */
extern int ef_orig_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, void **data_ptr); /* Defined in ef_origeeg.c */
extern int ef_orig_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, void ***data_ptr); /* Defined in ef_origeeg.c */
extern int ef_orig_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, void **data_ptr); /* Defined in ef_origep.c */
extern int ef_orig_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, void ***data_ptr); /* Defined in ef_origep.c */
extern int ef_orig_TF_read_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);
extern int ef_orig_TF_read_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr);

extern int ef_test_version(char *filename); /* Defined in ef_tools.c */


/*-------------------*/
/* Public functions. */
/*-------------------*/

/********************************************************/
/* ef_read_elan_file: read file (header+data). */
/********************************************************/
int ef_read_elan_file(char *filename, elan_struct_t *elan_struct)
{
	int err;
	
	err = ef_read_elan_header_file(filename, elan_struct);
	ef_alloc_data_array(elan_struct);
	err = ef_read_elan_data_all_channels_file(filename, elan_struct);
	
	return(err);
}


/****************************************************/
/* ef_read_elan_header_file: read file header only. */
/****************************************************/
int ef_read_elan_header_file(char *filename, elan_struct_t *elan_struct)
{
	int err=EF_NOERR, version;
	version = ef_test_version(filename);
	
	switch (version)
	{
		case HIST_VERSION:
			err = ef_orig_read_header(filename, elan_struct);
			break;
			
		case HDF5_VERSION:
			elan_struct->orig_info = NULL; /* No original file format information. */
			err = HDF5_read_header(filename, elan_struct);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", version);
	}
	
	return(err);
}


/****************************************************************************/
/* ef_read_elan_data_all_channels_file: read data for all channels at once. */
/****************************************************************************/
int ef_read_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct)
{
	int err=0;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err = ef_orig_read_data_all_channels(filename, elan_struct);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_data_all_channels(filename, elan_struct);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/**********************************************************************/
/* ef_read_elan_data_one_channel_file: read all data for one channel. */
/**********************************************************************/
int ef_read_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err = ef_orig_read_data_one_channel_file(filename, elan_struct, measureNum, chanNum);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_data_one_channel(filename, elan_struct, measureNum, chanNum);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_read_EEG_data_sampblock_one_chan: read an EEG data block for one channel and store in data_ptr.           */
/* char *filename: file to read from.                                                                           */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to read (starting from 0).                                             */
/* int sampNb: number of samples to read.                                                                       */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be sampNb double or double.                                 */
/* return value: number of samples actually read.                                                               */
/****************************************************************************************************************/
int ef_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	int err=EF_NOERR, actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err = ef_orig_read_EEG_data_sampblock_one_chan(filename, elan_struct, chanNum, sampStart, actualSampNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_EEG_data_sampblock_one_chan(filename, elan_struct, measureNum, chanNum, sampStart, actualSampNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_read_EEG_data_sampblock_all_chan: read an EEG data block for all channel and store in data_ptr.            */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).  */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] double or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr)
{
	int err=EF_NOERR, actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_read_EEG_data_sampblock_all_chan(filename, elan_struct, sampStart, actualSampNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_EEG_data_sampblock_all_chan(filename, elan_struct, measureNum, sampStart, actualSampNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_read_EPG_data_sampblock_one_chan: read an EP data block for one channel and store in data_ptr.            */
/* char *filename: file to read from.                                                                           */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to read (starting from 0).                                             */
/* int sampNb: number of samples to read.                                                                       */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be sampNb double or double.                                 */
/* return value: number of samples actually read.                                                               */
/****************************************************************************************************************/
int ef_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	int err=EF_NOERR, actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->ep.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err = ef_orig_read_EP_data_sampblock_one_chan(filename, elan_struct, chanNum, sampStart, actualSampNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_EP_data_sampblock_one_chan(filename, elan_struct, measureNum, chanNum, sampStart, actualSampNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_read_EP_data_sampblock_all_chan: read an EP data block for all channel and store in data_ptr.             */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).  */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] double or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr)
{
	int err=EF_NOERR, actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->ep.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_read_EP_data_sampblock_all_chan(filename, elan_struct, sampStart, actualSampNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_EP_data_sampblock_all_chan(filename, elan_struct, measureNum, sampStart, actualSampNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_read_TF_data_one_channel: read one time-frequency data buffer for measureNum and chanNum.                  */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int chanNum: indice of channel to read data from (starting from 0).                                           */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).  */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.     */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_read_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void ***data_ptr)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_one_channel(filename, elan_struct, chanNum,  0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_one_channel(filename, elan_struct, measureNum, chanNum, 0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_read_TF_data_all_channel: read time-frequency data buffer for measureNum and all channels.                 */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* void ****data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.     */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_read_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void ****data_ptr)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_all_channel(filename, elan_struct, 0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_all_channel(filename, elan_struct, measureNum, 0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_frequencyblock_one_channel: read all time samples for a frequency block and one channel.          */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel to read data from (starting from 0).                                          */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.    */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_one_channel(filename, elan_struct, chanNum,  0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_one_channel(filename, elan_struct, measureNum, chanNum, 0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_frequencyblock_all_channel: read all time samples for a frequency block and all channels.         */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.    */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ****data_ptr)
{
	int err=EF_NOERR;
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_all_channel(filename, elan_struct, 0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_all_channel(filename, elan_struct, measureNum, 0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_timeblock_one_channel: read a time block for all frequencies and one channel.                     */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel to read data from (starting from 0).                                          */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.tf.freq_nbxsampNb] double or double.        */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_one_channel(filename, elan_struct, chanNum,  sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_one_channel(filename, elan_struct, measureNum, chanNum, sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_timeblock_all_channel: read a time block for all frequencies and all channels.                    */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nb*elan_struct.tf.freq_nbxsampNb]     */
/*                 double or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ****data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_all_channel(filename, elan_struct, sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_all_channel(filename, elan_struct, measureNum, sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_tfblock_one_channel: read a time-frequency block for one channel.                                 */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel to read data from (starting from 0).                                          */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [freqNbxsampNb] double or double.                        */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
				
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_one_channel(filename, elan_struct, chanNum,  sampStart, sampNb, freqStart, freqNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_one_channel(filename, elan_struct, measureNum, chanNum, sampStart, sampNb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_read_TF_tfblock_all_channel: read a time-frequency block for channels.                                    */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nb*freqNbxsampNb]                     */
/*                 double or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			err =  ef_orig_TF_read_tfblock_all_channel(filename, elan_struct, sampStart, sampNb, freqStart, freqNb, data_ptr);
			break;
			
		case HDF5_VERSION:
			err = HDF5_read_TF_tfblock_all_channel(filename, elan_struct, measureNum, sampStart, sampNb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}

